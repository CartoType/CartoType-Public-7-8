#include "polygonstyledialog.h"
#include "ui_polygonstyledialog.h"
#include "util.h"
#include <QColorDialog>

PolygonStyleDialog::PolygonStyleDialog(QWidget& aParent,CStyleItem& aStyleItem,CartoType::CLegend& aLegend):
    QDialog(&aParent),
    m_ui(new Ui::PolygonStyleDialog),
    m_style_item(aStyleItem),
    m_legend(aLegend)
    {
    m_ui->setupUi(this);

    m_ui->fillColorText->setText(m_style_item.Attribute("fill"));
    m_ui->borderColorText->setText(m_style_item.Attribute("border"));
    m_ui->borderWidthText->setText(m_style_item.Attribute("borderWidth"));
    m_ui->opacityText->setText(m_style_item.Attribute("opacity"));
    m_ui->dashArrayText->setText(m_style_item.Attribute("dashArray"));
    m_ui->drawHeightCheckBox->setChecked(!strcmp(m_style_item.Attribute("height"),"yes"));

    DrawPolygonSample();
    }

PolygonStyleDialog::~PolygonStyleDialog()
    {
    delete m_ui;
    }

void PolygonStyleDialog::DrawPolygonSample()
    {
    std::string s("<CartoTypeStyleSheet><layer name='_temp'>");
    s += m_style_item.Xml();
    s += "</layer></CartoTypeStyleSheet>";
    m_legend.SetMainStyleSheet((const uint8_t*)s.data(),s.length());
    m_legend.SetBackgroundColor(CartoType::KWhite);
    m_legend.SetMarginWidth(0,"px");
    m_legend.SetMinLineHeight(1.5,"cm");
    m_legend.Clear();

    CartoType::CString string_attrib;
    string_attrib.SetAttribute("_t","40");

    bool draw_height = m_ui->drawHeightCheckBox->isChecked();
    if (draw_height)
        m_legend.SetPolygonRotation(20);

    m_legend.AddMapObjectLine(CartoType::TMapObjectType::Polygon,"_temp",nullptr,0,string_attrib,"");
    CartoType::TResult error;
    std::unique_ptr<CartoType::CBitmap> bitmap { m_legend.CreateLegend(error,2,"cm",5000,5000) };
    if (!error)
        {
        QImage image(bitmap->Width(),bitmap->Height(),QImage::Format_ARGB32_Premultiplied);
        CopyBitmapToImage(*bitmap,image);
        m_ui->image->setPixmap(QPixmap::fromImage(std::move(image)));
        }

    if (draw_height)
        m_legend.SetPolygonRotation(0);
    }

void PolygonStyleDialog::EditingFinishedHelper(const char* aKey,QLineEdit& aLineEdit)
    {
    QString cur_value(m_style_item.Attribute(aKey));
    if (aLineEdit.text() != cur_value)
        {
        m_style_item.SetAttribute(aKey,aLineEdit.text().toStdString());
        DrawPolygonSample();
        }
    }

void PolygonStyleDialog::ColorButtonHelper(const char* aKey,QLineEdit& aLineEdit)
    {
    if (PickColor(*this,aLineEdit))
        {
        m_style_item.SetAttribute(aKey,aLineEdit.text().toStdString());
        DrawPolygonSample();
        }
    }

void PolygonStyleDialog::on_fillColorText_editingFinished()
    {
    EditingFinishedHelper("fill",*m_ui->fillColorText);
    }

void PolygonStyleDialog::on_borderColorText_editingFinished()
    {
    EditingFinishedHelper("border",*m_ui->borderColorText);
    }

void PolygonStyleDialog::on_borderWidthText_editingFinished()
    {
    EditingFinishedHelper("borderWidth",*m_ui->borderWidthText);
    }

void PolygonStyleDialog::on_opacityText_editingFinished()
    {
    EditingFinishedHelper("opacity",*m_ui->opacityText);
    }

void PolygonStyleDialog::on_dashArrayText_editingFinished()
    {
    EditingFinishedHelper("dashArray",*m_ui->dashArrayText);
    }

void PolygonStyleDialog::on_drawHeightCheckBox_stateChanged(int aState)
    {
    if (aState == Qt::Checked)
        m_style_item.SetAttribute("height","yes");
    else
        m_style_item.SetAttribute("height","");
    DrawPolygonSample();
    }

void PolygonStyleDialog::on_fillColorButton_clicked()
    {
    ColorButtonHelper("fill",*m_ui->fillColorText);
    }

void PolygonStyleDialog::on_borderColorButton_clicked()
    {
    ColorButtonHelper("border",*m_ui->borderColorText);
    }
