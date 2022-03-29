#include "attributedialog.h"
#include "ui_attributedialog.h"
#include "util.h"
#include <cartotype_framework.h>

AttributeDialog::AttributeDialog(QWidget& aParent,const std::string& aElementName,const std::map<std::string,std::string>& aAttributes):
    QDialog(&aParent),
    m_ui(new Ui::AttributeDialog)
    {
    m_ui->setupUi(this);
    m_ui->elementName->setText(aElementName.c_str());

    m_form_row.push_back({m_ui->name01,m_ui->text01});
    m_form_row.push_back({m_ui->name02,m_ui->text02});
    m_form_row.push_back({m_ui->name03,m_ui->text03});
    m_form_row.push_back({m_ui->name04,m_ui->text04});
    m_form_row.push_back({m_ui->name05,m_ui->text05});
    m_form_row.push_back({m_ui->name06,m_ui->text06});
    m_form_row.push_back({m_ui->name07,m_ui->text07});
    m_form_row.push_back({m_ui->name08,m_ui->text08});
    m_form_row.push_back({m_ui->name09,m_ui->text09});
    m_form_row.push_back({m_ui->name10,m_ui->text10});
    m_form_row.push_back({m_ui->name11,m_ui->text11});
    m_form_row.push_back({m_ui->name12,m_ui->text12});
    m_form_row.push_back({m_ui->name13,m_ui->text13});
    m_form_row.push_back({m_ui->name14,m_ui->text14});
    m_form_row.push_back({m_ui->name15,m_ui->text15});
    m_form_row.push_back({m_ui->name16,m_ui->text16});

    size_t form_row_index = 0;
    for (const auto& p : aAttributes)
        {
        if (form_row_index >= m_form_row.size())
            break;
        m_form_row[form_row_index].first->setText(p.first.c_str());
        m_form_row[form_row_index].second->setText(p.second.c_str());
        form_row_index++;
        }

    /*
    Prevent the pick-colour button from getting focus,
    so that the line edit object doesn't lose its selection when the button is clicked,
    and also so that the line edit object keeps focus, so the user can carry on editing
    after picking a colour.
    */
    m_ui->pickColorButton->setFocusPolicy(Qt::NoFocus);
    }

AttributeDialog::~AttributeDialog()
    {
    delete m_ui;
    }

std::map<std::string,std::string> AttributeDialog::Attributes()
    {
    std::map<std::string,std::string> a;
    for (const auto& p : m_form_row)
        {
        const QLineEdit* name = p.first;
        const QLineEdit* text = p.second;
        if (name->text().length() && text->text().length())
            {
            std::string n = name->text().toStdString();
            std::string t = text->text().toStdString();
            a[n] = t;
            }
        }
    return a;
    }

void AttributeDialog::OnSelectionChanged(QLineEdit* aLineEdit)
    {
    QString sel = aLineEdit->selectedText();
    CartoType::CString s(sel.toUtf8());
    CartoType::TColor color;
    size_t length = color.Set(s);
    if (length && length == s.Length())
        {
        m_ui->pickColorButton->setEnabled(true);
        m_color_line_edit = aLineEdit;
        }
    else
        {
        m_ui->pickColorButton->setEnabled(false);
        m_color_line_edit = nullptr;
        }
    }

void AttributeDialog::on_text01_selectionChanged() { OnSelectionChanged(m_ui->text01); }
void AttributeDialog::on_text02_selectionChanged() { OnSelectionChanged(m_ui->text02); }
void AttributeDialog::on_text03_selectionChanged() { OnSelectionChanged(m_ui->text03); }
void AttributeDialog::on_text04_selectionChanged() { OnSelectionChanged(m_ui->text04); }
void AttributeDialog::on_text05_selectionChanged() { OnSelectionChanged(m_ui->text05); }
void AttributeDialog::on_text06_selectionChanged() { OnSelectionChanged(m_ui->text06); }
void AttributeDialog::on_text07_selectionChanged() { OnSelectionChanged(m_ui->text07); }
void AttributeDialog::on_text08_selectionChanged() { OnSelectionChanged(m_ui->text08); }
void AttributeDialog::on_text09_selectionChanged() { OnSelectionChanged(m_ui->text09); }
void AttributeDialog::on_text10_selectionChanged() { OnSelectionChanged(m_ui->text10); }
void AttributeDialog::on_text11_selectionChanged() { OnSelectionChanged(m_ui->text11); }
void AttributeDialog::on_text12_selectionChanged() { OnSelectionChanged(m_ui->text12); }
void AttributeDialog::on_text13_selectionChanged() { OnSelectionChanged(m_ui->text13); }
void AttributeDialog::on_text14_selectionChanged() { OnSelectionChanged(m_ui->text14); }
void AttributeDialog::on_text15_selectionChanged() { OnSelectionChanged(m_ui->text15); }
void AttributeDialog::on_text16_selectionChanged() { OnSelectionChanged(m_ui->text16); }

void AttributeDialog::on_pickColorButton_clicked()
    {
    if (m_color_line_edit == nullptr)
        return;
    int sel_start = m_color_line_edit->selectionStart();
    std::string color = PickColor(*this,m_color_line_edit->selectedText().toStdString());
    if (color.length())
        {
        QLineEdit* edit = m_color_line_edit; // keep a copy of m_color_line_edit because inserting text will cause OnSelectionChanged to set it to null
        edit->insert(QString(color.c_str()));
        edit->setSelection(sel_start,(int)color.length());
        }
    }
