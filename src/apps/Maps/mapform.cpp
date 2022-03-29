#include "mapform.h"
#include "ui_mapform.h"
#include "routedialog.h"
#include "ui_routedialog.h"
#include "finddialog.h"
#include "ui_finddialog.h"
#include "findaddressdialog.h"
#include "ui_findaddressdialog.h"
#include "scaledialog.h"
#include "ui_scaledialog.h"
#include "locationdialog.h"
#include "ui_locationdialog.h"
#include "layerdialog.h"
#include "ui_layerdialog.h"
#include "routeinstructionsdialog.h"
#include "ui_routeinstructionsdialog.h"
#include "routeprofiledialog.h"
#include "ui_routeprofiledialog.h"
#include "util.h"

#include <stdio.h>

#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>

MapForm::MapForm(QWidget* aParent,MainWindow& aMainWindow,const std::vector<QString>& aMapFileNameArray):
    QOpenGLWidget(aParent),
    m_ui(new Ui::MapForm),
    m_main_window(aMainWindow),
    m_find_dialog(this)
    {
    assert(!aMapFileNameArray.empty());
    m_filename_array.push_back(aMapFileNameArray[0]);
    m_ui->setupUi(this);

    m_route_point_array.resize(2);
    m_route_point_array[0] = TRoutePoint();
    m_route_point_array[1] = TRoutePoint();

    // Create the CartoType framework object.
    CartoType::CFramework::TParam param;
    param.iSharedEngine = m_main_window.Engine();
    param.iMapFileName = aMapFileNameArray[0].utf16();
    param.iStyleSheetFileName = m_main_window.DefaultStyleSheetPath();

    QRect rect(geometry());
    param.iViewWidth = rect.width();
    param.iViewHeight = rect.height();
    param.iTextIndexLevels = 1;

    CartoType::TResult error;
    m_framework = CartoType::CFramework::New(error,param);
    if (!error)
        {
        error = m_framework->CreateWritableMap(CartoType::TWritableMapType::Memory);
        m_writable_map_handle = m_framework->GetLastMapHandle();
        if (!error)
            m_framework->EditSetWritableMap(m_writable_map_handle);
        }
    if (error)
        {
        std::string s("cannot open map ");
        s += std::string(param.iMapFileName);
        m_main_window.ShowError(s.c_str(),error);
        return;
        }

    // Load any other maps.
    for (size_t i = 1; i < aMapFileNameArray.size(); i++)
        {
        error = m_framework->LoadMap(aMapFileNameArray[i].utf16());
        if (error)
            {
            std::string s("cannot open map ");
            s += aMapFileNameArray[i].toStdString();
            m_main_window.ShowError(s.c_str(),error);
            }
        }

    // Start an async task to load navigation data.
    m_framework->LoadNavigationData();

    // Prevent panning beyond the map extent, and zooming in further than 1:1000.
    m_framework->SetViewLimits(1000,0);

    // Set the window title to that of the data set.
    QString qname;
    SetString(qname,m_framework->DataSetName());
    setWindowTitle(qname);

    // Add a style for UK historic counties, but disable it so that it can be enabled if needed.
    static const char* historic_counties_style =
        "<CartoTypeStyleSheet><layer name='county/historic'>"
        "<shape border='grey+red' opacity='0.5' width='0.6pt'/>"
        "<scale max='1000000'><label case='upper' color='grey+red' font-size='8pt' font-weight='bold' letter-spacing='0.2em' opacity='0.5' position='horizontal' priority='-4' wrapWidth='20em'/></scale>"
        "</layer></CartoTypeStyleSheet>";
    m_framework->AppendStyleSheet((const uint8_t*)historic_counties_style,strlen(historic_counties_style));
    m_framework->EnableLayer("county/historic",false);

    m_framework->SetFollowMode(CartoType::TFollowMode::None);
    m_framework->SetLocale(m_metric_units ? "en_XX" : "en");
    m_framework->EnableTurnInstructions(m_simulate_routing);
    CartoType::TExtendedNoticePosition turn_instruction_position(CartoType::TNoticePosition::TopLeft,15,"mm",4,"mm");
    m_framework->SetTurnInstructions(m_metric_units,false,30,"mm",turn_instruction_position);

    CreateLegend();
    m_framework->EnableLegend(m_draw_legend || m_draw_scale);

    // Create the OpenGL map renderer.
    m_map_renderer = std::make_unique<CartoType::CQtMapRenderer>(*m_framework);

    // Create the initial map image.
    m_map_image.reset(new QImage(rect.width(),rect.height(),QImage::Format_ARGB32_Premultiplied));

    m_ui->perspective_slider->hide();

    SetGraphicsAcceleration(true);
    }

MapForm::~MapForm()
    {
    delete m_ui;
    }

QString MapForm::ViewState() const
    {
    CartoType::TViewState v { m_framework->ViewState() };
    return QString::fromStdString(v.ToXml());
    }

void MapForm::SetView(const CartoType::TViewState& aViewState)
    {
    m_framework->SetView(aViewState);
    m_ui->dial->setValue(int(m_framework->Rotation() * 100));
    m_main_window.UpdateNorthUp();
    m_ui->perspective_slider->setValue((int)m_framework->PerspectiveParam().iDeclinationDegrees);
    if (m_framework->Perspective())
        {
#ifndef Q_OS_MAC // vertical sliders don't work properly on Mac: see QTBUG-57194: QSlider in orientation vertical fill display inverted on Mac X.12
        m_ui->perspective_slider->show();
#endif
        }
    else
        m_ui->perspective_slider->hide();
    m_main_window.UpdatePerspective();
    }

void MapForm::resizeEvent(QResizeEvent* aEvent)
    {
    QOpenGLWidget::resizeEvent(aEvent);
    if (m_framework &&
        aEvent->size().width() > 0 &&
        aEvent->size().height() > 0)
        {
        m_framework->Resize(aEvent->size().width(),aEvent->size().height());
        m_map_image.reset(new QImage(aEvent->size(),QImage::Format_ARGB32_Premultiplied));
        }

    static bool first = true;
    if (first)
        {
        // m_framework->SetViewToWholeMap();
        first = false;
        }
    }

void MapForm::paintEvent(QPaintEvent* aEvent)
    {
    if (m_graphics_acceleration)
        {
        QOpenGLWidget::paintEvent(aEvent);
        return;
        }

    CartoType::TResult error = 0;
    const CartoType::TBitmap* map_bitmap = MapBitmap(error);
    if (map_bitmap && !error)
        {
        CopyBitmapToImage(*map_bitmap,*m_map_image);
        QPainter painter(this);
        painter.drawImage(0,0,*m_map_image);
        }
    }

void MapForm::mousePressEvent(QMouseEvent* aEvent)
    {
    if (aEvent->button() == Qt::MouseButton::LeftButton)
        LeftButtonDown(aEvent->x(),aEvent->y());
    else if (aEvent->button() == Qt::MouseButton::RightButton)
        RightButtonDown(aEvent->x(),aEvent->y());
    }

void MapForm::SetEditMode(bool aSet)
    {
    if (m_edit_mode != aSet)
        {
        m_edit_mode = aSet;
        setMouseTracking(m_edit_mode);
        ShowEditedObjectSize();
        }
    }

/*
Double-clicking the left mouse button enters map object editing mode.
If there is a point of an existing editable object within 2mm, it is selected,
otherwise a new object is inserted; if the shift key is down the new object is
a polygon, else it is a line.
*/
void MapForm::mouseDoubleClickEvent(QMouseEvent* aEvent)
    {
    if (aEvent->button() != Qt::MouseButton::LeftButton)
        return;

    CartoType::TPointFP p(aEvent->x(),aEvent->y());
    if (!m_edit_mode)
        {
        if (m_framework->EditSelectNearestPoint(p,2) == CartoType::KErrorNone)
            {
            SetEditMode(true);
            return;
            }
        }

    if (m_edit_mode)
        SetEditMode(false);
    else
        {
        if (aEvent->modifiers() == Qt::ShiftModifier)
            {
            if (m_framework->EditNewPolygonObject(p) == CartoType::KErrorNone)
                {
                SetEditMode(true);
                m_writable_map_changed = true;
                }
            }
        else
            {
            if (m_framework->EditNewLineObject(p) == CartoType::KErrorNone)
                {
                SetEditMode(true);
                m_writable_map_changed = true;
                }
            }
        }
    }

