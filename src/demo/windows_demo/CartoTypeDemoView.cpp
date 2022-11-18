
// CartoTypeDemoView.cpp : implementation of the CCartoTypeDemoView class
//

#include "stdafx.h"

// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "CartoTypeDemo.h"
#endif

#include "CartoTypeDemoDoc.h"
#include "CartoTypeDemoView.h"
#include "FindDialog.h"
#include "LatLongDialog.h"
#include "LayerDialog.h"
#include "SetScaleDialog.h"
#include "RouteDialog.h"
#include "MainFrm.h"
#include <fstream>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCartoTypeDemoView

IMPLEMENT_DYNCREATE(CCartoTypeDemoView, CView)

BEGIN_MESSAGE_MAP(CCartoTypeDemoView, CView)
    ON_WM_CONTEXTMENU()
    ON_WM_RBUTTONUP()
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_WM_MOUSEWHEEL()
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSELEAVE()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_DESTROY()
    ON_WM_TIMER()
    ON_COMMAND(ID_FILE_OPEN_IN_CURRENT_MAP,&CCartoTypeDemoView::OnFileOpenInCurrentMap)
    ON_COMMAND(ID_FILE_CLOSE,&CCartoTypeDemoView::OnFileClose)
    ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE,&CCartoTypeDemoView::OnUpdateFileClose)
    ON_COMMAND(ID_FILE_SAVE_AS_PNG,&CCartoTypeDemoView::OnFileSaveAsPng)
    ON_COMMAND(ID_FILE_SAVE_AS_CTSQL,&CCartoTypeDemoView::OnFileSaveAsCtsql)
    ON_COMMAND(ID_FIND,&CCartoTypeDemoView::OnFind)
    ON_UPDATE_COMMAND_UI(ID_FIND,&CCartoTypeDemoView::OnUpdateFind)
    ON_COMMAND(ID_FIND_ADDRESS,&CCartoTypeDemoView::OnFindAddress)
    ON_UPDATE_COMMAND_UI(ID_FIND_ADDRESS,&CCartoTypeDemoView::OnUpdateFindAddress)
    ON_COMMAND(ID_FIND_NEXT,&CCartoTypeDemoView::OnFindNext)
    ON_UPDATE_COMMAND_UI(ID_FIND_NEXT,&CCartoTypeDemoView::OnUpdateFindNext)
    ON_COMMAND(ID_VIEW_RELOAD_STYLE_SHEET,&CCartoTypeDemoView::OnViewReloadStyleSheet)
    ON_UPDATE_COMMAND_UI(ID_VIEW_RELOAD_STYLE_SHEET,&CCartoTypeDemoView::OnUpdateViewReloadStyleSheet)
    ON_COMMAND(ID_VIEW_ZOOM_IN,&CCartoTypeDemoView::OnViewZoomIn)
    ON_COMMAND(ID_VIEW_ZOOM_OUT,&CCartoTypeDemoView::OnViewZoomOut)
    ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM_IN,&CCartoTypeDemoView::OnUpdateViewZoomIn)
    ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM_OUT,&CCartoTypeDemoView::OnUpdateViewZoomOut)
    ON_COMMAND(ID_VIEW_ROTATE,&CCartoTypeDemoView::OnViewRotate)
    ON_UPDATE_COMMAND_UI(ID_VIEW_ROTATE,&CCartoTypeDemoView::OnUpdateViewRotate)
    ON_COMMAND(ID_NORTH_UP,&CCartoTypeDemoView::OnViewNorthup)
    ON_UPDATE_COMMAND_UI(ID_NORTH_UP,&CCartoTypeDemoView::OnUpdateViewNorthup)
    ON_COMMAND(ID_VIEW_LATLONG,&CCartoTypeDemoView::OnViewLatlong)
    ON_UPDATE_COMMAND_UI(ID_VIEW_LATLONG,&CCartoTypeDemoView::OnUpdateViewLatlong)
    ON_COMMAND(ID_VIEW_STYLESHEET,&CCartoTypeDemoView::OnViewStylesheet)
    ON_UPDATE_COMMAND_UI(ID_VIEW_STYLESHEET,&CCartoTypeDemoView::OnUpdateViewStylesheet)
    ON_COMMAND(ID_VIEW_LAYERS,&CCartoTypeDemoView::OnViewLayers)
    ON_UPDATE_COMMAND_UI(ID_VIEW_LAYERS,&CCartoTypeDemoView::OnUpdateViewLayers)
    ON_COMMAND(ID_VIEW_LEGEND,&CCartoTypeDemoView::OnViewLegend)
    ON_UPDATE_COMMAND_UI(ID_VIEW_LEGEND,&CCartoTypeDemoView::OnUpdateViewLegend)
    ON_COMMAND(ID_VIEW_SET_SCALE,&CCartoTypeDemoView::OnViewSetScale)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SET_SCALE,&CCartoTypeDemoView::OnUpdateViewSetScale)
    ON_COMMAND(ID_VIEW_PERSPECTIVE,&CCartoTypeDemoView::OnViewPerspective)
    ON_UPDATE_COMMAND_UI(ID_VIEW_PERSPECTIVE,&CCartoTypeDemoView::OnUpdateViewPerspective)
    ON_COMMAND(ID_VIEW_METRIC_UNITS,&CCartoTypeDemoView::OnViewMetricUnits)
    ON_UPDATE_COMMAND_UI(ID_VIEW_METRIC_UNITS,&CCartoTypeDemoView::OnUpdateViewMetricUnits)
    ON_COMMAND(ID_VIEW_DRIVING_INSTRUCTIONS,&CCartoTypeDemoView::OnViewTurnInstructions)
    ON_UPDATE_COMMAND_UI(ID_VIEW_DRIVING_INSTRUCTIONS,&CCartoTypeDemoView::OnUpdateViewTurnInstructions)
    ON_COMMAND(ID_VIEW_FLY_THROUGH,&CCartoTypeDemoView::OnViewFlyThrough)
    ON_UPDATE_COMMAND_UI(ID_VIEW_FLY_THROUGH,&CCartoTypeDemoView::OnUpdateViewFlyThrough)
    ON_COMMAND(ID_GO_ALONG_ROUTE,&CCartoTypeDemoView::OnGoAlongRoute)
    ON_UPDATE_COMMAND_UI(ID_GO_ALONG_ROUTE,&CCartoTypeDemoView::OnUpdateGoAlongRoute)
    ON_COMMAND(ID_ROUTE_REVERSE_ROUTE,&CCartoTypeDemoView::OnReverseRoute)
    ON_UPDATE_COMMAND_UI(ID_ROUTE_REVERSE_ROUTE,&CCartoTypeDemoView::OnUpdateReverseRoute)
    ON_COMMAND(ID_ROUTE_DELETE_ROUTE,&CCartoTypeDemoView::OnDeleteRoute)
    ON_UPDATE_COMMAND_UI(ID_ROUTE_DELETE_ROUTE,&CCartoTypeDemoView::OnUpdateDeleteRoute)
    ON_COMMAND(ID_ROUTE_DRIVE,&CCartoTypeDemoView::OnSetDriveProfile)
    ON_UPDATE_COMMAND_UI(ID_ROUTE_DRIVE,&CCartoTypeDemoView::OnUpdateSetDriveProfile)
    ON_COMMAND(ID_ROUTE_CYCLE,&CCartoTypeDemoView::OnSetCycleProfile)
    ON_UPDATE_COMMAND_UI(ID_ROUTE_CYCLE,&CCartoTypeDemoView::OnUpdateSetCycleProfile)
    ON_COMMAND(ID_ROUTE_WALK,&CCartoTypeDemoView::OnSetWalkProfile)
    ON_UPDATE_COMMAND_UI(ID_ROUTE_WALK,&CCartoTypeDemoView::OnUpdateSetWalkProfile)
    ON_COMMAND(ID_ROUTE_HIKE,&CCartoTypeDemoView::OnSetHikeProfile)
    ON_UPDATE_COMMAND_UI(ID_ROUTE_HIKE,&CCartoTypeDemoView::OnUpdateSetHikeProfile)
    ON_COMMAND(ID_ROUTE_SHORTEST,&CCartoTypeDemoView::OnShortestRoute)
    ON_UPDATE_COMMAND_UI(ID_ROUTE_SHORTEST,&CCartoTypeDemoView::OnUpdateShortestRoute)
    ON_COMMAND(ID_ROUTE_USEGRADIENTS,&CCartoTypeDemoView::OnUseGradients)
    ON_UPDATE_COMMAND_UI(ID_ROUTE_USEGRADIENTS,&CCartoTypeDemoView::OnUpdateUseGradients)
    ON_COMMAND(ID_ROUTE_SAVE_INSTRUCTIONS,&CCartoTypeDemoView::OnSaveRouteInstructions)
    ON_UPDATE_COMMAND_UI(ID_ROUTE_SAVE_INSTRUCTIONS,&CCartoTypeDemoView::OnUpdateSaveRouteInstructions)
    ON_COMMAND(ID_ROUTE_SAVE_ROUTE_XML,&CCartoTypeDemoView::OnSaveRouteXml)
    ON_UPDATE_COMMAND_UI(ID_ROUTE_SAVE_ROUTE_XML,&CCartoTypeDemoView::OnUpdateSaveRouteXml)
    ON_COMMAND(ID_ROUTE_SAVE_ROUTE_GPX,&CCartoTypeDemoView::OnSaveRouteGpx)
    ON_UPDATE_COMMAND_UI(ID_ROUTE_SAVE_ROUTE_GPX,&CCartoTypeDemoView::OnUpdateSaveRouteGpx)
    ON_COMMAND(ID_ROUTE_LOAD_ROUTE_XML,&CCartoTypeDemoView::OnLoadRouteXml)
    ON_UPDATE_COMMAND_UI(ID_ROUTE_LOAD_ROUTE_XML,&CCartoTypeDemoView::OnUpdateLoadRouteXml)
    ON_COMMAND(ID_EDIT_LOAD_KML,&CCartoTypeDemoView::OnEditLoadKml)
    ON_UPDATE_COMMAND_UI(ID_EDIT_LOAD_KML,&CCartoTypeDemoView::OnUpdateEditLoadKml)
    ON_COMMAND(ID_EDIT_SAVE_KML,&CCartoTypeDemoView::OnEditSaveKml)
    ON_UPDATE_COMMAND_UI(ID_EDIT_SAVE_KML,&CCartoTypeDemoView::OnUpdateEditSaveKml)
    ON_COMMAND(ID_EDIT_LOAD_CTMS,&CCartoTypeDemoView::OnEditLoadCtms)
    ON_UPDATE_COMMAND_UI(ID_EDIT_LOAD_CTMS,&CCartoTypeDemoView::OnUpdateEditLoadCtms)
    ON_COMMAND(ID_EDIT_SAVE_CTMS,&CCartoTypeDemoView::OnEditSaveCtms)
    ON_UPDATE_COMMAND_UI(ID_EDIT_SAVE_CTMS,&CCartoTypeDemoView::OnUpdateEditSaveCtms)
    ON_COMMAND(ID_EDIT_LOAD_CTSQL,&CCartoTypeDemoView::OnEditLoadCtsql)
    ON_UPDATE_COMMAND_UI(ID_EDIT_LOAD_CTSQL,&CCartoTypeDemoView::OnUpdateEditLoadCtsql)
    ON_COMMAND(ID_EDIT_SAVE_CTSQL,&CCartoTypeDemoView::OnEditSaveCtsql)
    ON_UPDATE_COMMAND_UI(ID_EDIT_SAVE_CTSQL,&CCartoTypeDemoView::OnUpdateEditSaveCtsql)
    ON_COMMAND(ID_EDIT_LOAD_GPX,&CCartoTypeDemoView::OnEditLoadGpx)
    ON_UPDATE_COMMAND_UI(ID_EDIT_LOAD_GPX,&CCartoTypeDemoView::OnUpdateEditLoadGpx)
