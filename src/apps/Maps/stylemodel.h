#ifndef STYLEMODEL_H__
#define STYLEMODEL_H__

#include <QAbstractItemModel>
#include <cartotype_framework.h>
#include <rapidxml.hpp>

class CStyleItem;

class CStyleModel: public QAbstractItemModel
    {
    Q_OBJECT

    public:
    explicit CStyleModel(QWidget& aParent,CartoType::CFramework& aFramework,const std::string& aStyleSheet,CartoType::CLegend& aLegend);
    ~CStyleModel();

    std::string EditedStyleSheetText() const;
    std::string OriginalStyleSheetText() const;
    bool HaveError() const { return !m_error_message.empty(); }
    const std::string& ErrorMessage() const { return m_error_message; }
    const std::string& ErrorLocation() const { return m_error_location; }

    QVariant data(const QModelIndex& aIndex,int aRole) const override;
    Qt::ItemFlags flags(const QModelIndex& aIndex) const override;
    QVariant headerData(int aSection,Qt::Orientation aOrientation,
                        int aRole = Qt::DisplayRole) const override;
    QModelIndex index(int aRow,int aColumn,
                      const QModelIndex& aParent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& aIndex) const override;
    int rowCount(const QModelIndex& aParent = QModelIndex()) const override;
    int columnCount(const QModelIndex& aParent = QModelIndex()) const override;

    private:
    bool ParseStyleSheet();
    void PopulateTreeNode(rapidxml::xml_node<>* aXmlNode,CStyleItem* aParent);

    QWidget& m_parent;
    CartoType::CFramework& m_framework;
    CartoType::CLegend& m_legend;
    const std::string m_style_sheet_text;
    std::string m_style_sheet_text_copy; // a copy of the style sheet text for use by rapidxml, which modifies the text it parses
    rapidxml::xml_document<> m_xml_document;
    CStyleItem* m_root;
    std::string m_error_message;
    std::string m_error_location;
    };

#endif // STYLEMODEL_H__