void MapForm::mouseReleaseEvent(QMouseEvent* aEvent)
    {
    if (aEvent->button() == Qt::MouseButton::LeftButton)
        LeftButtonUp(aEvent->x(),aEvent->y(),aEvent->modifiers() == Qt::ShiftModifier);
    else if (aEvent->button() == Qt::MouseButton::RightButton)
        RightButtonUp(aEvent->x(),aEvent->y());
    }

void MapForm::mouseMoveEvent(QMouseEvent* aEvent)
    {
    if (m_edit_mode)
        {
        m_framework->EditMoveCurrentPoint(CartoType::TPointFP(aEvent->x(),aEvent->y()));
        ShowEditedObjectSize();
        m_writable_map_changed = true;
        update();
        return;
        }

    if (m_map_drag_enabled)
        {
        m_map_drag_offset.iX = aEvent->x() - m_map_drag_anchor.iX;
        m_map_drag_offset.iY = aEvent->y() - m_map_drag_anchor.iY;

        if (m_graphics_acceleration)
            {
            m_framework->Pan(m_map_drag_anchor_in_map_coords.iX,m_map_drag_anchor_in_map_coords.iY,CartoType::TCoordType::Map,
                             aEvent->x(),aEvent->y(),CartoType::TCoordType::Display);
            m_map_drag_offset = CartoType::TPoint(0,0);
            m_map_drag_anchor.iX = aEvent->x();
            m_map_drag_anchor.iY = aEvent->y();
            m_map_drag_anchor_in_map_coords = m_map_drag_anchor;
            m_framework->ConvertPoint(m_map_drag_anchor_in_map_coords.iX,m_map_drag_anchor_in_map_coords.iY,CartoType::TCoordType::Display,CartoType::TCoordType::Map);
            }

        update();
        }
    }

void MapForm::wheelEvent(QWheelEvent* aEvent)
    {
    auto delta = aEvent->angleDelta().y();
    if (delta == 0)
        return;

    // If the mouse pointer is in the window, pan the map so that the point under the mouse pointer stays where it is.
    CartoType::TRect r(0,0,width(),height());
    QPointF point = aEvent->position();
    bool set_centre = false;
    CartoType::TPoint p(CartoType::Round(point.x()),CartoType::Round(point.y()));
    if (r.Contains(p))
        set_centre = true;

    double zoom = sqrt(2);
    int zoom_count = delta / QWheelEvent::DefaultDeltasPerStep;
    if (zoom_count == 0)
        zoom_count = delta >= 0 ? 1 : -1;
    zoom = pow(zoom,zoom_count);

    if (set_centre)
        m_framework->ZoomAt(zoom,p.iX,p.iY,CartoType::TCoordType::Display);
    else
        m_framework->Zoom(zoom);

    update();
    }

void MapForm::keyPressEvent(QKeyEvent* aEvent)
    {
    if (aEvent->type() != QEvent::KeyPress)
        return;

    int pan_x = 0;
    int pan_y = 0;
    switch (aEvent->key())
        {
        case Qt::Key_Escape:
            SetEditMode(false);
            break;

        case Qt::Key_Backspace:
            if (m_edit_mode)
                {
                m_framework->EditDeleteCurrentPoint();
                ShowEditedObjectSize();
                m_writable_map_changed = true;
                SetEditMode(false);
                }
            break;

        case Qt::Key_Delete:
            if (m_edit_mode)
                {
                QMessageBox message_box(QMessageBox::Question,"Delete editable object","Really delete this editable object?",
                                        QMessageBox::Yes | QMessageBox::Cancel);
                QMessageBox::StandardButton button = (QMessageBox::StandardButton)message_box.exec();
                if (button == QMessageBox::Yes)
                    {
                    m_framework->EditDeleteCurrentObject();
                    m_writable_map_changed = true;
                    SetEditMode(false);
                    }
                }
            break;

        case Qt::Key_Up:
            pan_x = 0;
            pan_y = -32;
            break;

        case Qt::Key_Down:
            pan_x = 0;
            pan_y = 32;
            break;

        case Qt::Key_Left:
            pan_x = -32;
            pan_y = 0;
            break;

        case Qt::Key_Right:
            pan_x = 32;
            pan_y = 0;
            break;
        }

    if (m_edit_mode && (pan_x || pan_y))
        {
        m_framework->Pan(pan_x,pan_y);
        auto pos = mapFromGlobal(cursor().pos());
        m_framework->EditMoveCurrentPoint(CartoType::TPointFP(pos.x(),pos.y()));
        ShowEditedObjectSize();
        m_writable_map_changed = true;
        }

    update();
    }

void MapForm::closeEvent(QCloseEvent* aEvent)
    {
    if (m_writable_map_changed)
        {
        QMessageBox message_box(QMessageBox::Question,"Quit without saving","You have unsaved added data. Close without saving it?",
                                QMessageBox::Yes | QMessageBox::Cancel);
        QMessageBox::StandardButton button = (QMessageBox::StandardButton)message_box.exec();
        if (button == QMessageBox::Yes)
            {
            m_main_window.OnMapFormDestroyed(this);
            aEvent->accept();
            }
        else
            aEvent->ignore();
        }
    else
        {
        m_main_window.OnMapFormDestroyed(this);
        aEvent->accept();
        }
    }

void MapForm::initializeGL()
    {
    // Protect against calls by the QOpenGLWidget constructor in cases where the map could not be opened.
    if (!m_map_renderer)
        return;

    m_map_renderer->Init();
    }

void MapForm::paintGL()
    {
    // Protect against calls by the QOpenGLWidget constructor in cases where the map could not be opened.
    if (!m_map_renderer)
        return;

    m_map_renderer->Draw();
    }

void MapForm::StopDragging()
    {
    setMouseTracking(false);
    if (m_map_drag_enabled)
        {
        m_map_drag_enabled = false;
        PanToDraggedPosition();
        }
    m_map_drag_offset = CartoType::TPoint(0,0);
    update();
    }

void MapForm::PanToDraggedPosition()
    {
    CartoType::TPoint to(m_map_drag_anchor);
    to += m_map_drag_offset;
    m_framework->Pan(m_map_drag_anchor,to);
    update();
    }

const CartoType::TBitmap* MapForm::MapBitmap(CartoType::TResult& aError)
    {
    if (!m_framework)
        return nullptr;

    // Get the map bitmap.
    bool redraw_needed = false;
    const CartoType::TBitmap* bitmap = m_framework->MapBitmap(aError,&redraw_needed);

    /*
    If there is a legend, or there's a drag offset, copy the map bitmap into the extra GC,
    offset if necessary, and draw them on top.
    */
    if (!aError && (m_draw_legend || m_draw_scale || m_map_drag_offset.iX || m_map_drag_offset.iY))
        {
        // Create the extra GC if necessary.
        if (m_extra_gc == nullptr || m_extra_gc->Bounds() != CartoType::TRect(0,0,bitmap->Width(),bitmap->Height()))
            m_extra_gc = m_framework->CreateGraphicsContext(bitmap->Width(),bitmap->Height());

        if (!aError)
            {
            assert(bitmap->DataBytes() == m_extra_gc->Bitmap()->DataBytes());
            assert(bitmap->RowBytes() == m_extra_gc->Bitmap()->RowBytes());

            // Copy the bitmap into the extra GC's bitmap. */
            if (m_map_drag_offset.iX || m_map_drag_offset.iY)
                {
                // Set the destination bitmap to white so that the edges don't have relics of the previous map.
                memset(const_cast<uint8_t*>(m_extra_gc->Bitmap()->Data()),0xFF,m_extra_gc->Bitmap()->DataBytes());

                // Draw the source bitmap at the required offset.
                m_extra_gc->DrawBitmap(*bitmap,m_map_drag_offset);
                }
            else
                memcpy(const_cast<uint8_t*>(m_extra_gc->Bitmap()->Data()),bitmap->Data(),bitmap->DataBytes());

            // Draw the legend.
            if (m_draw_legend)
                m_framework->DrawNotices(*m_extra_gc);

            bitmap = m_extra_gc->Bitmap();
            }
        }

    return bitmap;
    }