END_MESSAGE_MAP()

// CCartoTypeDemoView construction/destruction

CCartoTypeDemoView::CCartoTypeDemoView()
    {
    iRoutePointArray.resize(2);
    }

CCartoTypeDemoView::~CCartoTypeDemoView()
    {
    iMapRenderer = nullptr;
    iRouteIter = nullptr;
    iFramework = nullptr;
    iExtraGc = nullptr;
    }

BOOL CCartoTypeDemoView::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    return CView::PreCreateWindow(cs);
}

const CartoType::TBitmap* CCartoTypeDemoView::MapBitmap(CartoType::TResult& aError,const CartoType::TRect& aMapClientArea,bool& aRedrawNeeded)
    {
    // Get the map bitmap.
    const CartoType::TBitmap* bitmap = iFramework->MapBitmap(aError,&aRedrawNeeded);

    /*
    If there are driving instructions or a legend, or there's a drag offset, copy the map bitmap into the extra GC,
    offset if necessary, and draw them on top.
    */
    if (!aError && (iDrawTurnInstructions || iDrawLegend || iMapDragOffset.iX || iMapDragOffset.iY))
        {
        // Create the extra GC if necessary.
        if (iExtraGc == nullptr || iExtraGc->Bounds() != CartoType::TRect(0,0,bitmap->Width(),bitmap->Height()))
            iExtraGc = iFramework->CreateGraphicsContext(bitmap->Width(),bitmap->Height());

        if (!aError)
            {
            ASSERT(bitmap->DataBytes() == iExtraGc->Bitmap()->DataBytes());
            ASSERT(bitmap->RowBytes() == iExtraGc->Bitmap()->RowBytes());

            // Copy the bitmap into the extra GC's bitmap. */
            if (iMapDragOffset.iX || iMapDragOffset.iY)
                {
                // Set the destination bitmap to white so that the edges don't have relics of the previous map.
                memset(const_cast<uint8_t*>(iExtraGc->Bitmap()->Data()),0xFF,iExtraGc->Bitmap()->DataBytes());

                // Draw the source bitmap at the required offset.
                iExtraGc->DrawBitmap(*bitmap,iMapDragOffset);
                }
            else
                memcpy(const_cast<uint8_t*>(iExtraGc->Bitmap()->Data()),bitmap->Data(),bitmap->DataBytes());

            // Draw the legend and copyright notice.
            if (iDrawLegend)
                iFramework->DrawNotices(*iExtraGc);

            bitmap = iExtraGc->Bitmap();
            }
        }

    return bitmap;
    }

void CCartoTypeDemoView::CreateLegend()
    {
    auto legend = std::make_unique<CartoType::CLegend>(*iFramework);

    CartoType::TColor grey(90,90,90);
    legend->SetTextColor(grey);
    legend->SetAlignment(CartoType::TAlign::Center);
    CartoType::CString dataset_name(iFramework->DataSetName());
    iFramework->SetCase(dataset_name,CartoType::TLetterCase::Lower);
    iFramework->SetCase(dataset_name,CartoType::TLetterCase::Title);
    legend->SetFontSize(10,"pt");
    legend->AddTextLine(dataset_name);

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
    legend->AddMapObjectLine(CartoType::TMapObjectType::Line,"path","cyc",0,"","cycleway");
    legend->AddMapObjectLine(CartoType::TMapObjectType::Line,"path","bri",0,"","bridle path");
    legend->AddMapObjectLine(CartoType::TMapObjectType::Line,"path","foo",0,"","footpath");
    legend->AddMapObjectLine(CartoType::TMapObjectType::Polygon,"land/major","for",0,"Ashridge","forest or wood");
    legend->AddMapObjectLine(CartoType::TMapObjectType::Polygon,"land/minor","par",0,"Green Park","park, golf course or common");
    legend->AddMapObjectLine(CartoType::TMapObjectType::Polygon,"land/minor","gra",0,"","grassland");
    legend->AddMapObjectLine(CartoType::TMapObjectType::Polygon,"land/minor","orc",0,"","orchard, vineyard, etc.");
    legend->AddMapObjectLine(CartoType::TMapObjectType::Polygon,"land/minor","cmr",0,"","commercial or industrial");
    legend->AddMapObjectLine(CartoType::TMapObjectType::Polygon,"land/minor","cns",0,"","construction, quarry, landfill, etc.");
    legend->AddMapObjectLine(CartoType::TMapObjectType::Point,"amenity/minor","stn",0,"Berkhamsted","station");

    // Uncomment these lines to create a scale bar as part of the legend.
    // legend->SetAlignment(CartoType::TAlign::Center);
    // legend->AddScaleLine(iMetricUnits);

    legend->SetBorder(CartoType::KGray,1,4,"pt");
    CartoType::TColor b(CartoType::KWhite);
    b.SetAlpha(224);
    legend->SetBackgroundColor(b);

    iFramework->SetLegend(std::move(legend),1,"in",CartoType::TNoticePosition::TopRight);
    }

static void DrawBorder(CartoType::CGraphicsContext& aGc,const CartoType::TRect& aBounds,int aShrinkage,int aStrokeWidth,int aRadius,CartoType::TColor aBorderColor,CartoType::TColor aClearColor)
    {
    CartoType::TRect bounds(aBounds);
    bounds.iTopLeft.iX += aShrinkage;
    bounds.iTopLeft.iY += aShrinkage;
    bounds.iBottomRight.iX -= aShrinkage;
    bounds.iBottomRight.iY -= aShrinkage;

    CartoType::CContour contour;
    double half_stroke_width = aStrokeWidth * 32.0;
    double x1 = bounds.Left() * 64.0 + half_stroke_width;
    double x2 = bounds.Right() * 64.0 - half_stroke_width;
    double y1 = (bounds.Top() * 64.0 + bounds.Bottom() * 64.0) / 2.0;
    double y2 = y1;
    double width = (bounds.Height() - aStrokeWidth) * 64.0;
    contour.MakeRoundedRectangle(x1,y1,x2,y2,width,aRadius * 64);

    CartoType::TCircularPen pen;
    pen.iPenWidth = aStrokeWidth;
    aGc.SetPen(pen);
    aGc.SetColor(aClearColor);
    aGc.DrawShapeAndStroke(contour,aBorderColor);
    }

static void DrawBitmapWithAlpha(HDC aDeviceContext,const CartoType::TBitmap* aBitmap,const CartoType::TRect& aDestRect,const CartoType::TRect& aSourceRect)
    {
    HDC hdc = CreateCompatibleDC(aDeviceContext);
    BITMAPINFOHEADER bitmap_header;
    memset(&bitmap_header,0,sizeof(bitmap_header));
    bitmap_header.biSize = sizeof(BITMAPINFOHEADER);
    bitmap_header.biWidth = aBitmap->Width();
    bitmap_header.biHeight = aBitmap->Height();
    bitmap_header.biPlanes = 1;
    bitmap_header.biBitCount = 32;
    bitmap_header.biCompression = BI_RGB;
    bitmap_header.biSizeImage = 0;
    bitmap_header.biXPelsPerMeter = 1000;
    bitmap_header.biYPelsPerMeter = 1000;
    bitmap_header.biClrUsed = 0;
    bitmap_header.biClrImportant = 0;

    void* bitmap_data = nullptr;
    HBITMAP hbitmap = CreateDIBSection(hdc,(const BITMAPINFO*)&bitmap_header,DIB_RGB_COLORS,&bitmap_data,nullptr,0);

    // Reflect the data vertically and change from ABGR to BGRA.
    unsigned char* dest_row = (unsigned char*)bitmap_data;
    const unsigned char* source_row = aBitmap->Data() + (aBitmap->Height() - 1) * aBitmap->RowBytes();
    for (int y = 0; y < aBitmap->Height(); y++,dest_row += aBitmap->RowBytes(),source_row -= aBitmap->RowBytes())
        {
        const unsigned char* p = source_row;
        unsigned char* q = dest_row;
        unsigned char* end = q + aBitmap->RowBytes();
        while (q < end)
            {
            *q++ = p[1];
            *q++ = p[2];
            *q++ = p[3];
            *q++ = p[0];
            p += 4;
            }
        }
    SelectObject(hdc,hbitmap);

    BLENDFUNCTION bf;
    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = 0xFF;
    bf.AlphaFormat = AC_SRC_ALPHA;

    AlphaBlend(aDeviceContext,aDestRect.Left(),aDestRect.Top(),aDestRect.Width(),aDestRect.Height(),
               hdc,aSourceRect.Left(),aSourceRect.Top(),aSourceRect.Width(),aSourceRect.Height(),
               bf);

    DeleteObject(hbitmap);
    DeleteDC(hdc);
    }

