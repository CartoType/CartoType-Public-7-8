#include "styledialog.h"
#include "ui_styledialog.h"
#include "styleitem.h"
#include "linestyledialog.h"
#include "polygonstyledialog.h"
#include "svgdialog.h"
#include "attributedialog.h"
#include "util.h"
#include <QPushButton>

StyleDialog::StyleDialog(QWidget *aParent, CartoType::CFramework& aFramework,const std::string& aStyleSheet):
    QDialog(aParent),
    m_ui(new Ui::StyleDialog),
    m_framework(aFramework),
    m_legend(aFramework),
    m_style_model(*this,aFramework,aStyleSheet,m_legend)
    {
    if (m_style_model.HaveError())
        return;

    m_ui->setupUi(this);
    m_ui->styleTree->setModel(&m_style_model);
    m_ui->styleTree->setItemDelegate(&m_style_item_delegate);
    m_ui->styleTree->resizeColumnToContents(0);

    CartoType::CString name = "edit the custom style sheet and apply it to ";
    name += aFramework.DataSetName();
    QString qname;
    SetString(qname,name);
    setWindowTitle(qname);
    }

StyleDialog::~StyleDialog()
    {
    delete m_ui;
    }

void StyleDialog::on_buttonBox_accepted()
    {
    //+ use the edited style sheet
    accept();
    }

void StyleDialog::on_buttonBox_rejected()
    {
    reject();
    }

void StyleDialog::on_styleTree_doubleClicked(const QModelIndex& aIndex)
    {
    CStyleItem* item = static_cast<CStyleItem*>(aIndex.internalPointer());

    if (item->ElementName() == "line")
        EditLine(*item);
    else if (item->ElementName() == "shape")
        EditPolygon(*item);
    else if (item->ElementName() == "svg")
        EditSvg(*item);
    else
        EditAttributes(*item);
    }

void StyleDialog::EditLine(CStyleItem& aStyleItem)
    {
    LineStyleDialog dialog(*this,aStyleItem,m_legend);
    std::map<std::string,std::string> saved_attributes(aStyleItem.Attributes());
    if (dialog.exec() == QDialog::Accepted)
        aStyleItem.CreateImage();
    else
        aStyleItem.SetAttributes(std::move(saved_attributes));
    }

void StyleDialog::EditPolygon(CStyleItem& aStyleItem)
    {
    PolygonStyleDialog dialog(*this,aStyleItem,m_legend);
    std::map<std::string,std::string> saved_attributes(aStyleItem.Attributes());
    if (dialog.exec() == QDialog::Accepted)
        aStyleItem.CreateImage();
    else
        aStyleItem.SetAttributes(std::move(saved_attributes));
    }

void StyleDialog::EditSvg(CStyleItem& aStyleItem)
    {
    SvgDialog dialog(*this,m_framework,aStyleItem);
    if (dialog.exec() == QDialog::Accepted)
        {
        // Put the new SVG into aStyleItem unless it has syntax errors or is not an SVG node.
        std::string svg { dialog.SvgText() };
        rapidxml::xml_document<> document;
        try
            {
            document.parse<0>(&svg[0]);
            }
        catch (rapidxml::parse_error /*e*/)
            {
            return;
            }

        rapidxml::xml_node<char>* node = document.first_node("svg");
        if (node)
            {
            aStyleItem.SetXml(document.first_node());
            aStyleItem.CreateImage();

            // For now: parent icon has image.
            if (aStyleItem.Parent())
                aStyleItem.Parent()->CreateImage();
            }
        }
    }

void StyleDialog::EditAttributes(CStyleItem& aStyleItem)
    {
    AttributeDialog dialog(*this,aStyleItem.ElementName(),aStyleItem.Attributes());
    if (dialog.exec() == QDialog::Accepted)
        {
        aStyleItem.SetAttributes(dialog.Attributes());

        // Recreate images for the item's parent and all its children.
        if (aStyleItem.Parent())
            aStyleItem.Parent()->CreateImages();
        }
    }

std::string StyleDialog::EditedStyleSheetText() const
    {
    return m_style_model.EditedStyleSheetText();
    }

std::string StyleDialog::OriginalStyleSheetText() const
    {
    return m_style_model.OriginalStyleSheetText();
    }

void StyleDialog::on_buttonBox_clicked(QAbstractButton* aButton)
    {
    if (aButton == m_ui->buttonBox->button(QDialogButtonBox::Apply))
        emit ApplyStyleSheet(m_style_model.EditedStyleSheetText());
    }