void MapForm::CreateLegend()
    {
    auto legend = std::make_unique<CartoType::CLegend>(*m_framework);

    legend.reset(new CartoType::CLegend(*m_framework));
    static CartoType::TColor grey(90,90,90);
    legend->SetTextColor(grey);
    legend->SetAlignment(CartoType::TAlign::Center);
    CartoType::CString heading(m_framework->DataSetName());
    m_framework->SetCase(heading,CartoType::TLetterCase::Lower);
    m_framework->SetCase(heading,CartoType::TLetterCase::Title);
    legend->SetFontSize(10,"pt");
    legend->AddTextLine(heading);

    if (m_draw_legend)
        {
        legend->SetAlignment(CartoType::TAlign::Right);
        legend->SetFontSize(6,"pt");
        CartoType::CString s;
        s.SetAttribute("ref","M4");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Line,"road/major",nullptr,int32_t(CartoType::TRoadType::Motorway),s,"motorway");
        s.SetAttribute("ref","A40");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Line,"road/major",nullptr,int32_t(CartoType::TRoadType::Trunk),s,"trunk road");
        s.SetAttribute("ref","A414");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Line,"road/major",nullptr,int32_t(CartoType::TRoadType::Primary),s,"primary road");
        s.SetAttribute("ref","B4009");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Line,"road/mid",nullptr,int32_t(CartoType::TRoadType::Secondary),s,"secondary road");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Line,"road/mid",nullptr,int32_t(CartoType::TRoadType::Tertiary),"High Street","tertiary road");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Line,"road/minor",nullptr,int32_t(CartoType::TRoadType::Pedestrian),"","pedestrian road");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Line,"road/minor",nullptr,int32_t(CartoType::TRoadType::Track),"","track");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Line,"road/major",nullptr,int32_t(CartoType::TRoadType::Trunk) | CartoType::TRoadTypeFlag::TunnelFlag,"","road tunnel");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Line,"path","cyc",0,"","cycleway");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Line,"path","bri",0,"","bridle path");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Line,"path","foo",0,"","footpath");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Line,"path","ste",0,"","steps");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Polygon,"land/major","for",0,"Ashridge","forest or wood");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Polygon,"land/minor","par",0,"Green Park","park, golf course or common");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Polygon,"land/minor","gra",0,"","grassland");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Polygon,"land/minor","orc",0,"","orchard, vineyard, etc.");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Polygon,"land/minor","cmr",0,"","commercial or industrial");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Polygon,"land/minor","cns",0,"","construction, quarry, landfill, etc.");
        legend->AddMapObjectLine(CartoType::TMapObjectType::Point,"amenity/minor","stn",0,"Berkhamsted","station");
        }

    if (m_draw_scale)
        {
        legend->SetAlignment(CartoType::TAlign::Center);
        legend->SetFontSize(6,"pt");
        legend->AddScaleLine(m_metric_units);
        }

    legend->SetBorder(CartoType::KGray,1,4,"pt");
    static CartoType::TColor b(0xD0FFFFFF);
    legend->SetBackgroundColor(b);

    m_framework->SetLegend(std::move(legend),1,"in",CartoType::TNoticePosition::TopRight);
    }

void MapForm::EnableDrawLegend(bool aEnable)
    {
    if (aEnable != m_draw_legend)
        {
        m_draw_legend = aEnable;
        CreateLegend();
        m_framework->EnableLegend(m_draw_legend || m_draw_scale);
        update();
        }
    }

void MapForm::EnableDrawScale(bool aEnable)
    {
    if (aEnable != m_draw_scale)
        {
        m_draw_scale = aEnable;
        CreateLegend();
        m_framework->EnableLegend(m_draw_legend || m_draw_scale);
        update();
        }
    }

void MapForm::EnableDrawRotator(bool aEnable)
    {
    if (aEnable != m_draw_rotator)
        {
        m_draw_rotator = aEnable;
        if (m_draw_rotator)
            m_ui->dial->show();
        else
            m_ui->dial->hide();
        update();
        }
    }

void MapForm::DrawRange()
    {
    if (m_draw_range)
        {
        if (m_route_point_array[0].m_point != CartoType::TPointFP())
            {
            CartoType::TResult error = 0;
            CartoType::CGeometry g1 { m_framework->Range(error,nullptr,m_route_point_array[0].m_point.iX,m_route_point_array[0].m_point.iY,CartoType::TCoordType::Map,600,true) };
            CartoType::CGeometry g2 { m_framework->Range(error,nullptr,m_route_point_array[0].m_point.iX,m_route_point_array[0].m_point.iY,CartoType::TCoordType::Map,1200,true) };
            m_framework->InsertMapObject(0,"range",g1,"",0,m_range_id0,true);
            m_framework->InsertMapObject(0,"range",g2,"",0,m_range_id1,true);
            }
        }
    else
        {
        uint64_t count = 0;
        m_framework->DeleteMapObjects(0,m_range_id0,m_range_id0,count,"");
        m_framework->DeleteMapObjects(0,m_range_id1,m_range_id1,count,"");
        }
    update();
    }

void MapForm::EnableDrawRange(bool aEnable)
    {
    if (aEnable != m_draw_range)
        {
        m_draw_range = aEnable;
        DrawRange();
        }
    }

static CartoType::CString AreaOrLengthString(double aArea,double aLength,bool aMetricUnits)
    {
    CartoType::CString s;
    char buffer[64];
    if (aArea != 0)
        {
        if (aMetricUnits)
            {
            if (aArea > 1000000)
                sprintf(buffer,"area = %.2f sq km",aArea / 1000000);
            else
                sprintf(buffer,"area = %.2f ha",aArea / 10000);
            }
        else
            {
            // Convert to acres.
            double acres = aArea / 4046.856422;
            if (acres > 640)
                sprintf(buffer,"area = %.2f sq mi",acres / 640);
            else
                sprintf(buffer,"area = %.2f acres",acres);
            }
        s.Append(buffer);
        }
    else if (aLength != 0)
        {
        if (aMetricUnits)
            {
            if (aLength > 1000)
                sprintf(buffer,"length = %.2f km",aLength / 1000);
            else
                sprintf(buffer,"length = %.2f m",aLength);
            }
        else
            {
            // Convert to yards.
            double yards = aLength / 0.9144;
            if (yards > 1760)
                sprintf(buffer,"length = %.2f mi",yards / 1760);
            else
                sprintf(buffer,"length = %.2f yards",yards);
            }
        s.Append(buffer);
        }
    return s;
    }

void MapForm::ShowEditedObjectSize()
    {
    double area, length;
    auto error = m_framework->EditGetCurrentObjectAreaAndLength(area,length);
    if (error)
        return;

    CartoType::CString s = AreaOrLengthString(area,length,m_metric_units);
    QString qs;
    qs.setUtf16(s.Text(),int(s.Length()));
    m_main_window.statusBar()->showMessage(qs);
    }

void MapForm::LeftButtonDown(int32_t aX,int32_t aY)
    {
    if (m_edit_mode)
        {
        m_framework->EditAddCurrentPoint();
        m_writable_map_changed = true;
        return;
        }
    else
        {
        m_map_drag_enabled = true;
        m_map_drag_anchor.iX = aX;
        m_map_drag_anchor.iY = aY;
        m_map_drag_anchor_in_map_coords = m_map_drag_anchor;
        m_framework->ConvertPoint(m_map_drag_anchor_in_map_coords.iX,m_map_drag_anchor_in_map_coords.iY,CartoType::TCoordType::Display,CartoType::TCoordType::Map);
        setMouseTracking(true);
        }
    }