void CCartoTypeDemoView::OnDraw(CDC* pDC)
    {
    if (iMapRenderer)
        {
        iMapRenderer->Draw();
        return;
        }

    RECT clip;
    pDC->GetClipBox(&clip);
    CartoType::TRect dest_clip(clip.left,clip.top,clip.right,clip.bottom);

    RECT mc;
    GetClientRect(&mc);
    CartoType::TRect map_client_area(mc.left,mc.top,mc.right,mc.bottom);

    CartoType::TRect source_rect = map_client_area;
    CartoType::TRect dest_rect = map_client_area;

    // Return now if there is nothing to be drawn.
    dest_clip.Intersection(dest_rect);
    if (dest_clip.IsEmpty())
        return;
    CartoType::TRect source_clip(dest_clip);
    source_clip.Intersection(source_rect);
    if (source_clip.IsEmpty())
        return;

    // Get the map bitmap. If the image server is in use this will be the in-memory db only.
    CartoType::TResult error = 0;
    bool redraw_needed = false;
    const CartoType::TBitmap* map_bitmap = MapBitmap(error,map_client_area,redraw_needed);

    if (error)
        {
        CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();

        char buffer[256];
        auto error_string = CartoType::ErrorString(error);
        sprintf_s(buffer,"failed to draw map: %s",error_string.c_str());
        ::CString str(buffer);
        CMainFrame* main_window = (CMainFrame*)app->m_pMainWnd;
        main_window->SetMessageText(str);
        return;
        }

    // Blit the drawing surface to the window.
    BitmapInfoX bm;
    bm.bmiHeader.bV4Size = sizeof(BITMAPV4HEADER);
    bm.bmiHeader.bV4Width = map_bitmap->Width();
    bm.bmiHeader.bV4Height = -map_bitmap->Height();
    bm.bmiHeader.bV4Planes = 1;
    bm.bmiHeader.bV4BitCount = 32;
    bm.bmiHeader.bV4V4Compression = BI_BITFIELDS;
    bm.bmiHeader.bV4SizeImage = 0;
    bm.bmiHeader.bV4XPelsPerMeter = 1000;
    bm.bmiHeader.bV4YPelsPerMeter = 1000;
    bm.bmiHeader.bV4ClrUsed = 0;
    bm.bmiHeader.bV4ClrImportant = 0;
    bm.bmiHeader.bV4RedMask = 0xFF000000;
    bm.bmiHeader.bV4GreenMask = 0xFF0000;
    bm.bmiHeader.bV4BlueMask = 0xFF00;
    bm.bmiHeader.bV4AlphaMask = 0xFF;
    const unsigned char* map_data = (const unsigned char*)(map_bitmap->Data());

    /*
    Blit the whole GC if the clip rectangle is at the bottom left of the GC.
    This gets round a bug in StretchDIBits (when dealing with 32bpp bitmaps) that causes a y coordinate of 0 to
    be treated as the top, not the bottom, of the source bitmap.
    */
    if (source_clip.Top() > 0 && source_clip.Left() == 0 && source_clip.Bottom() == map_bitmap->Height())
        ::StretchDIBits(*pDC,
        0,0,map_bitmap->Width(),map_bitmap->Height(),
        0,0,map_bitmap->Width(),map_bitmap->Height(),
        map_data,(BITMAPINFO*)&bm,DIB_RGB_COLORS,SRCCOPY);
    else
        ::StretchDIBits(*pDC,dest_clip.Left(),dest_clip.Top(),dest_clip.Width(),dest_clip.Height(),
        source_clip.Left(),map_bitmap->Height() - source_clip.Bottom(),source_clip.Width(),source_clip.Height(),
        map_data,(BITMAPINFO*)&bm,DIB_RGB_COLORS,SRCCOPY);
    }

void CCartoTypeDemoView::CalculateAndDisplayRoute()
    {
    CartoType::TRouteCoordSet cs(CartoType::TCoordType::Map);

    for (const auto& p : iRoutePointArray)
        {
        CartoType::TRoutePoint rp;
        rp = p.iPoint;
        cs.iRoutePointArray.push_back(rp);
        }

    CartoType::TResult error = 0;
    CartoType::TRouteProfile profile = *iFramework->Profile(0);
    if (iBestRoute && cs.iRoutePointArray.size() > 3)
        {
        size_t iterations = cs.iRoutePointArray.size() * cs.iRoutePointArray.size();
        if (iterations < 16)
            iterations = 16;
        else if (iterations > 256)
            iterations = 256;
        auto r = iFramework->CreateBestRoute(error,profile,cs,true,true,iterations);
        if (!error)
            error = iFramework->UseRoute(*r,true);
        }
    else
        {
        auto r = iFramework->CreateRoute(error,profile,cs);
        if (!error)
            error = iFramework->UseRoute(*r,true);
        
        if (!error)
            {
            CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
            CMainFrame* main_window = (CMainFrame*)app->m_pMainWnd;
            char buffer[256];
            auto data = iFramework->RouteCreationData();
            sprintf_s(buffer,"route creation = %gs, expansion = %gs, node queries = %d, node misses = %d, arc queries = %d, arc misses = %d",
                      data.iRouteCalculationTime,data.iRouteExpansionTime,
                      int(data.iNodeCacheQueries),int(data.iNodeCacheMisses),
                      int(data.iArcCacheQueries),int(data.iArcCacheMisses));
            CartoType::CString s = buffer;
            s.Append((uint16_t)0);
            main_window->SetMessageText((LPCTSTR)s.Text());
            }
        }

    if (error)
        {
        CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
        app->ShowError(_TEXT("Routing error"),error);
        }
    else
        {
        if (iFramework->Route())
            iRouteIter.reset(new CartoType::TRouteIterator(*iFramework->Route()));

        bool at_end = false;
        SimulateNavigationFix(false,at_end);
        }
    }

void CCartoTypeDemoView::OnRButtonUp(UINT nFlags, CPoint point)
    {
    CView::OnRButtonUp(nFlags,point);
    CartoType::TPointFP p(point.x,point.y);

    // Get the address of the point clicked on.
    CartoType::CAddress address;
    iFramework->GetAddress(address,point.x,point.y,CartoType::TCoordType::Display);
    CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
    CMainFrame* main_window = (CMainFrame*)app->m_pMainWnd;

    CartoType::CString address_string;
    iFramework->ConvertPoint(p.iX,p.iY,CartoType::TCoordType::Display,CartoType::TCoordType::Degree);
    address_string = address.ToString(false,&p);

    address_string.Append((uint16_t)0);
    main_window->SetMessageText((LPCTSTR)address_string.Text());

    if (address_string.Length())
        address_string.Delete(address_string.Length() - 1,address_string.Length());
    address_string.Append("\r\n\r\n[");
    address_string += address.ToStringWithLabels();
    address_string.Append("]");

    // Add the altitude if available.
    CartoType::TCoordSet cs(&p,1);
    CartoType::TResult error;
    auto alt = iFramework->GetHeights(error,cs,CartoType::TCoordType::Degree);
    if (!error)
        {
        char buffer[32];
        sprintf_s(buffer," (altitude = %dm)",alt[0]);
        address_string.Append(buffer);
        }

    // If possible, get the Ordnance Survey National Grid reference.
    CartoType::CString os_grid_ref = CartoType::UKGridReferenceFromDegrees(p,3);
    if (os_grid_ref.Length())
        {
        address_string.Append("\r\n\r\nOrdnance Survey of Great Britain grid reference: ");
        address_string.Append(os_grid_ref);
        }

    // Get information about the objects clicked on - within 2mm.
    CartoType::CMapObjectArray object;
    double pixel_mm = iFramework->ResolutionDpi() / 25.4;
    iFramework->FindInDisplay(object,100,point.x,point.y,int(ceil(2 * pixel_mm)));
    CartoType::CMapObject* pushpin = nullptr;

    // See if we have a pushpin, and geocode the objects so that they can be sorted.
    for (auto& o : object)
        {
        if (o->LayerName() == "pushpin" && o->Type() == CartoType::TMapObjectType::Point)
            pushpin = o.get();
        o->SetUserData((int)o->GetGeoCodeType());
        }

    // Sort objects on geocode and add information about them to the 'Address and nearby objects' field.
    if (object.size())
        {
        bool first_object = true;
        std::sort(object.begin(),object.end(),CartoType::MapObjectUserDataLessThan);
        int max_size = int(10000.0 / iFramework->MapUnitSize());
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

    // Display the route dialog.
    CRouteDialog route_dialog(pushpin != nullptr);
    if (pushpin)
        route_dialog.iButtonIndex = 2;
    else if (iRoutePointArray[0].iPoint.iPoint == CartoType::TPointFP())
        route_dialog.iButtonIndex = 0;
    else
        route_dialog.iButtonIndex = 1;
    route_dialog.iBestRoute = iBestRoute;

    SetString(route_dialog.iAddress,address_string);

    // Convert the point clicked on to map coords.
    CartoType::TRoutePoint rp;
    rp.iPoint.iX = point.x;
    rp.iPoint.iY = point.y;
    iFramework->ConvertPoint(rp.iPoint.iX,rp.iPoint.iY,CartoType::TCoordType::Display,CartoType::TCoordType::Map);

    // Set the heading to that of the nearest road.
    CartoType::TNearestRoadInfo info;
    error = iFramework->FindNearestRoad(info,rp.iPoint.iX,rp.iPoint.iY,CartoType::TCoordType::Map,-1,false);
    static const char* compass_points[16] =
        { "n", "nne", "ne", "ene", "e", "ese", "se", "sse", "s", "ssw", "sw", "wsw", "w", "wnw", "nw", "nnw" };
    double d = 0;
    info.iHeadingInDegrees += 11.25;
    if (info.iHeadingInDegrees < 0)
        info.iHeadingInDegrees += 360;
    else if (info.iHeadingInDegrees > 360)
        info.iHeadingInDegrees -= 360;
    if (!error)
        for (const auto& p : compass_points)
            {
            if (info.iHeadingInDegrees >= d && info.iHeadingInDegrees < d + 22.5)
                {
                SetString(route_dialog.iHeading,p);
                break;
                }
            d += 22.5;
            }

    uint64_t pushpin_id = 0;
    if (pushpin)
        {
        CartoType::TText value = pushpin->GetStringAttribute(CartoType::TText());
        iPushPinName.Set(value);
        value = pushpin->GetStringAttribute("desc");
        iPushPinDesc.Set(value);
        pushpin_id = pushpin->Id();
        }
    else
        {
        iPushPinName.Set(address.ToString(false));
        iPushPinDesc.Set(address.ToString(true));
        }
    SetString(route_dialog.iPushPinName,iPushPinName);
    SetString(route_dialog.iPushPinDesc,iPushPinDesc);

    if (route_dialog.DoModal() != IDOK)
        return;

    // Stop any route fly-through.
    StopFlyThrough();

    // Get the heading if any.
    CartoType::CString h;
    SetString(h,route_dialog.iHeading);
    h.SetCase(CartoType::TLetterCase::Lower);
    d = 0;
    for (const auto& p : compass_points)
        {
        if (h == p)
            {
            rp.iHeading = d;
            rp.iHeadingKnown = true;
            break;
            }
        d += 22.5;
        }
    if (!rp.iHeadingKnown && h.Length())
        {
        size_t length = 0;
        d = h.ToDouble(&length);
        if (length == h.Length())
            {
            rp.iHeading = d;
            rp.iHeadingKnown = true;
            }
        }

    bool create_route = false;
    iBestRoute = route_dialog.iBestRoute != 0;
    if (route_dialog.iButtonIndex == 0) // set start of route
        {
        iRoutePointArray[0].iPoint = rp;
        create_route = true;
        }
    else if (route_dialog.iButtonIndex == 1) // set end of route
        {
        iRoutePointArray[iRoutePointArray.size() - 1].iPoint = rp;
        create_route = true;
        }
    else if (route_dialog.iButtonIndex == 2) // add or change a pushpin
        {
        SetString(iPushPinName,route_dialog.iPushPinName);
        SetString(iPushPinDesc,route_dialog.iPushPinDesc);
        CartoType::CString string_attrib;
        string_attrib.Set(iPushPinName);
        if (iPushPinDesc.Length())
            {
            string_attrib.Append((uint16_t)0);
            string_attrib.Append("desc=");
            string_attrib.Append(iPushPinDesc);
            }
        double x = 0,y = 0;
        if (pushpin)
            {
            CartoType::TRect r { pushpin->CBox() };
            x = r.iTopLeft.iX;
            y = r.iTopLeft.iY;
            }
        else
            {
            x = rp.iPoint.iX;
            y = rp.iPoint.iY;
            }
        CartoType::TTextLiteral(layer,u"pushpin");
        iFramework->InsertPointMapObject(iWritableMapHandle,layer,x,y,CartoType::TCoordType::Map,string_attrib,0,pushpin_id,true);

        if (route_dialog.iUseAsWayPoint)
            {
            TRoutePoint* waypoint = nullptr;

            for (size_t i = 1; waypoint == nullptr && i < iRoutePointArray.size() - 1; i++)
                if (iRoutePointArray[i].iId == pushpin_id)
                    waypoint = &iRoutePointArray[i];
            if (waypoint == nullptr)
                {
                TRoutePoint rp;
                iRoutePointArray.insert(iRoutePointArray.begin() + iRoutePointArray.size() - 1,rp);
                waypoint = &iRoutePointArray[iRoutePointArray.size() - 2];
                }
            if (waypoint)
                {
                waypoint->iPoint = rp;
                waypoint->iPoint.iPoint.iX = x;
                waypoint->iPoint.iPoint.iY = y;
                waypoint->iId = pushpin_id;
                create_route = true;
                }
            }  

        iWritableMapChanged = true;
        }
    else if (route_dialog.iButtonIndex == 3) // cut a pushpin
        {
        uint64_t count = 0;
        iFramework->DeleteMapObjects(iWritableMapHandle,pushpin_id,pushpin_id,count);
        for (size_t i = 1; i < iRoutePointArray.size() - 1; i++)
            if (iRoutePointArray[i].iId == pushpin_id)
                {
                iRoutePointArray.erase(iRoutePointArray.begin() + i);
                create_route = true;
                break;
                }
        iWritableMapChanged = true;
        }

    if (iRoutePointArray[0].iPoint.iPoint == CartoType::TPointFP() ||
        iRoutePointArray[iRoutePointArray.size() - 1].iPoint.iPoint == CartoType::TPointFP())
        create_route = false;

    if (create_route)
        CalculateAndDisplayRoute();

    Update();
    }

void CCartoTypeDemoView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
    theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CCartoTypeDemoView diagnostics

#ifdef _DEBUG
void CCartoTypeDemoView::AssertValid() const
{
    CView::AssertValid();
}

void CCartoTypeDemoView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CCartoTypeDemoDoc* CCartoTypeDemoView::GetDocument() const // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCartoTypeDemoDoc)));
    return (CCartoTypeDemoDoc*)m_pDocument;
}
#endif //_DEBUG

