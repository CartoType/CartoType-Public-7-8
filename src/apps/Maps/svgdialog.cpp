#include "svgdialog.h"
#include "ui_svgdialog.h"
#include "util.h"

SvgDialog::SvgDialog(QWidget& aParent,CartoType::CFramework& aFramework,CStyleItem& aStyleItem):
    QDialog(&aParent),
    m_framework(aFramework),
    m_ui(new Ui::SvgDialog),
    m_style_item(aStyleItem)
    {
    m_ui->setupUi(this);
    m_ui->horizontalLayout->setStretchFactor(m_ui->svgText,1);
    m_ui->horizontalLayout->setStretchFactor(m_ui->svgImage,1);
    m_ui->svgText->setText(m_style_item.Xml(true).c_str());

    /*
    Prevent the pick-colour button from getting focus so that the text edit
    object keeps focus, so the user can carry on editing after picking a colour.
    */
    m_ui->pickColorButton->setFocusPolicy(Qt::NoFocus);

    DrawSvgImage();
    }

SvgDialog::~SvgDialog()
    {
    delete m_ui;
    }

void SvgDialog::DrawSvgImage()
    {
    std::string svg { (const char*)(m_ui->svgText->toPlainText().toUtf8()) };
    CartoType::TMemoryInputStream input((const uint8_t*)svg.data(),svg.length());

    // Tell the layout to lay itself out so we can get the actual width and height of the image.
    m_ui->horizontalLayout->update();
    m_ui->horizontalLayout->activate();

    int w = m_ui->svgImage->width();
    int h = m_ui->svgImage->height();
    int size = std::min(w,h);

    CartoType::TFileLocation error_location;
    std::unique_ptr<CartoType::CBitmap> bitmap = m_framework.CreateBitmapFromSvg(input,error_location,size);

    if (bitmap && bitmap->Height() > size)
        {
        double f = double(size) / double(bitmap->Height());
        input.Seek(0);
        bitmap = m_framework.CreateBitmapFromSvg(input,error_location,int(size * f));
        }

    QList<QTextEdit::ExtraSelection> selections;
    if (!bitmap)
        {
        QTextEdit::ExtraSelection sel;
        sel.cursor = m_ui->svgText->textCursor(); // we have to get the cursor before setting it
        int sel_start = (int)error_location.iByteIndex;
        int sel_end = sel_start;
        sel_start = std::max(0,sel_start - 2);
        sel_end = std::min(m_ui->svgText->document()->characterCount(),sel_end + 2);

        sel.cursor.setPosition(sel_start,QTextCursor::MoveAnchor);
        sel.cursor.setPosition(sel_end,QTextCursor::KeepAnchor);
        sel.format.setFontUnderline(true);
        sel.format.setUnderlineColor(QColor(255,0,0));
        sel.format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
        sel.format.setForeground(QColor(255,0,0));
        selections.append(sel);

        char buffer[64];
        sprintf(buffer,"error at line %d, column %d",(int)error_location.iLineNumber,(int)error_location.iColumnNumber);
        m_ui->svgImage->setText(buffer);
        }
    m_ui->svgText->setExtraSelections(selections);

    if (bitmap)
        {
        QImage image(bitmap->Width(),bitmap->Height(),QImage::Format_ARGB32_Premultiplied);
        CopyBitmapToImage(*bitmap,image);
        m_ui->svgImage->setPixmap(QPixmap::fromImage(std::move(image)));
        }
    }

void SvgDialog::on_updateButton_clicked()
    {
    DrawSvgImage();
    }

void SvgDialog::on_pickColorButton_clicked()
    {
    QTextCursor cursor { m_ui->svgText->textCursor() };
    QString sel = cursor.selectedText();
    int sel_start = cursor.selectionStart();
    std::string color = PickColor(*this,sel.toStdString(),false);
    if (color.length())
        {
        cursor.insertText(QString(color.c_str()));
        cursor.setPosition(sel_start);
        cursor.setPosition(sel_start + (int)color.length(),QTextCursor::KeepAnchor);
        m_ui->svgText->setTextCursor(cursor);
        DrawSvgImage();
        }
    }

std::string SvgDialog::SvgText()
    {
    return m_ui->svgText->toPlainText().toStdString();
    }

void SvgDialog::on_svgText_selectionChanged()
    {
    QTextCursor cursor { m_ui->svgText->textCursor() };
    QString sel = cursor.selectedText();
    CartoType::CString s(sel.toUtf8());
    CartoType::TColor color;
    size_t length = color.Set(s);
    m_ui->pickColorButton->setEnabled(length && length == s.Length());
    }

void SvgDialog::on_redoButton_clicked()
    {
    m_ui->svgText->redo();
    DrawSvgImage();
    }

void SvgDialog::on_undoButton_clicked()
    {
    m_ui->svgText->undo();
    DrawSvgImage();
    }
