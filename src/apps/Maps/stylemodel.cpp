#include "stylemodel.h"
#include "styleitem.h"
#include "util.h"
#include <QPixmap>

CStyleModel::CStyleModel(QWidget& aParent, CartoType::CFramework& aFramework,const std::string& aStyleSheet,CartoType::CLegend& aLegend):
    QAbstractItemModel(&aParent),
    m_parent(aParent),
    m_framework(aFramework),
    m_legend(aLegend),
    m_style_sheet_text(aStyleSheet),
    m_root(nullptr)
    {
    if (!ParseStyleSheet())
        return;
    PopulateTreeNode(m_xml_document.first_node(),nullptr);
    if (m_root)
        m_root->CreateImages();
    }

CStyleModel::~CStyleModel()
    {
    delete m_root;
    }

std::string CStyleModel::EditedStyleSheetText() const
    {
    if (m_root)
        return m_root->Xml(true);
    return "";
    }

std::string CStyleModel::OriginalStyleSheetText() const
    {
    return m_style_sheet_text;
    }

bool CStyleModel::ParseStyleSheet()
    {
    try
        {
        m_style_sheet_text_copy = m_style_sheet_text;
        m_xml_document.parse<0>(&m_style_sheet_text_copy[0]);
        }
    catch (rapidxml::parse_error e)
        {
        m_error_message = e.what();
        const char* w = e.where<char>();
        m_error_location.assign(w,std::min(size_t(32),strlen(w)));
        return false;
        }

    // Check that the top-level object is a CartoTypeStyleSheet.
    rapidxml::xml_node<>* top_node = m_xml_document.first_node();
    if (top_node == nullptr || strcmp(top_node->name(),"CartoTypeStyleSheet"))
        {
        m_error_message = "not a CartoType style sheet";
        return false;
        }

    return true;
    }

QVariant CStyleModel::data(const QModelIndex& aIndex,int aRole) const
    {
    if (!aIndex.isValid())
        return QVariant();
    CStyleItem* item = static_cast<CStyleItem*>(aIndex.internalPointer());
    return item->Data(aIndex.column(),aRole);
    }

Qt::ItemFlags CStyleModel::flags(const QModelIndex& aIndex) const
    {
    if (!aIndex.isValid())
        return 0;
    return QAbstractItemModel::flags(aIndex);
    }

QVariant CStyleModel::headerData(int aSection,Qt::Orientation aOrientation,int aRole) const
    {
    if (aOrientation == Qt::Horizontal && aRole == Qt::DisplayRole)
        return m_root->Data(aSection,aRole);
    return QVariant();
    }

QModelIndex CStyleModel::index(int aRow,int aColumn,const QModelIndex& aParent) const
    {
    if (!hasIndex(aRow,aColumn,aParent))
        return QModelIndex();

    CStyleItem* parent_item = nullptr;
    if (!aParent.isValid())
        parent_item = m_root;
    else
        parent_item = static_cast<CStyleItem*>(aParent.internalPointer());

    CStyleItem* child_item = parent_item->Child(aRow);
    if (child_item)
        return createIndex(aRow,aColumn,child_item);
    else
        return QModelIndex();
    }

QModelIndex CStyleModel::parent(const QModelIndex& aIndex) const
    {
    if (!aIndex.isValid())
        return QModelIndex();

    CStyleItem* child_item = static_cast<CStyleItem*>(aIndex.internalPointer());
    CStyleItem* parent_item = child_item->Parent();
    if (parent_item == m_root)
        return QModelIndex();

    return createIndex(parent_item->Row(),0,parent_item);
    }

int CStyleModel::rowCount(const QModelIndex& aParent) const
    {
    if (aParent.column() > 0)
        return 0;

    CStyleItem* parent_item = nullptr;
    if (!aParent.isValid())
        parent_item = m_root;
    else
        parent_item = static_cast<CStyleItem*>(aParent.internalPointer());

    return parent_item->ChildCount();
    }

int CStyleModel::columnCount(const QModelIndex& aParent) const
    {
    if (aParent.isValid())
        return static_cast<CStyleItem*>(aParent.internalPointer())->ColumnCount();
    else
        return m_root->ColumnCount();
    }

void CStyleModel::PopulateTreeNode(rapidxml::xml_node<>* aXmlNode,CStyleItem* aParent)
    {
    assert(aXmlNode);
    CStyleItem* item = new CStyleItem(m_parent,m_framework,m_legend,m_style_sheet_text,aXmlNode,aParent);
    if (!aParent)
        {
        assert(m_root == nullptr);
        m_root = item;
        }

    // Make a recursive call to add the child items, unless this is an SVG node, in which case the content is added as a string.
    if (strcmp(aXmlNode->name(),"svg"))
        for (rapidxml::xml_node<>* node = aXmlNode->first_node(); node; node = node->next_sibling())
            PopulateTreeNode(node,item);
    }