static void AppendDistance(CartoType::CString& aDistance,double aMetres,bool aMetricUnits)
    {
    char text[64];

    if (aMetricUnits)
        {
        if (aMetres < 50)
            sprintf_s(text,"%d meters",CartoType::Round(aMetres));
        else if (aMetres < 250)
            sprintf_s(text,"%d meters",CartoType::Round(aMetres / 10) * 10);
        else if (aMetres < 100000)
            sprintf_s(text,"%.1fkm",aMetres / 1000);
        else
            sprintf_s(text,"%.fkm",aMetres / 1000);
        }
    else
        {
        double yards = aMetres * 1.093613298;
        double miles = yards / 1760;
        if (yards < 55)
            sprintf_s(text,"%d yards",CartoType::Round(yards));
        else if (yards < 220)
            sprintf_s(text,"%d yards",CartoType::Round(yards / 10) * 10);
        else if (miles < 100)
            sprintf_s(text,"%.1f miles",miles);
        else
            sprintf_s(text,"%.f miles",miles);
        }

    aDistance.Append(text);
    }

static void AppendTime(CartoType::CString& aTime,double aSeconds)
    {
    char text[64];

    int hours = (int)aSeconds;
    int seconds = hours % 60;
    hours /= 60;
    int minutes = hours % 60;
    hours /= 60;

    int n = 0;
    if (hours > 1)
        n = sprintf_s(text,"%d hours",hours);
    else if (hours == 1)
        n = sprintf_s(text,"1 hour");
    if (hours > 0)
        n += sprintf_s(text + n,sizeof(text) - n," ");
    if (hours > 0 || minutes > 0)
        {
        if (minutes == 1)
            n += sprintf_s(text + n,sizeof(text) - n,"1 minute");
        else
            n += sprintf_s(text + n,sizeof(text) - n,"%d minutes",minutes);
        }
    if (hours == 0 && minutes == 0)
        {
        if (seconds == 1)
            sprintf_s(text,"1 second");
        else
            sprintf_s(text,"%d seconds",seconds);
        }
    aTime.Append(text);
    }

void CCartoTypeDemoView::PanToDraggedPosition()
    {
    auto to = iMapDragAnchor;
    to += iMapDragOffset;
    CartoType::TResult error = iFramework->Pan(iMapDragAnchor,to);

    if (error)
        ((CCartoTypeDemoApp*)AfxGetApp())->ShowError(_TEXT("Error panning to dragged position"),error);
    Update();
    }

void CCartoTypeDemoView::StopDragging()
    {
    iMapDragEnabled = false;
    PanToDraggedPosition();
    iMapDragOffset = CartoType::TPoint(0,0);
    TRACKMOUSEEVENT t;
    t.cbSize = sizeof(t);
    t.dwFlags = TME_LEAVE | TME_CANCEL;
    t.dwHoverTime = 0;
    t.hwndTrack = *this;
    TrackMouseEvent(&t);
    Update();
    }

void CCartoTypeDemoView::OnDestroy()
    {
    if (iWritableMapChanged &&
        AfxMessageBox(_TEXT("You have unsaved editable data (pushpins, etc). Save it now?"),MB_YESNO) == IDYES)
        OnEditSaveCtms();
    CView::OnDestroy();
    }

CartoType::TResult CCartoTypeDemoView::LoadStyleSheet(const ::CString& aPath)
    {
    CartoType::CString path;
    SetString(path,aPath);
    CartoType::TResult error = iFramework->SetStyleSheet(path);
    if (error)
        {
        CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
        app->ShowError("error loading stylesheet",error);
        }
    else
        Update();
    CreateLegend();
    return error;
    }

/** Simulate a navigation fix after moving along the route by aDriveDistance metres, or to the next turn if aDriveDistance is zero. */
void CCartoTypeDemoView::SimulateNavigationFix(bool aGoToNextTurn,bool& aAtEnd)
    {
    if (!iRouteIter)
        {
        aAtEnd = true;
        return;
        }

    CartoType::TPoint p = iRouteIter->Position();
    if (aGoToNextTurn)
        {
        // Move along the route to the next turn.
        double distance = iFramework->FirstTurn().iDistance;
        if (distance >= 10 || iFramework->NavigationState() == CartoType::TNavigationState::Arrival)
            p = iFramework->FirstTurn().iPosition;
        else if (iFramework->SecondTurn().iDistance > 0)
            {
            distance += iFramework->SecondTurn().iDistance;
            p = iFramework->SecondTurn().iPosition;
            }
        else
            {
            const CartoType::CRouteSegment* segment = iFramework->NextRouteSegment();
            if (segment)
                {
                auto end = segment->iPath.End();
                if (end.second)
                    p = end.first;
                }
            }
        iOnRouteTime += distance / 13.888;
        }

    // Simulate passing some GPS data to the navigator.
    CartoType::TNavigationData nav;
    nav.iValidity = CartoType::TNavigationData::KPositionValid | CartoType::TNavigationData::KTimeValid;
    nav.iPosition.iX = p.iX;
    nav.iPosition.iY = p.iY;
    nav.iTime = iOnRouteTime;
    iFramework->ConvertPoint(nav.iPosition.iX,nav.iPosition.iY,CartoType::TCoordType::Map,CartoType::TCoordType::Degree);
    iFramework->Navigate(nav);

    if (!aGoToNextTurn)
        {
        // Get distance in metres.
        double distance = iFlyThroughSpeedInKph / 3.6 * iTimerIntervalInSeconds;

        // Convert to map units.
        distance /= iFramework->MapUnitSize();

        // Move along the route.
        aAtEnd = !iRouteIter->Forward(distance);
        iOnRouteTime += iTimerIntervalInSeconds;

        // Adjust the speed.
        double new_speed = iFlyThroughSpeedInKph;
        auto turn = iFramework->FirstTurn();
        CartoType::TRoadType road_type = CartoType::TRoadType(uint32_t(turn.iFromRoadType) & CartoType::TRoadTypeFlag::RoadTypeMask);

        switch (road_type)
            {
            case CartoType::TRoadType::Motorway: new_speed = 110; break;
            case CartoType::TRoadType::Trunk: new_speed = 110; break;
            case CartoType::TRoadType::Primary: new_speed = 100; break;
            case CartoType::TRoadType::Secondary: new_speed = 80; break;
            case CartoType::TRoadType::Minor: new_speed = 50; break;
            case CartoType::TRoadType::Byway: new_speed = 20; break;
            case CartoType::TRoadType::AccessRamp: new_speed = 50; break;
            case CartoType::TRoadType::Service: new_speed = 50; break;
            case CartoType::TRoadType::VehicularFerry: new_speed = 20; break;
            case CartoType::TRoadType::PassengerFerry: new_speed = 20; break;
            case CartoType::TRoadType::Path: new_speed = 5; break;
            case CartoType::TRoadType::Stairway: new_speed = 1; break;
            case CartoType::TRoadType::CyclePath: new_speed = 15; break;
            case CartoType::TRoadType::FootPath: new_speed = 5; break;
            default: new_speed = 30; break;
            }

        if (road_type != CartoType::TRoadType::Motorway)
            {
            if (turn.iDistance < 30)
                new_speed = std::min(new_speed,20.0);
            else if (turn.iDistance < 15)
                new_speed = std::min(new_speed,15.0);
            else if (turn.iDistance < 5)
                new_speed = std::min(new_speed,8.0);
            }

        if (new_speed > iFlyThroughSpeedInKph)
            iFlyThroughSpeedInKph = std::min(new_speed,iFlyThroughSpeedInKph + 5 * iTimerIntervalInSeconds);
        else if (new_speed < iFlyThroughSpeedInKph)
            iFlyThroughSpeedInKph = std::max(new_speed,iFlyThroughSpeedInKph - 20 * iTimerIntervalInSeconds);

        char buffer[16];
        sprintf_s(buffer,"%.2f",iFlyThroughSpeedInKph);
        iFramework->SetCopyrightNotice(buffer);
        }
    }

