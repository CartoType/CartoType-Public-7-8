#include "linestyledialog.h"
#include "ui_linestyledialog.h"
#include "util.h"
#include <QColorDialog>

LineStyleDialog::LineStyleDialog(QWidget& aParent,CStyleItem& aStyleItem,CartoType::CLegend& aLegend):
    QDialog(&aParent),
    m_ui(new Ui::LineStyleDialog),
    m_style_item(aStyleItem),
    m_legend(aLegend)
    {
    m_ui->setupUi(this);

    m_ui->lineWidthText->setText(m_style_item.Attribute("width"));
    m_ui->lineColorText->setText(m_style_item.Attribute("fill"));
    m_ui->lineDashText->setText(m_style_item.Attribute("dashArray"));
    m_ui->borderWidthText->setText(m_style_item.Attribute("borderWidth"));
    m_ui->borderColorText->setText(m_style_item.Attribute("border"));
    m_ui->centerLineWidthText->setText(m_style_item.Attribute("centerLineWidth"));
    m_ui->centerLineColorText->setText(m_style_item.Attribute("centerLine"));
    m_ui->opacityText->setText(m_style_item.Attribute("opacity"));
    m_ui->truncationAtStartText->setText(m_style_item.Attribute("truncationAtStart"));
    m_ui->truncationAtEndText->setText(m_style_item.Attribute("truncationAtEnd"));

    if (aStyleItem.ElementName() == "highlight")
        m_ui->distanceText->setText(m_style_item.Attribute("distance"));
    else
        {
        m_ui->distanceLabel->hide();
        m_ui->distanceText->hide();
        }

    DrawLineSamples();
    }

LineStyleDialog::~LineStyleDialog()
    {
    delete m_ui;
    }

void LineStyleDialog::DrawLineSamples()
    {
    std::string s("<CartoTypeStyleSheet><layer name='_temp'>");
    s += m_style_item.Xml();
    s += "</layer></CartoTypeStyleSheet>";
    m_legend.SetMainStyleSheet((const uint8_t*)s.data(),s.length());
    m_legend.SetBackgroundColor(CartoType::KWhite);
    m_legend.SetMarginWidth(0,"px");
    m_legend.SetMinLineHeight(1.5,"cm");

    static const int scale_array[9] = { 5000, 10000, 25000, 50000, 100000, 250000, 500000, 1000000, 2500000 };
    DrawLineSample(*m_ui->scale0,*m_ui->image0,scale_array[0]);
    DrawLineSample(*m_ui->scale1,*m_ui->image1,scale_array[1]);
    DrawLineSample(*m_ui->scale2,*m_ui->image2,scale_array[2]);
    DrawLineSample(*m_ui->scale3,*m_ui->image3,scale_array[3]);
    DrawLineSample(*m_ui->scale4,*m_ui->image4,scale_array[4]);
    DrawLineSample(*m_ui->scale5,*m_ui->image5,scale_array[5]);
    DrawLineSample(*m_ui->scale6,*m_ui->image6,scale_array[6]);
    DrawLineSample(*m_ui->scale7,*m_ui->image7,scale_array[7]);
    DrawLineSample(*m_ui->scale8,*m_ui->image8,scale_array[8]);
    }

void LineStyleDialog::DrawLineSample(QLabel& aText,QLabel& aImage,int aScale)
    {
    QString s;
    s.setNum(aScale);
    QString text("1:");
    text += s;
    aText.setText(text);

    m_legend.Clear();
    m_legend.AddMapObjectLine(CartoType::TMapObjectType::Line,"_temp",nullptr,0,"","");
    CartoType::TResult error;
    std::unique_ptr<CartoType::CBitmap> bitmap { m_legend.CreateLegend(error,2,"cm",aScale,aScale) };
    if (bitmap)
        {
        QImage image(bitmap->Width(),bitmap->Height(),QImage::Format_ARGB32_Premultiplied);
        CopyBitmapToImage(*bitmap,image);
        aImage.setPixmap(QPixmap::fromImage(std::move(image)));
        }
    }

void LineStyleDialog::on_lineColorText_editingFinished()
    {
    EditingFinishedHelper("fill",*m_ui->lineColorText);
    }

void LineStyleDialog::on_lineWidthText_editingFinished()
    {
    EditingFinishedHelper("width",*m_ui->lineWidthText);
    }

void LineStyleDialog::on_borderColorText_editingFinished()
    {
    EditingFinishedHelper("border",*m_ui->borderColorText);
    }

void LineStyleDialog::on_borderWidthText_editingFinished()
    {
    EditingFinishedHelper("borderWidth",*m_ui->borderWidthText);
    }

void LineStyleDialog::on_centerLineColorText_editingFinished()
    {
    EditingFinishedHelper("centerLine",*m_ui->centerLineColorText);
    }

void LineStyleDialog::on_centerLineWidthText_editingFinished()
    {
    EditingFinishedHelper("centerLineWidth",*m_ui->centerLineWidthText);
    }

void LineStyleDialog::on_lineDashText_editingFinished()
    {
    EditingFinishedHelper("dashArray",*m_ui->lineDashText);
    }

void LineStyleDialog::on_opacityText_editingFinished()
    {
    EditingFinishedHelper("opacity",*m_ui->opacityText);
    }

void LineStyleDialog::on_truncationAtStartText_editingFinished()
    {
    EditingFinishedHelper("truncationAtStart",*m_ui->truncationAtStartText);
    }

void LineStyleDialog::on_truncationAtEndText_editingFinished()
    {
    EditingFinishedHelper("truncationAtEnd",*m_ui->truncationAtEndText);
    }

void LineStyleDialog::on_distanceText_editingFinished()
    {
    EditingFinishedHelper("distance",*m_ui->distanceText);
    }

void LineStyleDialog::EditingFinishedHelper(const char* aKey,QLineEdit& aLineEdit)
    {
    QString cur_value(m_style_item.Attribute(aKey));
    if (aLineEdit.text() != cur_value)
        {
        m_style_item.SetAttribute(aKey,aLineEdit.text().toStdString());
        DrawLineSamples();
        }
    }

void LineStyleDialog::on_lineColorButton_clicked()
    {
    ColorButtonHelper("fill",*m_ui->lineColorText);
    }

void LineStyleDialog::on_borderColorButton_clicked()
    {
    ColorButtonHelper("border",*m_ui->borderColorText);
    }

void LineStyleDialog::on_centerLineColorButton_clicked()
    {
    ColorButtonHelper("centerLine",*m_ui->centerLineColorText);
    }

void LineStyleDialog::ColorButtonHelper(const char* aKey,QLineEdit& aLineEdit)
    {
    if (PickColor(*this,aLineEdit))
        {
        m_style_item.SetAttribute(aKey,aLineEdit.text().toStdString());
        DrawLineSamples();
        }
    }