void MapForm::LeftButtonUp(int32_t aX,int32_t aY,bool aShift)
    {
    if (m_edit_mode)
        return;

    m_map_drag_offset.iX = aX - m_map_drag_anchor.iX;
    m_map_drag_offset.iY = aY - m_map_drag_anchor.iY;

    // Supply a simulated navigation fix.
    if (m_simulate_routing && aShift && m_map_drag_offset.iX == 0 && m_map_drag_offset.iY == 0)
        {
        CartoType::TNavigationData nav;
        nav.iValidity = CartoType::TNavigationData::KPositionValid | CartoType::TNavigationData::KTimeValid;
        nav.iPosition.iX = aX;
        nav.iPosition.iY = aY;
        m_on_route_time += 20;
        nav.iTime = m_on_route_time;
        m_framework->ConvertPoint(nav.iPosition.iX,nav.iPosition.iY,CartoType::TCoordType::Display,CartoType::TCoordType::Degree);
        m_framework->Navigate(nav);

        // Show the new navigation state in the status bar.
        CartoType::CString state = "navigation state: ";
        switch (m_framework->NavigationState())
            {
            case CartoType::TNavigationState::None: state += "None (no route has been created, or navigation is disabled)"; break;
            case CartoType::TNavigationState::NoPosition: state += "NoPosition (there is a route, and navigation is enabled, but no position has been supplied)"; break;
            case CartoType::TNavigationState::Turn: state += "Turn (the position is on the route and turn information is available)"; break;
            case CartoType::TNavigationState::OffRoute: state += "OffRoute (the position is off the route)"; break;
            case CartoType::TNavigationState::ReRouteNeeded: state += "ReRouteNeeded (a new route needs to be calculated)"; break;
            case CartoType::TNavigationState::ReRouteDone: state += "ReRouteDone (a new route has been calculated after a period off route)"; break;
            case CartoType::TNavigationState::TurnRound: state += "TurnRound (the position is on the route but a U-turn is needed)"; break;
            case CartoType::TNavigationState::Arrival: state += "Arrival (the position is on the route and very close to the destination)"; break;
            }
        QString state_qs;
        state_qs.setUtf16(state.Text(),(int)state.Length());
        m_main_window.statusBar()->showMessage(state_qs);
        }

    StopDragging();
    }

void MapForm::RightButtonDown(int32_t /*aX*/,int32_t /*aY*/)
    {

    }

void MapForm::RightButtonUp(int32_t aX,int32_t aY)
    {
    ShowRouteDialog(aX,aY);
    }

void MapForm::ShowRouteDialog(int32_t aX,int32_t aY)
    {
    CartoType::TPointFP p(aX,aY);

    // Get the address of the point clicked on.
    CartoType::CAddress address;
    m_framework->GetAddress(address,aX,aY,CartoType::TCoordType::Display);

    CartoType::CString address_string;
    m_framework->ConvertPoint(p.iX,p.iY,CartoType::TCoordType::Display,CartoType::TCoordType::Degree);

    address_string = address.ToString(false,&p);

    address_string.Append("\r\n\r\n[");
    address_string += address.ToStringWithLabels();
    address_string.Append("]");

    // Add the altitude if available.
    double x[] = { p.iX, p.iY };
    CartoType::TCoordSet cs(x,2);
    CartoType::TResult error;
    auto alt = m_framework->GetHeights(error,cs,CartoType::TCoordType::Degree);
    if (!error)
        {
        char buffer[32];
        sprintf(buffer," (altitude = %dm)",alt[0]);
        address_string.Append(buffer);
        }

    // If possible, get the Ordnance Survey National Grid reference.
    CartoType::CString os_grid_ref = CartoType::UKGridReferenceFromDegrees(p,3);
    if (os_grid_ref.Length())
        {
        address_string.Append("\r\n\r\nOrdnance Survey of Great Britain grid reference: ");
        address_string.Append(os_grid_ref);
        }

    // Get information about the objects clicked on - within 2 millimetres.
    CartoType::CMapObjectArray object;
    double pixel_mm = m_framework->ResolutionDpi() / 25.4;
    m_framework->FindInDisplay(object,100,aX,aY,int(ceil(2 * pixel_mm)));
    CartoType::CMapObject* pushpin = nullptr;

    // See if we have a pushpin, and geocode the objects so that they can be sorted.
    for (auto& p : object)
        {
        if (p->LayerName() == "pushpin" && p->Type() == CartoType::TMapObjectType::Point)
            pushpin = p.get();
        p->SetUserData((int)p->GetGeoCodeType());
        }

    // Sort objects on geocode and add information about them to the 'Address and nearby objects' field.
    if (object.size())
        {
        bool first_object = true;
        std::sort(object.begin(),object.end(),CartoType::MapObjectUserDataLessThan);
        int max_size = int(10000.0 / m_framework->MapUnitSize());
        for (auto& p : object)
            {
            // Skip objects with an bounding box wider or taller than 10 km.; avoids reporting entire nation, etc.
            if (p->CBoxBiggerThan(max_size))
                continue;
            // Skip unnamed map_extent and outline objects.
            if (!p->Label().Length())
                {
                if (p->LayerName() == "outline" || p->LayerName() == "map-extent")
                    continue;
                }
            if (first_object)
                {
                address_string.Append("\r\n\r\nNearby objects:\r\n");
                first_object = false;
                }
            address_string.Append(p->VerboseDescription().c_str());
            address_string.Append("\r\n");
            }
        }

    // Set up the route dialog.
    CartoType::CString pushpin_color = m_pushpin_color;
    uint16_t pushpin_glyph = m_pushpin_glyph;
    if (pushpin)
        {
        CartoType::TText c { pushpin->GetStringAttribute("_color") };
        if (c.Length())
            pushpin_color = c;
        CartoType::TText g { pushpin->GetStringAttribute("_iconText") };
        if (g.Length() == 1)
            pushpin_glyph = g[0];
        }
    RouteDialog route_dialog(*this,*m_framework,pushpin_color,pushpin_glyph);
    if (pushpin)
        {
        route_dialog.m_ui->addPushPin->setText("Edit pushpin");
        route_dialog.m_ui->addPushPin->setChecked(true);
        route_dialog.EnablePushPinEditing();
        }
    else
        {
        route_dialog.m_ui->cutPushPin->hide();
        if (m_route_point_array[0].m_point == CartoType::TPointFP())
            route_dialog.m_ui->setStartOfRoute->setChecked(true);
        else
            route_dialog.m_ui->setEndOfRoute->setChecked(true);
        }
    route_dialog.m_ui->visitWayPointsInAnyOrder->setChecked(m_best_route);

    QString qs;
    SetString(qs,address_string);
    route_dialog.m_ui->address->setPlainText(qs);

    uint64_t pushpin_id = 0;
    if (pushpin)
        {
        CartoType::TText value = pushpin->GetStringAttribute("");
        SetString(m_pushpin_name,value);
        value = pushpin->GetStringAttribute("desc");
        SetString(m_pushpin_desc,value);
        pushpin_id = pushpin->Id();
        }
    else
        {
        SetString(m_pushpin_name,address.ToString(false));
        SetString(m_pushpin_desc,address.ToString(true));
        }
    route_dialog.m_ui->pushPinName->setText(m_pushpin_name);
    route_dialog.m_ui->pushPinDesc->setPlainText(m_pushpin_desc);

    // Display the route dialog.
    if (route_dialog.exec() != QDialog::DialogCode::Accepted)
        return;

    bool create_route = false;
    m_best_route = route_dialog.m_ui->visitWayPointsInAnyOrder->isChecked();
    if (route_dialog.m_ui->setStartOfRoute->isChecked()) // set start of route
        {
        CartoType::TPointFP p(aX,aY);
        m_framework->ConvertPoint(p.iX,p.iY,CartoType::TCoordType::Display,CartoType::TCoordType::Map);
        m_route_point_array[0].m_point = p;
        DrawRange();
        create_route = true;
        }
    else if (route_dialog.m_ui->setEndOfRoute->isChecked()) // set end of route
        {
        CartoType::TPointFP p(aX,aY);
        m_framework->ConvertPoint(p.iX,p.iY,CartoType::TCoordType::Display,CartoType::TCoordType::Map);
        m_route_point_array[m_route_point_array.size() - 1].m_point = p;
        create_route = true;
        }
    else if (route_dialog.m_ui->addPushPin->isChecked()) // add or change a pushpin
        {
        m_pushpin_name = route_dialog.m_ui->pushPinName->text();
        m_pushpin_desc = route_dialog.m_ui->pushPinDesc->toPlainText();
        m_pushpin_color = route_dialog.PushPinColor();
        m_pushpin_glyph = route_dialog.PushPinGlyph();

        CartoType::CString string_attrib;
        string_attrib.Set(m_pushpin_name.utf16());
        if (m_pushpin_desc.length())
            string_attrib.SetAttribute("desc",m_pushpin_desc.utf16());
        string_attrib.SetAttribute("_color",m_pushpin_color);
        double x = 0,y = 0;
        if (pushpin)
            {
            CartoType::TRect r { pushpin->CBox() };
            x = r.iTopLeft.iX;
            y = r.iTopLeft.iY;
            }
        else
            {
            x = aX;
            y = aY;
            m_framework->ConvertPoint(x,y,CartoType::TCoordType::Display,CartoType::TCoordType::Map);
            }
        if (m_pushpin_glyph)
            {
            CartoType::TText t(&m_pushpin_glyph,1);
            string_attrib.SetAttribute("_iconText",t);
            }
        m_framework->InsertPointMapObject(m_writable_map_handle,"pushpin",x,y,CartoType::TCoordType::Map,string_attrib,0,pushpin_id,true);
        m_main_window.UpdateSaveAddedData();
        m_main_window.UpdateDeletePushpins();

        if (route_dialog.m_ui->useAsWayPoint->isChecked())
            {
            TRoutePoint* waypoint = nullptr;

            for (int i = 1; waypoint == nullptr && i < (int)m_route_point_array.size() - 1; i++)
                if (m_route_point_array[i].m_id == pushpin_id)
                    waypoint = &m_route_point_array[i];
            if (waypoint == nullptr)
                {
                TRoutePoint rp;
                m_route_point_array.insert(m_route_point_array.begin() + m_route_point_array.size() - 1,rp);
                waypoint = &m_route_point_array[m_route_point_array.size() - 2];
                }
            if (waypoint)
                {
                waypoint->m_point.iX = x;
                waypoint->m_point.iY = y;
                waypoint->m_id = pushpin_id;
                create_route = true;
                }
            }

        m_writable_map_changed = true;
        }
    else if (route_dialog.m_ui->cutPushPin->isChecked()) // cut a pushpin
        {
        uint64_t count = 0;
        m_framework->DeleteMapObjects(m_writable_map_handle,pushpin_id,pushpin_id,count);
        for (int i = 1; i < (int)m_route_point_array.size() - 1; i++)
            if (m_route_point_array[i].m_id == pushpin_id)
                {
                m_route_point_array.erase(m_route_point_array.begin() + i);
                create_route = true;
                break;
                }
        m_writable_map_changed = true;
        m_main_window.UpdateSaveAddedData();
        m_main_window.UpdateDeletePushpins();
        }

    if (create_route)
        CalculateAndDisplayRoute();

    update();
    }