// CCartoTypeDemoView message handlers

void CCartoTypeDemoView::OnInitialUpdate()
    {
    __super::OnInitialUpdate();

    EnableScrollBarCtrl(SB_BOTH,false);
    CartoType::TResult error = CartoType::KErrorNone;

    CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
    ASSERT_VALID(app);
    CCartoTypeDemoDoc* doc = GetDocument();
    ASSERT_VALID(doc);


    CartoType::CFramework::TParam param;
    param.iSharedEngine = app->Engine();
    param.iSharedMapDataSet = doc->MapDataSet();
    param.iStyleSheetFileName = app->DefaultStyleSheetName();
    RECT rect;
    GetClientRect(&rect);
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;
    param.iViewHeight = h;
    param.iViewWidth = w;
    param.iTextIndexLevels = 0;
    
    iFramework = CartoType::CFramework::New(error,param);
    if (!error)
        {
        error = iFramework->CreateWritableMap(CartoType::TWritableMapType::Memory);
        iWritableMapHandle = iFramework->GetLastMapHandle();
        }
    if (error)
        {
        app->ShowError(_TEXT("error creating CartoType framework"),error);
        return;
        }
    OutputMapExtent();

    // Start loading navigation data asynchronously.
    iFramework->LoadNavigationData();

    iFramework->SetFollowMode(CartoType::TFollowMode::None);
    iFramework->SetLocale(iMetricUnits ? "en_xx" : "en");

    CreateLegend();
    iFramework->EnableLegend(iDrawLegend);
    iFramework->EnableTurnInstructions(iDrawTurnInstructions);
    iFramework->SetScaleBar(iMetricUnits,2.5,"in",CartoType::TNoticePosition::BottomLeft);
    iFramework->SetTurnInstructions(iMetricUnits,true,2,"in",CartoType::TNoticePosition::TopLeft);
    iFramework->SetVehiclePosOffset(0,0.25);
    iFramework->SetAnimateTransitions(true);

    // COMMENT OUT THE NEXT LINE TO DISABLE GRAPHICS ACCELERATION
    iMapRenderer = std::make_unique<CartoType::CMapRenderer>(*iFramework,m_hWnd);
 
    // Add a call to iFramework->SetResolutionDpi to set the pixel size. The default resolution is 144dpi, which works well on Windows.

    TestCode();
    }

static std::unique_ptr<CartoType::CRoute> CreateSnappedRoute(CartoType::TResult& aError,CartoType::CFramework& aFramework,
                                                             const CartoType::TRouteProfile& aFindRoadProfile,const CartoType::TRouteProfile& aProfile,
                                                             CartoType::TPointFP aStart,CartoType::TPointFP aEnd)
    {
    CartoType::TLocationMatchParam param;
    param.iMaxRoadDistanceInMeters = 500;
    aFramework.SetLocationMatchParam(param);
    aFramework.SetMainProfile(aFindRoadProfile);
    CartoType::TNearestRoadInfo info;
    aError = aFramework.FindNearestRoad(info,aStart.iX,aStart.iY,CartoType::TCoordType::Degree,-1,false);
    if (aError)
        return nullptr;
    aStart = info.iNearestPoint;
    aError = aFramework.FindNearestRoad(info,aEnd.iX,aEnd.iY,CartoType::TCoordType::Degree,-1,false);
    if (aError)
        return nullptr;
    aEnd = info.iNearestPoint;
    CartoType::TCoordSetOfTwoPoints cs(aStart.iX,aStart.iY,aEnd.iX,aEnd.iY);
    param.iMaxRoadDistanceInMeters = 15;
    aFramework.SetLocationMatchParam(param);
    return aFramework.CreateRoute(aError,aProfile,cs,CartoType::TCoordType::Map);
    }

void CCartoTypeDemoView::TestCode()
    {
    }

void CCartoTypeDemoView::OnSize(UINT nType,int cx,int cy)
    {
    __super::OnSize(nType,cx,cy);
    if (iFramework)
        {
        iFramework->Resize(cx,cy);
        Update();
        }
    }

BOOL CCartoTypeDemoView::OnEraseBkgnd(CDC* /*pDC*/)
    {
    // Suppress erasure of the background.
    return TRUE;
    }

bool CCartoTypeDemoView::SetCentre(CPoint& aPoint)
    {
    GetCursorPos(&aPoint);
    ScreenToClient(&aPoint);
    CRect r;
    GetClientRect(&r);
    bool set_centre = false;
    if (aPoint.x >= r.left && aPoint.x < r.right && aPoint.y >= r.top && aPoint.y <= r.bottom)
        {
        aPoint.x -= r.left;
        aPoint.y -= r.top;
        set_centre = true;
        }
    return set_centre;
    }

BOOL CCartoTypeDemoView::OnMouseWheel(UINT nFlags,short zDelta,CPoint point)
    {
    if (zDelta == 0)
        return TRUE;

    CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();

    // If the mouse pointer is in the window, centre the zoom at that point.
    bool set_centre = SetCentre(point);

    /*
    If using the image server, always zoom by a factor of 2 because
    the image server system snaps to zoom levels arranged in factors
    of 2, and zooming less will often have no effect because it
    will snap back to the previous level.
    */
    double zoom = sqrt(app->ZoomFactor());

    int zoom_count = zDelta / WHEEL_DELTA;
    if (zoom_count == 0)
        zoom_count = zDelta >= 0 ? 1 : -1;

    zoom = pow(zoom,zoom_count);

    if (set_centre)
        iFramework->ZoomAt(zoom,point.x,point.y,CartoType::TCoordType::Display);
    else
        iFramework->Zoom(zoom);

    Update();
    return TRUE;
    }


void CCartoTypeDemoView::OnMouseMove(UINT /*nFlags*/,CPoint point)
    {
    if (iMapDragEnabled)
        {
        iMapDragOffset.iX = point.x - iMapDragAnchor.iX;
        iMapDragOffset.iY = point.y - iMapDragAnchor.iY;

        if (iMapRenderer)
            {
            iFramework->Pan(iMapDragAnchorInMapCoords.iX,iMapDragAnchorInMapCoords.iY,CartoType::TCoordType::Map,
                            point.x,point.y,CartoType::TCoordType::Display);
            iMapDragOffset = CartoType::TPoint(0,0);
            iMapDragAnchor.iX = point.x;
            iMapDragAnchor.iY = point.y;
            iMapDragAnchorInMapCoords = iMapDragAnchor;
            iFramework->ConvertPoint(iMapDragAnchorInMapCoords.iX,iMapDragAnchorInMapCoords.iY,CartoType::TCoordType::Display,CartoType::TCoordType::Map);
            }

        if (!iMapRenderer)
            {
            CDC* dc = GetDC();
            if (dc)
                {
                OnDraw(dc);
                ReleaseDC(dc);
                }
            }
        }
    }

void CCartoTypeDemoView::OnMouseLeave()
    {
    StopDragging();
    }

void CCartoTypeDemoView::OnLButtonDown(UINT nFlags,CPoint point)
    {
    iMapDragEnabled = true;
    iMapDragAnchor.iX = point.x;
    iMapDragAnchor.iY = point.y;
    iMapDragAnchorInMapCoords = iMapDragAnchor;
    iFramework->ConvertPoint(iMapDragAnchorInMapCoords.iX,iMapDragAnchorInMapCoords.iY,CartoType::TCoordType::Display,CartoType::TCoordType::Map);
    TRACKMOUSEEVENT t;
    t.cbSize = sizeof(t);
    t.dwFlags = TME_LEAVE;
    t.dwHoverTime = 0;
    t.hwndTrack = *this;
    TrackMouseEvent(&t);
    }

void CCartoTypeDemoView::OnLButtonUp(UINT nFlags,CPoint point)
    {
    iMapDragOffset.iX = point.x - iMapDragAnchor.iX;
    iMapDragOffset.iY = point.y - iMapDragAnchor.iY;

    // Supply a simulated navigation fix if the left mouse button is pressed with the shift key down.	
    if (iShiftLeftClickSimulatesNavigationFix)
        {
        if (iMapDragOffset.iX == 0 && iMapDragOffset.iY == 0 && (nFlags & MK_SHIFT))
            {
            CartoType::TNavigationData nav;
            nav.iValidity = CartoType::TNavigationData::KPositionValid | CartoType::TNavigationData::KTimeValid;
            nav.iPosition.iX = point.x;
            nav.iPosition.iY = point.y;
            iOnRouteTime += 20;
            nav.iTime = iOnRouteTime;
            iFramework->ConvertPoint(nav.iPosition.iX,nav.iPosition.iY,CartoType::TCoordType::Display,CartoType::TCoordType::Degree);
            iFramework->Navigate(nav);

            CartoType::CString state = "navigation state: ";
            switch (iFramework->NavigationState())
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

            CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
            CMainFrame* main_window = (CMainFrame*)app->m_pMainWnd;
            state.Append((uint16_t)0);
            main_window->SetMessageText((LPCTSTR)state.Text());
            }
        }

    StopDragging();
    }

void CCartoTypeDemoView::OnFileOpenInCurrentMap()
    {
    ::CString filename;
    CartoType::TResult error = CartoType::KErrorGeneral;
    if (AfxGetApp()->DoPromptFileName(filename,AFX_IDS_OPENFILE,OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,true,nullptr))
        {
        CartoType::CString f;
        SetString(f,filename);
        bool used_built_in_profiles = iFramework->BuiltInProfileCount() > 0;
        if (f.Last(10) == ".ctm1_ctci")
            {
            std::string key("password");
            error = iFramework->LoadMap(f,&key);
            }
        else
            error = iFramework->LoadMap(f);
        if (!error)
            {
            iMapHandle.push_back(iFramework->GetLastMapHandle());
            if (used_built_in_profiles || iFramework->BuiltInProfileCount() > 0)
                SetProfile(0);
            }
        }
    if (error)
        {
        CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
        app->ShowError(_TEXT("failed to load map"),error);
        }

    OutputMapExtent();

    if (iMapRenderer)
        {
        iMapRenderer = nullptr;
        iMapRenderer = std::make_unique<CartoType::CMapRenderer>(*iFramework,m_hWnd);
        }

    if (error == CartoType::KErrorNone)
        Update();
    }

void CCartoTypeDemoView::OnFileClose()
    {
    size_t index = iMapHandle.size();
    if (index)
        {
        index--;
        iFramework->UnloadMapByHandle(iMapHandle[index]);
        iMapHandle.erase(iMapHandle.begin() + index);
        Update();
        }
    }

