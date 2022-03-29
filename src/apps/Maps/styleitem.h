#ifndef STYLEITEM_H__
#define STYLEITEM_H__

#include <QList>
#include <QVariant>
#include <QImage>
#include <QItemDelegate>
#include "rapidxml.hpp"
#include <memory>
#include <cartotype_framework.h>

class CStyleItem
    {
    public:
    CStyleItem(QWidget& aWidget,CartoType::CFramework& aFramework,CartoType::CLegend& aLegend,
               const std::string& aStyleSheetText,const rapidxml::xml_node<>* aXmlNode,CStyleItem* aParent = nullptr);
    ~CStyleItem();

    void CreateImages();
    CStyleItem* Child(int aRow);
    int ChildCount() const;
    int ColumnCount() const;
    QVariant Data(int aColumn,int aRole) const;
    int Row() const;
    CStyleItem* Parent();
    const char* Attribute(const char* aKey) const;
    void SetAttribute(const char* aKey,const std::string& aValue);
    const std::string& ElementName() const { return m_element_name; }
    std::string Xml(bool aIndent = false);
    const std::map<std::string,std::string>& Attributes() const { return m_attributes; }
    void SetXml(const rapidxml::xml_node<>* aXmlNode);
    void SetAttributes(std::map<std::string,std::string>&& aAttributes) { m_attributes = aAttributes; }
    void CreateImage();

    private:
    void AppendChild(CStyleItem* aChild);
    void AppendXmlContext(std::string& aPrecedingContext,std::string& aFollowingContext,const CStyleItem* aChildNode,bool aAllowEverything);
    std::string XmlHelper(bool aIndent,int aIndentLevel);

    QWidget& m_widget; // the widget owning the tree model: provides a font used to measure the caption size
    CStyleItem* m_parent;
    CartoType::CFramework& m_framework;
    CartoType::CLegend& m_legend;
    const std::string& m_style_sheet_text;
    QList<CStyleItem*> m_child_list;
    QString m_caption;
    QSize m_caption_size;
    std::string m_element_name;
    std::map<std::string,std::string> m_attributes;
    std::unique_ptr<QPixmap> m_image;
    std::string m_svg_text;
    };

class CStyleItemDelegate: public QItemDelegate
    {
    public:
    CStyleItemDelegate(QObject* aParent = nullptr);
    void paint(QPainter* aPainter,
               const QStyleOptionViewItem& aOption,
               const QModelIndex& aIndex) const override;
    QSize sizeHint(const QStyleOptionViewItem& aOption,
                   const QModelIndex& aIndex) const override;
    };

#endif // STYLEITEM_H__