void MapForm::CalculateAndDisplayRoute()
    {
    // Do nothing if either of the route points are (0,0), which by convention is the illegal position.
    if (m_route_point_array[0].m_point == CartoType::TPointFP() ||
        m_route_point_array[m_route_point_array.size() - 1].m_point == CartoType::TPointFP())
        return;

    std::vector<CartoType::TPointFP> p;
    for (const auto& q : m_route_point_array)
        p.push_back(q.m_point);
    CartoType::TCoordSet cs(p);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    CartoType::TResult error = 0;
    CartoType::TRouteProfile profile(*m_framework->Profile(0));
    if (m_best_route && cs.Count() > 3)
        {
        size_t iterations = cs.Count() * cs.Count();
        if (iterations < 16)
            iterations = 16;
        else if (iterations > 256)
            iterations = 256;
        auto r = m_framework->CreateBestRoute(error,profile,cs,CartoType::TCoordType::Map,true,true,iterations);
        if (!error)
            error = m_framework->UseRoute(*r,true);
        }
    else
        {
        auto r = m_framework->CreateRoute(error,profile,cs,CartoType::TCoordType::Map);
        if (!error)
            error = m_framework->UseRoute(*r,true);
        }
    QApplication::restoreOverrideCursor();

    if (error)
        m_main_window.ShowError("Routing error",error);

    const CartoType::CRoute* r = m_framework->Route();
    if (r)
        {
        CartoType::CString text;
        m_framework->AppendDistance(text,r->iDistance,m_metric_units);
        text += " (";
        m_framework->AppendTime(text,r->iTime);
        text += ")";
        char buffer[256];
        auto data = m_framework->RouteCreationData();
        sprintf(buffer," [calculated in %gs, expanded in %gs]",data.iRouteCalculationTime,data.iRouteExpansionTime);
        text += buffer;

        QString text_qs;
        text_qs.setUtf16(text.Text(),(int)text.Length());
        m_main_window.statusBar()->showMessage(text_qs);
        }

    m_main_window.UpdateManageRoute();
    }

void MapForm::Find()
    {
    m_find_dialog.Set(*m_framework);
    QString qs;
    qs.setUtf16(m_find_text.Text(),(int)m_find_text.Length());
    m_find_dialog.m_ui->findText->setText(qs);
    m_find_dialog.m_ui->findText->setFocus();

    m_found_object_index = 0;
    while (m_find_dialog.exec() == QDialog::DialogCode::Accepted)
        {
        m_find_text.Set(m_find_dialog.m_ui->findText->text().utf16());
        if (m_find_text.Length() == 0)
            continue;

        m_found_object = m_find_dialog.FoundObjectArray();
        if (m_found_object.size() == 0)
            {
            std::string s("Text ");
            s += std::string(m_find_text) + " not found";
            continue;
            }

        uint64_t count = 0;
        m_framework->DeleteMapObjects(0,m_found_object_id,m_found_object_id,count);
        m_found_object_id = 0;
        ShowNextFoundObject();
        break;
        }

    m_main_window.UpdateFindNext();
    }

void MapForm::FindAddress()
    {
    FindAddressDialog find_dialog;

    SetString(*find_dialog.m_ui->building,m_address.iBuilding);
    SetString(*find_dialog.m_ui->feature,m_address.iFeature);
    SetString(*find_dialog.m_ui->street,m_address.iStreet);
    SetString(*find_dialog.m_ui->subLocality,m_address.iSubLocality);
    SetString(*find_dialog.m_ui->locality,m_address.iLocality);
    SetString(*find_dialog.m_ui->island,m_address.iIsland);
    SetString(*find_dialog.m_ui->subAdminArea,m_address.iSubAdminArea);
    SetString(*find_dialog.m_ui->adminArea,m_address.iAdminArea);
    SetString(*find_dialog.m_ui->country,m_address.iCountry);
    SetString(*find_dialog.m_ui->postCode,m_address.iPostCode);

    bool stop = false;

    m_found_object_index = 0;
    while (!stop)
        {
        if (find_dialog.exec() == QDialog::DialogCode::Accepted)
            {
            m_found_object.clear();
            SetString(m_address.iBuilding,*find_dialog.m_ui->building);
            SetString(m_address.iFeature,*find_dialog.m_ui->feature);
            SetString(m_address.iStreet,*find_dialog.m_ui->street);
            SetString(m_address.iSubLocality,*find_dialog.m_ui->subLocality);
            SetString(m_address.iLocality,*find_dialog.m_ui->locality);
            SetString(m_address.iIsland,*find_dialog.m_ui->island);
            SetString(m_address.iSubAdminArea,*find_dialog.m_ui->subAdminArea);
            SetString(m_address.iAdminArea,*find_dialog.m_ui->adminArea);
            SetString(m_address.iCountry,*find_dialog.m_ui->country);
            SetString(m_address.iPostCode,*find_dialog.m_ui->postCode);

            CartoType::TResult error = m_framework->FindAddress(m_found_object,100,m_address,true);
            if (error || m_found_object.size() == 0)
                {
                m_main_window.ShowError("address not found",error);
                if (error)
                    stop = true;
                }
            else
                {
                ShowNextFoundObject();
                stop = true;
                }
            }
        else
            stop = true;
        }

    m_main_window.UpdateFindNext();
    }

void MapForm::FindNext()
    {
    ShowNextFoundObject();
    }