void CCartoTypeDemoView::OnUpdateFileClose(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable(iMapHandle.size() > 0);
    }

void CCartoTypeDemoView::OnFileSaveAsPng()
    {
    CFileDialog file_dialog(false,_TEXT(".png"),nullptr,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,TEXT("PNG files (*.png)|*.png||"));
    if (file_dialog.DoModal() == IDOK)
        {
        CString p = file_dialog.GetPathName();
        CartoType::CString path;
        SetString(path,p);
        CartoType::TResult error;
        try
            {
            CartoType::CFileOutputStream output_stream(path);
            const CartoType::TBitmap* bitmap = iFramework->MapBitmap(error);
            if (!error)
                error = bitmap->WritePng(output_stream,false);
            }
        catch (CartoType::TResult e)
            {
            error = e;
            }
        if (error)
            {
            CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
            app->ShowError(_TEXT("failed to save as PNG"),error);
            }
        }
    }

void CCartoTypeDemoView::OnFileSaveAsCtsql()
    {
    CFileDialog file_dialog(false,_TEXT(".ctsql"),nullptr,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,TEXT("CartoType SQLite database files (*.ctsql)|*.ctsql||"));
    if (file_dialog.DoModal() == IDOK)
        {
        CString p = file_dialog.GetPathName();
        CartoType::CString path;
        SetString(path,p);
        CartoType::TResult error = iFramework->SaveMap(iFramework->GetMainMapHandle(),path,CartoType::TFileType::CTSQL);
        if (error)
            {
            CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
            app->ShowError(_TEXT("failed to save data to a CTSQL file"),error);
            }
        }
    }

void CCartoTypeDemoView::ShowNextFoundObject()
    {
    const auto& object = iFoundObject[iFoundObjectIndex];

    CartoType::CString summary;
    iFramework->GeoCodeSummary(summary,*object);
    CartoType::TResult error = 0;
    double length = object->LengthOrPerimeter(error);
    double area = 0;
    if (!error)
        area = object->Area(error);
    if (!error)
        {
        char buffer[64];
        if (area != 0)
            {
            if (iMetricUnits)
                {
                if (area > 1000000)
                    sprintf_s(buffer,"; area = %.2f sq km",area / 1000000);
                else
                    sprintf_s(buffer,"; area = %.2f ha",area / 10000);
                }
            else
                {
                // Convert to acres.
                double acres = area / 4046.856422;
                if (acres > 640)
                    sprintf_s(buffer,"; area = %.2f sq mi",acres / 640);
                else
                    sprintf_s(buffer,"; area = %.2f acres",acres);
                }
            summary.Append(buffer);
            }
        else if (length != 0)
            {
            if (iMetricUnits)
                {
                if (length > 1000)
                    sprintf_s(buffer,"; length = %.2f km",length / 1000);
                else
                    sprintf_s(buffer,"; length = %.2f m",length);
                }
            else
                {
                // Convert to yards.
                double yards = length / 0.9144;
                if (yards > 1760)
                    sprintf_s(buffer,"; length = %.2f mi",yards / 1760);
                else
                    sprintf_s(buffer,"; length = %.2f yards",yards);
                }
            summary.Append(buffer);
            }
        }

    CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
    CMainFrame* main_window = (CMainFrame*)app->m_pMainWnd;
    summary.Append((uint16_t)0);
    main_window->SetMessageText((LPCTSTR)summary.Text());

    CartoType::TTextLiteral(found_layer,u"found");
    if (object->Type() == CartoType::TMapObjectType::Line)
        {
        auto envelope = object->Envelope(5.0 / iFramework->MapUnitSize());
        error = iFramework->InsertMapObject(0,found_layer,CartoType::CGeometry(envelope,CartoType::TCoordType::Map,true),"",0,iFoundObjectId,true);
        }
    else
        {
        error = iFramework->InsertCopyOfMapObject(0,found_layer,*object,object->Type() == CartoType::TMapObjectType::Point ? 50 : 0,
                                                  CartoType::TCoordType::MapMeter,iFoundObjectId,true);
        }

    if (!error)
        {
        bool animate = iFramework->SetAnimateTransitions(true);
        iFramework->SetView(*object,64,4000);
        iFramework->SetAnimateTransitions(animate);
        }
    if (error)
        app->ShowError(_TEXT("Error displaying found object"),error);

    iFoundObjectIndex = (iFoundObjectIndex + 1) % iFoundObject.size();
    Update();
    }

void CCartoTypeDemoView::OnFind()
    {
    iFindTextDialog.Set(iFramework.get());

    bool stop = false;
    iFoundObjectIndex = 0;
    while (!stop)
        {
        if (iFindTextDialog.DoModal() == IDOK)
            {
            iFoundObject = iFindTextDialog.FoundObjectArray();

            if (iFoundObject.size() == 0)
                {
                ::CString msg("Text ");
                msg += iFindTextDialog.iFindText;
                msg += " not found";
                AfxMessageBox(msg);
                }
            else
                {
                uint64_t count = 0;
                iFramework->DeleteMapObjects(0,iFoundObjectId,iFoundObjectId,count);
                iFoundObjectId = 0;
                ShowNextFoundObject();

                // Insert pushpins for first 10 found objects.
                iFramework->DeleteMapObjects(0,0,UINT64_MAX,count,"@layer=='pushpin' and Type==3");
                CartoType::CString layer("pushpin");
                int n = 0;
                for (const auto& p : iFoundObject)
                    {
                    auto centre = p->CenterOfGravity();
                    uint64_t id = 0;
                    iFramework->InsertPointMapObject(0,layer,centre.iX,centre.iY,CartoType::TCoordType::Map,"",3,id,false);
                    if (++n == 10)
                        break;
                    }

                stop = true;
                }
            }
        else
            stop = true;
        }
    }

void CCartoTypeDemoView::OnUpdateFind(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    }

