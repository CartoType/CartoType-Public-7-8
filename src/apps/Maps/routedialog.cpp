#include "routedialog.h"
#include "ui_routedialog.h"
#include "util.h"

RouteDialog::RouteDialog(QWidget& aParent, CartoType::CFramework& aFramework, const std::string& aPushPinColor, uint16_t aPushPinGlyph):
    QDialog(&aParent),
    m_ui(new Ui::RouteDialog),
    m_framework(aFramework),
    m_pushpin_color(aPushPinColor),
    m_pushpin_glyph(aPushPinGlyph)
    {
    m_ui->setupUi(this);
    PopulateGlyphList();
    }

void RouteDialog::PopulateGlyphList()
    {
    static constexpr int font_size = 32;
    static constexpr int bitmap_size = font_size + 4;

    m_ui->pushPinGlyph->clear();
    m_ui->pushPinGlyph->setIconSize({ bitmap_size, bitmap_size});
    CartoType::TFontSpec fs;
    fs.SetName("Mapkey Icons");
    fs.SetToSize(font_size);
    CartoType::TFont font { m_framework.Font(fs) };
    auto gc { m_framework.CreateGraphicsContext(bitmap_size,bitmap_size) };
    CartoType::TTextParam param;
    param.iBaseline = CartoType::TTextBaseline::Middle;
    CartoType::TTextMetrics metrics;

    CartoType::TColor c(m_pushpin_color);
    if (c.IsNull())
        c = CartoType::KBlack;
    gc->SetColor(c);
    for (uint16_t code = 0xe002; code <= 0xe0b1; code++)
        {
        QString caption;
        gc->Clear();
        int type = code;
        if (code == 0xe002)
            {
            type = 0;           // first line is blank, indicating the normal, non-text, pushpin glyph
            caption = "(no custom glyph)";
            }
        else
            {
            CartoType::TText text(&code,1);
            font.DrawText(gc.get(),text,CartoType::TPoint(0,bitmap_size / 2),param,metrics);
            }
        QImage image(bitmap_size,bitmap_size,QImage::Format_ARGB32_Premultiplied);
        CopyBitmapToImage(*gc->Bitmap(),image);
        QIcon icon(QPixmap::fromImage(image));
        QListWidgetItem* item { new QListWidgetItem(icon,caption,nullptr,type) };
        m_ui->pushPinGlyph->addItem(item);
        if (type == m_pushpin_glyph)
            m_ui->pushPinGlyph->setCurrentItem(item);
        }
    }

RouteDialog::~RouteDialog()
    {
    delete m_ui;
    }

void RouteDialog::on_setStartOfRoute_clicked()
    {
    EnablePushPinEditing();
    }

void RouteDialog::on_setEndOfRoute_clicked()
    {
    EnablePushPinEditing();
    }

void RouteDialog::on_addPushPin_clicked()
    {
    EnablePushPinEditing();
    }

void RouteDialog::on_cutPushPin_clicked()
    {
    EnablePushPinEditing();
    }

void RouteDialog::EnablePushPinEditing()
    {
    bool enable = m_ui->addPushPin->isChecked() || m_ui->cutPushPin->isChecked();
    m_ui->pushPinNameLabel->setEnabled(enable);
    m_ui->pushPinName->setEnabled(enable);
    m_ui->pushPinDescLabel->setEnabled(enable);
    m_ui->pushPinDesc->setEnabled(enable);
    m_ui->pushPinColor->setEnabled(enable);
    m_ui->pushPinGlyph->setEnabled(enable);
    m_ui->useAsWayPoint->setEnabled(enable);
    }

void RouteDialog::on_pushPinColor_clicked()
    {
    std::string prev_color { m_pushpin_color };
    std::string c = PickColor(*this,m_pushpin_color,false);
    if (!c.empty())
        {
        m_pushpin_color = c;
        if (c != prev_color)
            PopulateGlyphList();
        }
    }

void RouteDialog::on_pushPinGlyph_currentItemChanged(QListWidgetItem* aCurrent,QListWidgetItem* /*aPrevious*/)
    {
    if (aCurrent)
        m_pushpin_glyph = aCurrent->type();
    }