void MapForm::ShowNextFoundObject()
    {
    auto& object = m_found_object[m_found_object_index];

    CartoType::TResult error;
    CartoType::TTextLiteral(found_layer,u"found");
    if (object->Type() == CartoType::TMapObjectType::Line)
        {
        auto envelope = object->Envelope(5.0 / m_framework->MapUnitSize());
        error = m_framework->InsertMapObject(0,found_layer,CartoType::CGeometry(envelope,CartoType::TCoordType::Map,true),"",0,m_found_object_id,true);
        }
    else
        {
        error = m_framework->InsertCopyOfMapObject(0,found_layer,*object,object->Type() == CartoType::TMapObjectType::Point ? 50 : 0,
                                                  CartoType::TCoordType::MapMeter,m_found_object_id,true);
        }

    if (!error)
        {
        bool animate = m_framework->SetAnimateTransitions(true);
        m_framework->SetView(*object,64,4000);
        m_framework->SetAnimateTransitions(animate);
        }
    if (error)
        return;

    CartoType::CString summary;
    m_framework->GeoCodeSummary(summary,*object);
    double length = object->LengthOrPerimeter(error);
    double area = 0;
    if (!error)
        area = object->Area(error);
    if (!error && (area || length))
        {
        summary += "; ";
        summary += AreaOrLengthString(area,length,m_metric_units);
        }
    QString summary_qs;
    summary_qs.setUtf16(summary.Text(),(int)summary.Length());
    m_main_window.statusBar()->showMessage(summary_qs);

    m_found_object_index = (m_found_object_index + 1) % m_found_object.size();
    update();
    }

void MapForm::on_dial_valueChanged(int aValue)
    {
    m_framework->SetRotation(double(aValue) / 100.0);
    m_main_window.UpdateNorthUp();
    update();
    }

void MapForm::SetRotation(double aAngle)
    {
    m_framework->SetRotation(aAngle);
    m_ui->dial->setValue(int(aAngle * 100));
    update();
    }

void MapForm::ReverseRoute()
    {
    CartoType::TResult error = m_framework->ReverseRoutes();
    if (!error)
        {
        std::reverse(m_route_point_array.begin(),m_route_point_array.end());
        update();
        }
    }

void MapForm::DeleteRoute()
    {
    m_route_point_array.resize(2);
    m_route_point_array[0] = m_route_point_array[1] = TRoutePoint();
    m_framework->DeleteRoutes();
    m_main_window.UpdateManageRoute();
    update();
    }

void MapForm::DeletePushpins()
    {
    uint64_t deleted_count = 0;
    m_framework->DeleteMapObjects(m_writable_map_handle,0,UINT64_MAX,deleted_count,"@layer=='pushpin'");
    m_main_window.UpdateDeletePushpins();
    update();
    }

void MapForm::SetSimulateRouting(bool aSet)
    {
    m_simulate_routing = aSet;
    m_framework->EnableTurnInstructions(aSet);
    m_framework->ForceRedraw();
    update();
    }

bool MapForm::HasPushpins() const
    {
    CartoType::CMapObjectArray object_array;
    CartoType::TFindParam param;
    param.iMaxObjectCount = 1;
    param.iLayers = "pushpin";
    param.iMerge = false;
    m_framework->Find(object_array,param);
    return object_array.size() > 0;
    }

size_t MapForm::BuiltInProfileCount()
    {
    return m_framework->BuiltInProfileCount();
    }

const CartoType::TRouteProfile* MapForm::BuiltInProfile(size_t aIndex)
    {
    return m_framework->BuiltInProfile(aIndex);
    }

void MapForm::SetRouteProfileIndex(size_t aIndex)
    {
    if (aIndex == m_route_profile_index)
        return;

    m_route_profile_index = aIndex;
    if (m_framework->BuiltInProfileCount())
        m_framework->SetBuiltInProfile(aIndex);
    else if (aIndex == 4)
        m_framework->SetMainProfile(m_main_window.CustomRouteProfile());
    else
        {
        auto type = CartoType::TRouteProfileType::Car;
        switch (aIndex)
            {
            case 1: type = CartoType::TRouteProfileType::Cycle; break;
            case 2: type = CartoType::TRouteProfileType::Walk; break;
            case 3: type = CartoType::TRouteProfileType::Hike; break;
            default: break;
            }
        m_framework->SetMainProfile(type);
        }
    CalculateAndDisplayRoute();
    if (m_draw_range)
        DrawRange();
    m_main_window.UpdateRouteProfile();
    update();
    }

void MapForm::SetPreferredRouterType(CartoType::TRouterType aRouterType)
    {
    if (aRouterType != m_framework->PreferredRouterType())
        {
        m_framework->SetPreferredRouterType(aRouterType);
        m_framework->LoadNavigationData();
        CalculateAndDisplayRoute();
        update();
        }
    }

void MapForm::ViewRouteInstructions()
    {
    const CartoType::CRoute* route = m_framework->Route();
    if (!route)
        return;

    RouteInstructionsDialog dialog(this);
    CartoType::CString s(m_framework->RouteInstructions(*route));
    QString instructions;
    instructions.setUtf16(s.Text(),(int)s.Length());
    dialog.m_ui->text->setPlainText(instructions);
    dialog.exec();
    }

bool MapForm::EditCustomRouteProfile(CartoType::TRouteProfile& aRouteProfile)
    {
    RouteProfileDialog dialog(this,aRouteProfile,m_metric_units);
    if  (dialog.exec() == QDialog::DialogCode::Accepted)
        {
        aRouteProfile = dialog.RouteProfile();
        if (m_framework->BuiltInProfileCount() == 0 && m_route_profile_index == 4) // if using the custom route profile
            {
            m_framework->SetMainProfile(aRouteProfile);
            if (m_route_point_array[0].m_point != CartoType::TPointFP() &&
                m_route_point_array[m_route_point_array.size() - 1].m_point != CartoType::TPointFP())
                CalculateAndDisplayRoute();
            }
        return true;
        }
    return false;
    }

void MapForm::SaveRouteInstructions()
    {
    if (!m_framework->Route())
        return;

    std::string filename { GetSaveFile(*this,"Save route instructions","text files","txt") };
    if (filename.empty())
        return;

    CartoType::TResult error = 0;
    FILE* file = fopen(filename.c_str(),"w");
    if (!file)
        error = CartoType::KErrorIo;
    if (!error)
        {
        std::string text { m_framework->RouteInstructions(*m_framework->Route()) };
        size_t n = fwrite(text.c_str(),text.length(),1,file);
        if (!n)
            error = CartoType::KErrorIo;
        }
    if (file)
        fclose(file);
    if (error)
        m_main_window.ShowError("failed to save the route instructions to a text file",error);
    }

void MapForm::SaveRouteAsXml()
    {
    if (!m_framework->Route())
        return;

    CartoType::CString path { GetSaveFile(*this,"Save Route","CartoType route files","ctroute") };
    if (!path.Length())
        return;
    CartoType::TResult error = m_framework->WriteRouteAsXml(*m_framework->Route(),path);
    if (error)
        m_main_window.ShowError("failed to save the route to a CTROUTE file",error);
    }

void MapForm::SaveRouteAsGpx()
    {
    if (!m_framework->Route())
        return;

    CartoType::CString path { GetSaveFile(*this,"Save route as GPX","GPX files","gpx") };
    if (!path.Length())
        return;

    CartoType::TResult error = m_framework->WriteRouteAsXml(*m_framework->Route(),path,CartoType::TFileType::GPX);
    if (error)
        m_main_window.ShowError("failed to save the route to a GPX file",error);
    }

void MapForm::LoadRouteFromXml()
    {
    QString path = QFileDialog::getOpenFileName(this,"Load route","","GPX files or CartoType route files (*.gpx *.ctroute)");
    if (!path.length())
        return;
    CartoType::TResult error = m_framework->ReadRouteFromXml(path.utf16(),true);
    if (error)
        m_main_window.ShowError("failed to load a route from a CTROUTE file",error);
    else
        {
        m_route_point_array.clear();
        const CartoType::CRoute* r = m_framework->Route();
        if (r)
            {
            size_t sections = 0;
            int prev_section = -1;
            for (const auto& s : r->iRouteSegment)
                {
                if (s->iSection != prev_section)
                    {
                    sections++;
                    prev_section = s->iSection;
                    if (s->iPath.Points())
                        {
                        TRoutePoint p;
                        p.m_point = s->iPath.Point(0);
                        m_route_point_array.push_back(p);
                        }
                    }
                }
            if (r->iRouteSegment.size())
                {
                const auto& s = r->iRouteSegment.back();
                if (s->iPath.Points())
                    {
                    TRoutePoint p;
                    p.m_point = s->iPath.Point(s->iPath.Points() - 1);
                    m_route_point_array.push_back(p);
                    }
                }
            }
        if (m_route_point_array.size() < 2)
            {
            m_route_point_array.resize(2);
            m_route_point_array[0] = m_route_point_array[1] = TRoutePoint();
            }
        m_main_window.UpdateManageRoute();
        update();
        }
    }