void CCartoTypeDemoView::OnFindAddress()
    {
    CFindAddressDialog find_dialog;
    SetString(find_dialog.iBuilding,iAddress.iBuilding);
    SetString(find_dialog.iFeature,iAddress.iFeature);
    SetString(find_dialog.iStreet,iAddress.iStreet);
    SetString(find_dialog.iSubLocality,iAddress.iSubLocality);
    SetString(find_dialog.iLocality,iAddress.iLocality);
    SetString(find_dialog.iSubAdminArea,iAddress.iSubAdminArea);
    SetString(find_dialog.iAdminArea,iAddress.iAdminArea);
    SetString(find_dialog.iCountry,iAddress.iCountry);
    SetString(find_dialog.iPostCode,iAddress.iPostCode);

    bool stop = false;

    iFoundObjectIndex = 0;
    while (!stop)
        {
        if (find_dialog.DoModal() == IDOK)
            {
            CWaitCursor cursor;

            iFoundObject.clear();
            CartoType::TResult error = 0;
            SetString(iAddress.iBuilding,find_dialog.iBuilding);
            SetString(iAddress.iFeature,find_dialog.iFeature);
            SetString(iAddress.iStreet,find_dialog.iStreet);
            SetString(iAddress.iSubLocality,find_dialog.iSubLocality);
            SetString(iAddress.iLocality,find_dialog.iLocality);
            SetString(iAddress.iSubAdminArea,find_dialog.iSubAdminArea);
            SetString(iAddress.iAdminArea,find_dialog.iAdminArea);
            SetString(iAddress.iCountry,find_dialog.iCountry);
            SetString(iAddress.iPostCode,find_dialog.iPostCode);
            error = iFramework->FindAddress(iFoundObject,100,iAddress,true);

            if (iFoundObject.size() == 0)
                {
                ::CString msg("address not found");
                AfxMessageBox(msg);
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
    }

void CCartoTypeDemoView::OnUpdateFindAddress(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    }

void CCartoTypeDemoView::OnFindNext()
    {
    ShowNextFoundObject();
    }

void CCartoTypeDemoView::OnUpdateFindNext(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable(iFoundObject.size() > 1);
    }

void CCartoTypeDemoView::OnViewReloadStyleSheet()
    {
    iFramework->ReloadStyleSheet();
    CreateLegend();
    Update();
    }

void CCartoTypeDemoView::OnUpdateViewReloadStyleSheet(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    }

void CCartoTypeDemoView::OnViewZoomIn()
    {
    CPoint point;
    bool set_centre = SetCentre(point);

    CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
    if (set_centre)
        iFramework->ZoomAt(app->ZoomFactor(),point.x,point.y,CartoType::TCoordType::Display);
    else
        iFramework->Zoom(app->ZoomFactor());
    Update();
    }

void CCartoTypeDemoView::OnUpdateViewZoomIn(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    }

void CCartoTypeDemoView::OnViewZoomOut()
    {
    CPoint point;
    bool set_centre = SetCentre(point);

    CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
    if (set_centre)
        iFramework->ZoomAt(1.0 / app->ZoomFactor(),point.x,point.y,CartoType::TCoordType::Display);
    else
        iFramework->Zoom(1.0 / app->ZoomFactor());
    Update();
    }

void CCartoTypeDemoView::OnUpdateViewZoomOut(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    }

void CCartoTypeDemoView::OnViewRotate()
    {
    CPoint point;
    if (SetCentre(point))
        iFramework->RotateAndZoom(15,1,point.x,point.y,CartoType::TCoordType::Display);
    else
        iFramework->Rotate(15);
    Update();
    }

void CCartoTypeDemoView::OnUpdateViewRotate(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    }

void CCartoTypeDemoView::OnViewNorthup()
    {
    iFramework->SetRotation(0);
    Update();
    }

void CCartoTypeDemoView::OnUpdateViewNorthup(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable(iFramework->Rotation() != 0);
    }

void CCartoTypeDemoView::OnViewLatlong()
    {
    CLatLongDialog lat_long_dialog;

    double minx,miny,maxx,maxy;
    iFramework->GetView(minx,miny,maxx,maxy,CartoType::TCoordType::Degree);
    double cur_long = (minx + maxx) / 2;
    double cur_lat = (miny + maxy) / 2;
    iFramework->GetMapExtent(minx,miny,maxx,maxy,CartoType::TCoordType::Degree);

    char buffer[32];
    sprintf_s(buffer,"%.4f",cur_long);
    SetString(lat_long_dialog.iLongitude,buffer);
    sprintf_s(buffer,"%.4f",cur_lat);
    SetString(lat_long_dialog.iLatitude,buffer);

    if (lat_long_dialog.DoModal() == IDOK)
        {
        CartoType::TTextBuffer<32> lo;
        SetString(lo,lat_long_dialog.iLongitude);
        CartoType::TTextBuffer<32> la;
        SetString(la,lat_long_dialog.iLatitude);
        double x = lo.ToDouble();
        double y = la.ToDouble();

        if (x < minx)
            x = minx;
        else if (x > maxx)
            x = maxx;

        if (y < miny)
            y = miny;
        else if (y > maxy)
            y = maxy;

        CartoType::TResult error = iFramework->SetViewCenter(x,y,CartoType::TCoordType::Degree);
        if (error)
            {
            CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
            app->ShowError(_TEXT("Set center error"),error);
            }
        Update();
        }
    }

void CCartoTypeDemoView::OnUpdateViewLatlong(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    }

void CCartoTypeDemoView::OnViewStylesheet()
    {
    CFileDialog file_dialog(true,_TEXT(".ctstyle"),nullptr,0,TEXT("style sheets (*.ctstyle)|*.ctstyle||"));
    if (file_dialog.DoModal() == IDOK)
        LoadStyleSheet(file_dialog.GetPathName());
    }

void CCartoTypeDemoView::OnUpdateViewStylesheet(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    }

void CCartoTypeDemoView::OnViewLayers()
    {
    CLayerDialog dialog(*iFramework);
    if (dialog.DoModal() == IDOK)
        Update();
    }

void CCartoTypeDemoView::OnUpdateViewLayers(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    }

void CCartoTypeDemoView::OnViewLegend()
    {
    iDrawLegend = !iDrawLegend;
    iFramework->EnableLegend(iDrawLegend);
    iFramework->ForceRedraw();
    Update();
    }

void CCartoTypeDemoView::OnUpdateViewLegend(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    pCmdUI->SetCheck(iDrawLegend);
    }

void CCartoTypeDemoView::OnViewSetScale()
    {
    CSetScaleDialog dialog;
    dialog.iScale.Format(_TEXT("%d"),(int)iFramework->ScaleDenominator());
    if (dialog.DoModal() == IDOK)
        {
        CartoType::TTextBuffer<32> scale_text;
        SetString(scale_text,dialog.iScale);
        iFramework->SetScaleDenominatorInView(scale_text.ToInt32());
        Update();
        }
    }

void CCartoTypeDemoView::OnUpdateViewSetScale(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    }

void CCartoTypeDemoView::OnViewPerspective()
    {
    iFramework->SetPerspective(!iFramework->Perspective());
    Update();
    }

void CCartoTypeDemoView::OnUpdateViewPerspective(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    pCmdUI->SetCheck(iFramework->Perspective());
    }

void CCartoTypeDemoView::OnViewMetricUnits()
    {
    iMetricUnits = !iMetricUnits;
    iFramework->SetLocale(iMetricUnits ? "en_xx" : "en");
    CreateLegend();
    iFramework->SetScaleBar(iMetricUnits,2.5,"in",CartoType::TNoticePosition::BottomLeft);
    iFramework->SetTurnInstructions(iMetricUnits,true,2,"in",CartoType::TNoticePosition::TopLeft);
    if (iDrawLegend || iDrawTurnInstructions)
        {
        iFramework->ForceRedraw();
        Update();
        }
    }

void CCartoTypeDemoView::OnUpdateViewMetricUnits(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    pCmdUI->SetCheck(iMetricUnits);
    }

void CCartoTypeDemoView::OnViewTurnInstructions()
    {
    iDrawTurnInstructions = !iDrawTurnInstructions;
    iFramework->EnableTurnInstructions(iDrawTurnInstructions);
    iFramework->ForceRedraw();
    Update();
    }

void CCartoTypeDemoView::OnUpdateViewTurnInstructions(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable(iFramework->Route() != nullptr);
    pCmdUI->SetCheck(iDrawTurnInstructions);
    }

void CCartoTypeDemoView::StopFlyThrough()
    {
    if (iFlyThroughTimer)
        {
        KillTimer(iFlyThroughTimer);
        iFlyThroughTimer = 0;
        iRouteIter = nullptr;
        iFramework->SetFollowMode(CartoType::TFollowMode::None);
        iFramework->SetPerspective(false);
        iFramework->SetRotation(0);
        Update();
        }
    }

void CCartoTypeDemoView::OnViewFlyThrough()
    {
    // If doing a fly-through, cancel it.
    if (iFlyThroughTimer)
        {
        StopFlyThrough();
        return;
        }

    iRouteIter.reset(new CartoType::TRouteIterator(*iFramework->Route()));
    if (iRouteIter)
        {
        iFramework->SetFollowMode(CartoType::TFollowMode::LocationHeading);
        iFlyThroughTimer = (UINT)SetTimer(1,UINT(iTimerIntervalInSeconds * 1000),0);
        }
    }

void CCartoTypeDemoView::OnUpdateViewFlyThrough(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable(iFramework->Navigating());
    pCmdUI->SetCheck(iFlyThroughTimer != 0);
    }

void CCartoTypeDemoView::OnGoAlongRoute()
    {
    bool at_end = false;
    SimulateNavigationFix(true,at_end);

    // Temporarily turn the follow mode on so that the view jumps to the position.
    iFramework->SetFollowMode(CartoType::TFollowMode::Location);
    iFramework->SetFollowMode(CartoType::TFollowMode::None);

    // Uncomment to test position extrapolation.
    // CartoType::TNavigationData n;
    // n.iValidity = CartoType::TNavigationData::ESpeedValid;
    // n.iSpeed = 100;
    // iFramework->Navigate(n);

    Update();
    }

void CCartoTypeDemoView::OnUpdateGoAlongRoute(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable(iFramework->Navigating());
    }

void CCartoTypeDemoView::OnReverseRoute()
    {
    std::reverse(iRoutePointArray.begin(),iRoutePointArray.end());
    if (iRoutePointArray[0].iPoint.iPoint != CartoType::TPointFP() &&
        iRoutePointArray[iRoutePointArray.size() - 1].iPoint.iPoint != CartoType::TPointFP())
        CalculateAndDisplayRoute();
    }

void CCartoTypeDemoView::OnUpdateReverseRoute(CCmdUI* pCmdUI)
    {
    pCmdUI->Enable(iFramework->Route() != nullptr);
    }

void CCartoTypeDemoView::OnDeleteRoute()
    {
    StopFlyThrough();
    iRoutePointArray.resize(2);
    iRoutePointArray[0] = iRoutePointArray[1] = TRoutePoint();
    iFramework->DeleteRoutes();
    Update();
    }

void CCartoTypeDemoView::OnUpdateDeleteRoute(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable(iFramework->Route() != nullptr);
    }

void CCartoTypeDemoView::SetProfile(size_t aProfileIndex)
    {
    StopFlyThrough();

    size_t b = iFramework->BuiltInProfileCount();
    if (b == 0)
        {
        iRouteProfileIndex = aProfileIndex;
        auto type = CartoType::TRouteProfileType::Car;
        switch (aProfileIndex)
            {
            case 1: type = CartoType::TRouteProfileType::Cycle; break;
            case 2: type = CartoType::TRouteProfileType::Walk; break;
            case 3: type = CartoType::TRouteProfileType::Hike; break;
            default: break;
            }
        CartoType::TRouteProfile p = type;
        p.iShortest = iShortestRoute;
        if (!iUseGradients)
            p.iGradientFlags = 0;
        iFramework->SetMainProfile(p);
        }
    else
        {
        iFramework->SetBuiltInProfile(aProfileIndex);
        if (aProfileIndex < b)
            {
            iRouteProfileIndex = aProfileIndex;
            iShortestRoute = iFramework->BuiltInProfile(aProfileIndex)->iShortest;
            iUseGradients = iFramework->BuiltInProfile(aProfileIndex)->iGradientFlags != 0;
            }
        }
    if (iRoutePointArray[0].iPoint.iPoint != CartoType::TPointFP() &&
        iRoutePointArray[iRoutePointArray.size() - 1].iPoint.iPoint != CartoType::TPointFP())
        CalculateAndDisplayRoute();
    }

void CCartoTypeDemoView::OutputMapExtent()
    {
    double min_long,min_lat,max_long,max_lat;
    auto error = iFramework->GetMapExtent(min_long,min_lat,max_long,max_lat,CartoType::TCoordType::Degree);
    if (!error)
        {
        char buffer[256];
        sprintf_s(buffer,"map extent: %g%s %g%s ... %g%s %g%s\n",
                  min_long < 0 ? -min_long : min_long,min_long < 0 ? "W" : "E",
                  min_lat < 0 ? -min_lat : min_lat,min_lat < 0 ? "S" : "N",
                  max_long < 0 ? -max_long : max_long,max_long < 0 ? "W" : "E",
                  max_lat < 0 ? -max_lat : max_lat,max_lat < 0 ? "S" : "N");
        OutputDebugStringA(buffer);
        }
    }

// Set the drive profile, or profile 0 if using built-in profiles.
void CCartoTypeDemoView::OnSetDriveProfile()
    {
    SetProfile(0);
    }

void CCartoTypeDemoView::SetMenuItemToBuiltInProfileName(CCmdUI* aMenuItem,size_t aProfileIndex)
    {
    if (iFramework->BuiltInProfileCount() > aProfileIndex)
        {
        aMenuItem->Enable(true);
        CartoType::CString s = iFramework->BuiltInProfile(aProfileIndex)->iName;
        s.SetCase(CartoType::TLetterCase::Title);
        s += " (built-in)";
        s.Append(uint16_t(0));
        aMenuItem->SetText((LPCTSTR)s.Text());
        }
    else
        {
        aMenuItem->Enable(false);
        aMenuItem->SetText(_TEXT("(none)"));
        }
    }

void CCartoTypeDemoView::OnUpdateSetDriveProfile(CCmdUI *pCmdUI)
    {
    if (iFramework->BuiltInProfileCount())
        SetMenuItemToBuiltInProfileName(pCmdUI,0);
    else
        pCmdUI->SetText(_TEXT("Drive"));
    pCmdUI->SetCheck(iRouteProfileIndex == 0);
    }

// Set the cycle profile, or profile 1 if using built-in profiles.
void CCartoTypeDemoView::OnSetCycleProfile()
    {
    SetProfile(1);
    }

void CCartoTypeDemoView::OnUpdateSetCycleProfile(CCmdUI *pCmdUI)
    {
    if (iFramework->BuiltInProfileCount())
        SetMenuItemToBuiltInProfileName(pCmdUI,1);
    else
        pCmdUI->SetText(_TEXT("Cycle"));
    pCmdUI->SetCheck(iRouteProfileIndex == 1);
    }

// Set the walk profile, or profile 2 if using built-in profiles.
void CCartoTypeDemoView::OnSetWalkProfile()
    {
    SetProfile(2);
    }

void CCartoTypeDemoView::OnUpdateSetWalkProfile(CCmdUI *pCmdUI)
    {
    if (iFramework->BuiltInProfileCount())
        SetMenuItemToBuiltInProfileName(pCmdUI,2);
    else
        pCmdUI->SetText(_TEXT("Walk"));
    pCmdUI->SetCheck(iRouteProfileIndex == 2);
    }

// Set the hike profile, or profile 3 if using built-in profiles.
void CCartoTypeDemoView::OnSetHikeProfile()
    {
    SetProfile(3);
    }

void CCartoTypeDemoView::OnUpdateSetHikeProfile(CCmdUI* pCmdUI)
    {
    if (iFramework->BuiltInProfileCount())
        SetMenuItemToBuiltInProfileName(pCmdUI,3);
    else
        pCmdUI->SetText(_TEXT("Hike"));
    pCmdUI->SetCheck(iRouteProfileIndex == 3);
    }

void CCartoTypeDemoView::OnShortestRoute()
    {
    iShortestRoute = !iShortestRoute;
    SetProfile(iRouteProfileIndex);
    }

void CCartoTypeDemoView::OnUpdateShortestRoute(CCmdUI* pCmdUI)
    {
    pCmdUI->Enable(iFramework->BuiltInProfileCount() == 0);
    pCmdUI->SetCheck(iShortestRoute);
    }

void CCartoTypeDemoView::OnUseGradients()
    {
    iUseGradients = !iUseGradients;
    SetProfile(iRouteProfileIndex);
    }

void CCartoTypeDemoView::OnUpdateUseGradients(CCmdUI* pCmdUI)
    {
    pCmdUI->Enable(iFramework->BuiltInProfileCount() == 0 && iFramework->NavigationDataHasGradients());
    pCmdUI->SetCheck(iUseGradients);
    }

void CCartoTypeDemoView::OnSaveRouteInstructions()
    {
    CFileDialog file_dialog(false,_TEXT(".txt"),nullptr,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,TEXT("Text files (*.txt)|*.txt||"));
    if (file_dialog.DoModal() == IDOK)
        {
        CString p = file_dialog.GetPathName();
        CartoType::CString path;
        SetString(path,p);
        std::string path_utf8(path);
        CartoType::TResult error = 0;
        FILE* file = nullptr;
        fopen_s(&file,path_utf8.c_str(),"w");
        if (!file)
            error = CartoType::KErrorIo;
        if (!error)
            {
            std::string text(iFramework->RouteInstructions(*iFramework->Route()));
            size_t n = fwrite(text.c_str(),text.length(),1,file);
            if (!n)
                error = CartoType::KErrorIo;
            }
        fclose(file);
        if (error)
            {
            CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
            app->ShowError(_TEXT("failed to save the route instructions to a text file"),error);
            }
        Update();
        }
    }

void CCartoTypeDemoView::OnUpdateSaveRouteInstructions(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable(iFramework->Route() != nullptr);
    }

void CCartoTypeDemoView::OnSaveRouteXml()
    {
    CFileDialog file_dialog(false,_TEXT(".ctroute"),nullptr,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,TEXT("CartoType Route files (*.ctroute)|*.ctroute||"));
    if (file_dialog.DoModal() == IDOK)
        {
        CString p = file_dialog.GetPathName();
        CartoType::CString path;
        SetString(path,p);
        CartoType::TResult error = iFramework->WriteRouteAsXml(*iFramework->Route(),path);
        if (error)
            {
            CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
            app->ShowError(_TEXT("failed to save the route to a .ctroute file"),error);
            }
        Update();
        }
    }

void CCartoTypeDemoView::OnUpdateSaveRouteXml(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable(iFramework->Route() != nullptr);
    }

void CCartoTypeDemoView::OnSaveRouteGpx()
    {
    CFileDialog file_dialog(false,_TEXT(".gpx"),nullptr,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,TEXT("GPX files (*.gpx)|*.gpx||"));
    if (file_dialog.DoModal() == IDOK)
        {
        CString p = file_dialog.GetPathName();
        CartoType::CString path;
        SetString(path,p);
        CartoType::TResult error = iFramework->WriteRouteAsXml(*iFramework->Route(),path,CartoType::TFileType::GPX);
        if (error)
            {
            CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
            app->ShowError(_TEXT("failed to save the route to a GPX file"),error);
            }
        Update();
        }
    }

void CCartoTypeDemoView::OnUpdateSaveRouteGpx(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable(iFramework->Route() != nullptr);
    }

void CCartoTypeDemoView::OnLoadRouteXml()
    {
    CFileDialog file_dialog(true,_TEXT(".ctroute"),nullptr,0,TEXT("CartoType Route files (*.ctroute)|*.ctroute|GPX files (*.gpx)|*.gpx||"));
    if (file_dialog.DoModal() == IDOK)
        {
        CString p = file_dialog.GetPathName();
        CartoType::CString path;
        SetString(path,p);
        CartoType::TResult error = iFramework->ReadRouteFromXml(path,true);
        if (iFramework->Route())
            {
            iRouteIter = std::make_unique<CartoType::TRouteIterator>(*iFramework->Route());
            bool at_end = false;
            SimulateNavigationFix(false,at_end);
            }
        if (error)
            {
            CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
            app->ShowError(_TEXT("failed to read a route from a file"),error);
            }
        Update();
        }
    }

void CCartoTypeDemoView::OnUpdateLoadRouteXml(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    }

void CCartoTypeDemoView::OnTimer(UINT_PTR aEventId)
    {
    if (iManualDrawTimer)
        iMapRenderer->Draw();

    if (iFlyThroughTimer && iRouteIter)
        {
        bool at_end = false;
        SimulateNavigationFix(false,at_end);

        // Stop the fly-through if we have reached the end.
        if (at_end)
            StopFlyThrough();

        Update();
        }
    }

void CCartoTypeDemoView::OnEditLoadKml()
    {
    CFileDialog file_dialog(true,_TEXT(".kml"),nullptr,0,TEXT("KML files (*.kml)|*.kml||"));
    if (file_dialog.DoModal() == IDOK)
        {
        CString p = file_dialog.GetPathName();
        CartoType::CString path;
        SetString(path,p);
        CartoType::TResult error = iFramework->ReadMap(iWritableMapHandle,path,CartoType::TFileType::KML);
        if (error)
            {
            CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
            app->ShowError(_TEXT("failed to read data from a KML file"),error);
            }
        Update();
        }
    }

void CCartoTypeDemoView::OnUpdateEditLoadKml(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    }

void CCartoTypeDemoView::OnEditSaveKml()
    {
    CFileDialog file_dialog(false,_TEXT(".kml"),nullptr,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,TEXT("KML files (*.kml)|*.kml||"));
    if (file_dialog.DoModal() == IDOK)
        {
        CString p = file_dialog.GetPathName();
        CartoType::CString path;
        SetString(path,p);
        CartoType::TResult error = iFramework->SaveMap(iWritableMapHandle,path,CartoType::TFileType::KML);
        if (error)
            {
            CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
            app->ShowError(_TEXT("failed to write data to a KML file"),error);
            }
        else
            iWritableMapChanged = false;
        }
    }

void CCartoTypeDemoView::OnUpdateEditSaveKml(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable(!iFramework->MapIsEmpty(iWritableMapHandle));
    }

void CCartoTypeDemoView::OnEditLoadCtms()
    {
    CFileDialog file_dialog(true,_TEXT(".ctms"),nullptr,0,TEXT("CTMS files (*.ctms)|*.ctms||"));
    if (file_dialog.DoModal() == IDOK)
        {
        CString p = file_dialog.GetPathName();
        CartoType::CString path;
        SetString(path,p);
        CartoType::TResult error = iFramework->ReadMap(iWritableMapHandle,path,CartoType::TFileType::CTMS);
        if (error)
            {
            CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
            app->ShowError(_TEXT("failed to read data from a CTMS file"),error);
            }
        Update();
        }
    }

void CCartoTypeDemoView::OnUpdateEditLoadCtms(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    }

void CCartoTypeDemoView::OnEditSaveCtms()
    {
    CFileDialog file_dialog(false,_TEXT(".ctms"),nullptr,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,TEXT("CTMS files (*.ctms)|*.ctms||"));
    if (file_dialog.DoModal() == IDOK)
        {
        CString p = file_dialog.GetPathName();
        CartoType::CString path;
        SetString(path,p);
        CartoType::TResult error = iFramework->SaveMap(iWritableMapHandle,path,CartoType::TFileType::CTMS);
        if (error)
            {
            CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
            app->ShowError(_TEXT("failed to save data to a CTMS file"),error);
            }
        else
            iWritableMapChanged = false;
        }
    }

void CCartoTypeDemoView::OnUpdateEditSaveCtms(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable(!iFramework->MapIsEmpty(iWritableMapHandle));
    }

void CCartoTypeDemoView::OnEditLoadCtsql()
    {
    CFileDialog file_dialog(true,_TEXT(".ctsql"),nullptr,0,TEXT("CTSQL files (*.ctsql)|*.ctsql||"));
    if (file_dialog.DoModal() == IDOK)
        {
        CString p = file_dialog.GetPathName();
        CartoType::CString path;
        SetString(path,p);
        CartoType::TResult error = iFramework->ReadMap(iWritableMapHandle,path,CartoType::TFileType::CTSQL);
        if (error)
            {
            CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
            app->ShowError(_TEXT("failed to read data from a CTSQL file"),error);
            }
        Update();
        }
    }

void CCartoTypeDemoView::OnUpdateEditLoadCtsql(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    }

void CCartoTypeDemoView::OnEditSaveCtsql()
    {
    CFileDialog file_dialog(false,_TEXT(".ctsql"),nullptr,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,TEXT("CartoType SQLite database files (*.ctsql)|*.ctsql||"));
    if (file_dialog.DoModal() == IDOK)
        {
        CString p = file_dialog.GetPathName();
        CartoType::CString path;
        SetString(path,p);
        CartoType::TResult error = iFramework->SaveMap(iWritableMapHandle,path,CartoType::TFileType::CTSQL);
        if (error)
            {
            CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
            app->ShowError(_TEXT("failed to save data to a CTSQL file"),error);
            }
        else
            iWritableMapChanged = false;
        }
    }

void CCartoTypeDemoView::OnUpdateEditSaveCtsql(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable(!iFramework->MapIsEmpty(iWritableMapHandle));
    }

void CCartoTypeDemoView::OnEditLoadGpx()
    {
    CFileDialog file_dialog(true,_TEXT(".gpx"),nullptr,0,TEXT("GPX files (*.gpx)|*.gpx||"));
    if (file_dialog.DoModal() == IDOK)
        {
        CString p = file_dialog.GetPathName();
        CartoType::CString path;
        SetString(path,p);
        CartoType::TResult error = iFramework->ReadGpx(iWritableMapHandle,path);
        if (error)
            {
            CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
            app->ShowError(_TEXT("failed to read data from a GPX file"),error);
            }
        Update();
        }
    }

void CCartoTypeDemoView::OnUpdateEditLoadGpx(CCmdUI *pCmdUI)
    {
    pCmdUI->Enable();
    }