void MapForm::SaveImageAsPng()
    {
    CartoType::CString path = GetSaveFile(*this,"Save image as PNG","PNG files","png");
    if (!path.Length())
        return;
    auto error = m_framework->WriteMapImage(path,CartoType::TFileType::PNG,true);
    if (error)
        m_main_window.ShowError("failed to save the image as a PNG file",error);
    }

bool MapForm::HasWritableData() const
    {
    return !m_framework->MapIsEmpty(m_writable_map_handle);
    }

void MapForm::SaveWritableDataAsCtms()
    {
    CartoType::CString path { GetSaveFile(*this,"Save added data as CTMS","CartoType serialized map files","ctms") };
    if (!path.Length())
        return;
    CartoType::TResult error = m_framework->SaveMap(m_writable_map_handle,path,CartoType::TFileType::CTMS);
    if (error)
        m_main_window.ShowError("failed to save data to a CTMS file",error);
    else
        m_writable_map_changed = false;
    }

void MapForm::LoadWritableDataFromCtms()
    {
    QString qs = QFileDialog::getOpenFileName(this,"Import data from a CTMS file","","CartoType serialized map files (*.ctms)");
    CartoType::CString filename;
    filename.Set(qs.utf16(),qs.length());
    CartoType::TResult error = m_framework->ReadMap(m_writable_map_handle,filename,CartoType::TFileType::CTMS);
    if (error)
        m_main_window.ShowError("failed to read data from a CTMS file",error);
    m_main_window.UpdateSaveAddedData();
    }

void MapForm::LoadWritableDataFromGpx()
    {
    QString filename = QFileDialog::getOpenFileName(this,"Import data from a GPX file","","GPX files (*.gpx)");
    CartoType::TResult error = m_framework->ReadGpx(m_writable_map_handle,filename.utf16());
    if (error)
        m_main_window.ShowError("failed to read data from a GPX file",error);
    m_main_window.UpdateSaveAddedData();
    }

void MapForm::SetScale()
    {
    ScaleDialog scale_dialog(this);
    QString scale_qs;
    scale_qs.setNum((int)m_framework->GetScaleDenominatorInView());
    scale_dialog.m_ui->scale->setText(scale_qs);
    if (scale_dialog.exec() == QDialog::DialogCode::Accepted)
        {
        scale_qs = scale_dialog.m_ui->scale->text();
        int new_scale = scale_qs.toInt();
        m_framework->SetScaleDenominatorInView(new_scale);
        update();
        }
    }

void MapForm::GoToLocation(double aLong,double aLat)
    {
    double minx,miny,maxx,maxy;
    m_framework->GetMapExtent(minx,miny,maxx,maxy,CartoType::TCoordType::Degree);
    if (aLong < minx)
        aLong = minx;
    else if (aLong > maxx)
        aLong = maxx;
    if (aLat < miny)
        aLat = miny;
    else if (aLat > maxy)
        aLat = maxy;

    m_found_object.clear();
    m_found_object_index = 0;
    CartoType::TTextLiteral(found_layer,u"found");
    m_framework->InsertCircleMapObject(0,found_layer,
                                       aLong,aLat,CartoType::TCoordType::Degree,50,CartoType::TCoordType::MapMeter,
                                       nullptr,0,m_found_object_id,true);
    m_framework->SetScaleDenominator(25000);
    m_framework->SetViewCenter(aLong,aLat,CartoType::TCoordType::Degree);
    update();
    }

void MapForm::GoToLocation()
    {
    LocationDialog location_dialog(this);
    double minx,miny,maxx,maxy;
    m_framework->GetView(minx,miny,maxx,maxy,CartoType::TCoordType::Degree);
    double cur_long = (minx + maxx) / 2;
    double cur_lat = (miny + maxy) / 2;
    QString long_qs;
    long_qs.setNum(cur_long);
    location_dialog.m_ui->longitude->setText(long_qs);
    QString lat_qs;
    lat_qs.setNum(cur_lat);
    location_dialog.m_ui->latitude->setText(lat_qs);
    if (location_dialog.exec() == QDialog::DialogCode::Accepted)
        {
        double new_long = location_dialog.m_ui->longitude->text().toDouble();
        double new_lat = location_dialog.m_ui->latitude->text().toDouble();
        GoToLocation(new_long,new_lat);
        }
    }

void MapForm::GoToOrdnanceSurveyGridRef()
    {
    LocationDialog location_dialog(this);
    location_dialog.m_ui->lat_label->hide();
    location_dialog.m_ui->latitude->hide();
    location_dialog.m_ui->long_label->setText("grid reference");
    location_dialog.setWindowTitle("Go to Grid Reference");

    double minx,miny,maxx,maxy;
    m_framework->GetView(minx,miny,maxx,maxy,CartoType::TCoordType::Degree);
    double cur_long = (minx + maxx) / 2;
    double cur_lat = (miny + maxy) / 2;
    CartoType::TPointFP p(cur_long,cur_lat);
    CartoType::CString ref = CartoType::UKGridReferenceFromDegrees(p,3);
    QString ref_qs;
    ref_qs.setUtf16(ref.Text(),(int)ref.Length());
    location_dialog.m_ui->longitude->setText(ref_qs);
    if (location_dialog.exec() == QDialog::DialogCode::Accepted)
        {
        ref_qs = location_dialog.m_ui->longitude->text();
        ref.Set(ref_qs.utf16());
        p = CartoType::PointInDegreesFromUKGridReference(ref);
        if (p.iX != 0 && p.iY != 0)
            GoToLocation(p.iX,p.iY);
        }
    }

void MapForm::ChooseStyleSheet()
    {
    QString qs = QFileDialog::getOpenFileName(this,"Choose Style Sheet","","CartoType style sheets (*.ctstyle *.xml)");
    CartoType::CString filename;
    filename.Set(qs.utf16(),qs.length());
    if (filename.Length())
        {
        CartoType::TResult error = m_framework->SetStyleSheet(filename);
        if (error)
            m_main_window.ShowError("error loading stylesheet",error);
        else
            {
            CreateLegend();
            update();
            }
        }
    }

void MapForm::ReloadStyleSheet()
    {
    m_framework->ReloadStyleSheet();
    CreateLegend();
    update();
    }

void MapForm::ApplyStyleSheet(const std::string& aStyleSheet)
    {
    if (aStyleSheet.length())
        {
        m_main_window.SetCustomStyleSheet(aStyleSheet);

        if (!m_use_custom_style_sheet)
            SetUseCustomStyleSheet(true);
        else
            {
            m_framework->SetStyleSheet((const uint8_t*)aStyleSheet.data(),aStyleSheet.length());
            CreateLegend();
            update();
            }
        m_style_sheet_applied = true;
        }
    }

void MapForm::FinishEditingCustomStyleSheet(int /*aResult*/)
    {
    m_style_sheet_applied = false;
    }

void MapForm::ChooseLayers()
    {
    LayerDialog layer_dialog(this);
    std::vector<CartoType::CString> layers { m_framework->LayerNames() };
    for (const auto& layer_name : layers)
        {
        QString qs_name; qs_name.setUtf16(layer_name.Text(),(int)layer_name.Length());
        QListWidgetItem* item = new QListWidgetItem(qs_name);
        if (m_framework->LayerIsEnabled(layer_name))
            item->setCheckState(Qt::Checked);
        else
            item->setCheckState(Qt::Unchecked);
        layer_dialog.m_ui->list->addItem(item);
        }
    if (layer_dialog.exec() == QDialog::DialogCode::Accepted)
        {
        for (int i = 0; i < layer_dialog.m_ui->list->count(); i++)
            {
            QString qs_name = layer_dialog.m_ui->list->item(i)->text();
            CartoType::CString layer_name; layer_name.Set(qs_name.utf16());
            bool checked = layer_dialog.m_ui->list->item(i)->checkState() == Qt::Checked;
            m_framework->EnableLayer(layer_name,checked);
            }
        update();
        }
    }

void MapForm::SetMetricUnits(bool aEnable)
    {
    if (aEnable != m_metric_units)
        {
        m_metric_units = aEnable;
        m_framework->SetLocale(m_metric_units ? "en_xx" : "en");
        CreateLegend();
        if (DrawScaleEnabled())
            update();
        }
    }

void MapForm::SetNightMode(bool aSet)
    {
    m_framework->SetNightMode(aSet);
    CreateLegend();
    update();
    }

bool MapForm::NightMode() const
    {
    return m_framework->NightMode();
    }

void MapForm::SetGraphicsAcceleration(bool aEnable)
    {
    if (aEnable != m_graphics_acceleration)
        {
        m_graphics_acceleration = aEnable;
        if (m_graphics_acceleration)
            {
            connect(&m_graphics_acceleration_update_timer,SIGNAL(timeout()),this,SLOT(update()));
            m_graphics_acceleration_update_timer.start(33);
            }
        else
            m_graphics_acceleration_update_timer.stop();
        update();
        }
    }

void MapForm::on_perspective_slider_valueChanged(int aValue)
    {
    CartoType::TPerspectiveParam p;
    p.iDeclinationDegrees = m_perspective_angle = aValue;
    p.iFieldOfViewDegrees = 38;
    m_framework->SetPerspective(p);
    update();
    }

void MapForm::SetPerspective(bool aEnable)
    {
    if (aEnable != m_framework->Perspective())
        {
        if (aEnable)
            {
            CartoType::TPerspectiveParam p;
            p.iDeclinationDegrees = m_perspective_angle;
            p.iFieldOfViewDegrees = 38;
            m_framework->SetPerspective(p);
#ifndef Q_OS_MAC // vertical sliders don't work properly on Mac: see QTBUG-57194: QSlider in orientation vertical fill display inverted on Mac X.12
            m_ui->perspective_slider->show();
#endif
            }
        else
            {
            m_framework->SetPerspective(false);
            m_ui->perspective_slider->hide();
            }
        update();
        }
    }

bool MapForm::Perspective() const
    {
    return m_framework->Perspective();
    }

void MapForm::SetDraw3DBuildings(bool aEnable)
    {
    if (aEnable != m_framework->Draw3DBuildings())
        {
        m_framework->SetDraw3DBuildings(aEnable);
        update();
        }
    }

bool MapForm::Draw3DBuildings() const
    {
    return m_framework->Draw3DBuildings();
    }

bool MapForm::MapIncludesGreatBritain() const
    {
    CartoType::TRectFP bounds;
    m_framework->GetMapExtent(bounds,CartoType::TCoordType::Degree);
    static CartoType::TRectFP gb(-10,49,4,62);
    bounds.Intersection(gb);
    return !bounds.IsEmpty();
    }

void MapForm::SetUseCustomStyleSheet(bool aEnable)
    {
    if (m_use_custom_style_sheet == aEnable)
        return;
    m_use_custom_style_sheet = aEnable;
    m_main_window.UpdateStyleSheet();
    if (aEnable)
        {
        m_saved_style_sheet = m_framework->GetStyleSheetData(0);

        // If there's no custom style sheet, copy the current style sheet into it.
        if (m_main_window.CustomStyleSheet().empty())
            m_main_window.SetCustomStyleSheet(m_framework->GetStyleSheetText(0));

        const std::string& style = m_main_window.CustomStyleSheet();
        m_framework->SetStyleSheet((const uint8_t*)style.data(),style.length());
        }
    else
        m_framework->SetStyleSheet(m_saved_style_sheet);
    CreateLegend();
    update();
    }

void MapForm::OnCustomRouteProfileLoaded()
    {
    m_framework->SetMainProfile(m_main_window.CustomRouteProfile());
    CalculateAndDisplayRoute();
    }

void MapForm::OnCustomStyleSheetLoaded()
    {
    if (m_use_custom_style_sheet)
        {
        const std::string& style = m_main_window.CustomStyleSheet();
        m_framework->SetStyleSheet((const uint8_t*)style.data(),style.length());
        }
    }

void MapForm::Print(bool aPreview)
    {
    QPrinter printer(QPrinter::HighResolution);

    if (!aPreview)
        {
        QPrintDialog dialog(&printer,this);
        dialog.setWindowTitle(tr("Print Map"));
        if (dialog.exec() != QDialog::Accepted)
            return;
        }

    CartoType::TResult error = 0;
    std::unique_ptr<CartoType::CFramework> print_framework { m_framework->Copy(error) };
    if (error)
        {
        m_main_window.ShowError("failed to make a CartoType::CFramework copy for printing",error);
        return;
        }
    print_framework->SetResolutionDpi(printer.resolution());
    QRectF page_rect { printer.pageRect(QPrinter::DevicePixel) };

    auto CreateImage = [&](QRectF aPageRect)->QImage
        {
        print_framework->Resize(int(aPageRect.width()),int(aPageRect.height()));
        print_framework->SetScaleDenominator(m_framework->ScaleDenominator());
        CartoType::TRectFP view;
        m_framework->GetView(view,CartoType::TCoordType::Map);
        print_framework->SetViewCenter((view.Left() + view.Right()) / 2,(view.Top() + view.Bottom()) / 2,CartoType::TCoordType::Map);
        print_framework->SetRotation(m_framework->Rotation());
        print_framework->SetScaleDenominatorInView(m_framework->GetScaleDenominatorInView());

        const CartoType::TBitmap* map_bitmap = print_framework->MapBitmap(error);
        if (error)
            {
            m_main_window.ShowError("failed to draw the map for printing",error);
            return QImage();
            }

        CartoType::CLegend legend(*print_framework);
        legend.SetAlignment(CartoType::TAlign::Center);
        legend.SetFontSize(6,"pt");
        legend.SetTextColor(CartoType::KDarkGray);
        legend.AddScaleLine(m_metric_units);
        CartoType::TColor cartotype_red(178,31,41);
        legend.SetTextColor(cartotype_red);
        legend.SetFontFamily("DejaVu Serif");
        legend.AddTextLine("");
        legend.AddTextLine("CartoType                    www.cartotype.com");
        legend.SetBorder(CartoType::KGray,1,4,"pt");
        CartoType::TColor b(CartoType::KWhite);
        b.SetAlpha(200);
        legend.SetBackgroundColor(b);
        auto bs = m_framework->BlendStyleSet();
        CartoType::TResult error;
        auto legend_bitmap = legend.CreateLegend(error,2,"in",std::max((int)print_framework->ScaleDenominator(),8000),print_framework->GetScaleDenominatorInView(),&bs);
        if (!error)
            {
            auto gc = print_framework->GetMapGraphicsContext();
            gc->DrawBitmap(*legend_bitmap,CartoType::TPoint(aPageRect.width() - legend_bitmap->Width() - 16,aPageRect.height() - legend_bitmap->Height() - 16));
            }

        QImage image { int(aPageRect.width()),int(aPageRect.height()),QImage::Format_ARGB32_Premultiplied };
        CopyBitmapToImage(*map_bitmap,image);
        return image;
        };
    QImage image { CreateImage(page_rect) };
    if (image.isNull())
        return;

    auto Paint = [&image,&page_rect,&CreateImage](QPrinter* aPrinter)->void
        {
        QRectF cur_page_rect { aPrinter->pageRect(QPrinter::DevicePixel) };
        if (cur_page_rect != page_rect)
            {
            image = CreateImage(cur_page_rect);
            if (image.isNull())
                return;
            page_rect = cur_page_rect;
            }

        QPainter painter;
        painter.begin(aPrinter);
        painter.drawImage(0,0,image);
        painter.end();
        };

    if (aPreview)
        {
        QPrintPreviewDialog preview(&printer,this);
        connect(&preview,&QPrintPreviewDialog::paintRequested,Paint);
        preview.exec();
        }
    else
        Paint(&printer);
    }

/** Loads a map in addition to the current map as an overlay or side-by-side tile. */
void MapForm::LoadMap(const QString& aFileName)
    {
    CartoType::CString filename = aFileName.utf16();
    CartoType::TResult error = m_framework->LoadMap(filename);
    bool used_built_in_profiles = m_framework->BuiltInProfileCount() > 0;
    if (error)
        m_main_window.ShowError("failed to load map in current map",error);
    else
        {
        m_filename_array.push_back(aFileName);
        if (used_built_in_profiles || m_framework->BuiltInProfileCount() > 0)
            SetRouteProfileIndex(0);
        m_framework->SetViewLimits(1000,0);
        m_map_renderer = std::make_unique<CartoType::CQtMapRenderer>(*m_framework);
        m_map_renderer->Init();
        }
    }
