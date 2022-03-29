#include "styleitem.h"
#include "util.h"
#include <QPainter>
#include <rapidxml_print.hpp>

CStyleItem::CStyleItem(QWidget& aWidget,CartoType::CFramework& aFramework,CartoType::CLegend& aLegend,
                       const std::string& aStyleSheetText, const rapidxml::xml_node<>* aXmlNode, CStyleItem* aParent):
    m_widget(aWidget),
    m_parent(aParent),
    m_framework(aFramework),
    m_legend(aLegend),
    m_style_sheet_text(aStyleSheetText)
    {
    // Attach this item to its parent.
    if (aParent)
        aParent->AppendChild(this);
    SetXml(aXmlNode);
    }

CStyleItem::~CStyleItem()
    {
    qDeleteAll(m_child_list);
    }

void CStyleItem::AppendChild(CStyleItem* aChild)
    {
    m_child_list.append(aChild);
    }

CStyleItem* CStyleItem::Child(int aRow)
    {
    return m_child_list.value(aRow);
    }

int CStyleItem::ChildCount() const
    {
    return m_child_list.count();
    }

int CStyleItem::ColumnCount() const
    {
    return 2;
    }

QVariant CStyleItem::Data(int aColumn,int aRole) const
    {
    if (aRole == Qt::DisplayRole)
        {
        if (aColumn == 0)
            return m_caption;
        }

    else if (aRole == Qt::DecorationRole)
        {
        if (aColumn == 1)
            {
            if (m_image)
                return *m_image;
            }
        }

    else if (aRole == Qt::SizeHintRole)
        {
        if (aColumn == 0)
            return m_caption_size;

        if (aColumn == 1)
            {
            if (m_image)
                return m_image->size() + QSize(4,4);
            }
        }

    return QVariant();
    }

int CStyleItem::Row() const
    {
    if (m_parent)
        return m_parent->m_child_list.indexOf(const_cast<CStyleItem*>(this));
    return 0;
    }

CStyleItem* CStyleItem::Parent()
    {
    return m_parent;
    }

const char* CStyleItem::Attribute(const char* aKey) const
    {
    if (!aKey || !aKey[0])
        return "";
    auto iter = m_attributes.find(aKey);
    if (iter != m_attributes.end())
        return iter->second.c_str();
    return "";
    }

void CStyleItem::SetAttribute(const char* aKey,const std::string& aValue)
    {
    if (aValue.empty())
        m_attributes.erase(aKey);
    else
        m_attributes[aKey] = aValue;
    }

void CStyleItem::AppendXmlContext(std::string& aPrecedingContext,std::string& aFollowingContext,
                                  const CStyleItem* aChildNode,bool aAllowEverything)
    {
    std::string* dest = &aPrecedingContext;
    for (const auto p : m_child_list)
        {
        if (p == aChildNode)
            dest = &aFollowingContext;
        else
            {
            if (aAllowEverything ||
                p->m_element_name == "line" ||
                p->m_element_name == "shape" ||
                p->m_element_name == "icon" ||
                p->m_element_name == "label")
                *dest += p->Xml();
            }
        }
    }

void AppendContainedLayers(QString& aDest,const rapidxml::xml_node<>* aXmlNode,bool aNested,bool& aHaveLayers)
    {
    for (rapidxml::xml_node<>* node = aXmlNode->first_node(); node; node = node->next_sibling())
        {
        const char* element_name = node->name();
        if (!strcmp(element_name,"layer"))
            {
            rapidxml::xml_attribute<char>* a = node->first_attribute("name");
            if (a && a->value())
                {
                if (!aHaveLayers)
                    {
                    aDest.append(" (");
                    aHaveLayers = true;
                    }
                else
                    aDest.append(", ");
                aDest.append(a->value());
                }
            }
        else if (!strcmp(element_name,"scale") ||
                 !strcmp(element_name,"zoom") ||
                 !strcmp(element_name,"if"))
            AppendContainedLayers(aDest,node,true,aHaveLayers);
        }
    if (!aNested && aHaveLayers)
        aDest.append(")");
    }

void CStyleItem::SetXml(const rapidxml::xml_node<>* aXmlNode)
    {
    // Store the element name.
    m_element_name = (const char*)aXmlNode->name();

    // Store the attributes.
    for (rapidxml::xml_attribute<char>* a = aXmlNode->first_attribute(); a; a = a->next_attribute())
        m_attributes[a->name()] = a->value();

    QString element(m_element_name.c_str());
    QString name;
    if (element == "layer" || element == "CartoTypeStyleSheet")
        {
        rapidxml::xml_attribute<char>* a = aXmlNode->first_attribute("name");
        if (a)
            name = a->value();
        }
    else if (element == "icon" || element == "macro" || element == "def" || element == "rampGradient")
        {
        rapidxml::xml_attribute<char>* a = aXmlNode->first_attribute("id");
        if (a)
            name = a->value();
        if (name.length())
            element += " definition";
        else
            {
            a = aXmlNode->first_attribute("ref");
            if (a)
                name = a->value();
            if (name.length())
                element += " reference";
            }
        }
    else if (element == "scale" || element == "zoom")
        {
        rapidxml::xml_attribute<char>* a = aXmlNode->first_attribute("min");
        rapidxml::xml_attribute<char>* b = aXmlNode->first_attribute("max");
        if (a || b)
            {
            if (a)
                name = a->value();
            else
                name = "0";
            name += "...";
            if (b)
                name += b->value();
            }
        }
    else if (element == "condition" || element == "if")
        {
        rapidxml::xml_attribute<char>* a = aXmlNode->first_attribute("exp");
        if (a)
            {
            name += " ";
            name += a->value();
            }
        }

    m_caption = element;
    if (name.length())
        {
        m_caption += ": ";
        m_caption += name;
        }

    // Add summary of contained layers if any.
    if (strcmp(aXmlNode->name(),"CartoTypeStyleSheet"))
        {
        bool have_layers = false;
        AppendContainedLayers(m_caption,aXmlNode,false,have_layers);
        }

    // Work out the size of the caption in pixels.
    m_caption_size = m_widget.fontMetrics().size(0,m_caption);

    // Store the whole SVG text if this is an SVG node.
    m_svg_text.clear();
    if (!strcmp(aXmlNode->name(),"svg"))
        rapidxml::print(std::back_inserter(m_svg_text),*aXmlNode,0);
    }

/**
Creates an image for the node, if appropriate.
The image is the map object or icon drawn by the XML.
*/
void CStyleItem::CreateImage()
    {
    CartoType::TMapObjectType map_object_type = CartoType::TMapObjectType::None;
    if (m_element_name == "svg")
        {
        CartoType::TMemoryInputStream input_stream((const uint8_t*)m_svg_text.data(),m_svg_text.length());
        CartoType::TFileLocation error_location;
        std::unique_ptr<CartoType::CBitmap> bitmap = m_framework.CreateBitmapFromSvg(input_stream,error_location,64);
        if (bitmap && bitmap->Height() > 64)
            {
            input_stream.Seek(0);
            bitmap = m_framework.CreateBitmapFromSvg(input_stream,error_location,int(64.0 * 64.0 / bitmap->Height()));
            }
        if (bitmap)
            {
            QImage image(bitmap->Width(),bitmap->Height(),QImage::Format_ARGB32_Premultiplied);
            CopyBitmapToImage(*bitmap,image);
            m_image.reset(new QPixmap(QPixmap::fromImage(std::move(image))));
            }
        return;
        }
    else if (m_element_name == "shape")
        map_object_type = CartoType::TMapObjectType::Polygon;
    else if (m_element_name == "line")
        map_object_type = CartoType::TMapObjectType::Line;
    else
        return;

    int scale = 10000;

    // Find the <layer> if any.
    CStyleItem* parent_layer = nullptr;
    std::vector<CStyleItem*> parent_array;
    bool layer_found = false;
    for (parent_layer = this; parent_layer; parent_layer = parent_layer->m_parent)
        {
        parent_array.push_back(parent_layer);
        if (parent_layer->m_element_name == "layer")
            {
            layer_found = true;
            break;
            }
        }

    // If there's no <layer> this must be a definition, so use the definition on its own.
    if (!layer_found)
        parent_array.erase(parent_array.begin() + 1,parent_array.end());
    std::reverse(parent_array.begin(),parent_array.end());

    /*
    Create a new layer called _temp and insert any
    relevant contextual elements (line, shape, icon, label)
    found in enclosing elements before the child element.
    Determine the map object type if it's not yet known.
    */
    std::string temp_layer("<layer name='_temp'>");
    std::string preceding_context;
    std::string following_context;
    for (size_t i = 1; i < parent_array.size(); i++)
        parent_array[i - 1]->AppendXmlContext(preceding_context,following_context,
                                              parent_array[i],i == parent_array.size() - 1);

    // Experiment: if this is an icon, change its size to something discernible.
    std::string saved_width_attrib;
    if (m_element_name == "icon")
        {
        saved_width_attrib = Attribute("width");
        SetAttribute("width","6mm");
        }

    std::string element = Xml();

    if (m_element_name == "icon")
        SetAttribute("width",saved_width_attrib);

    temp_layer += preceding_context;
    temp_layer += element;
    temp_layer += following_context;
    temp_layer += "</layer>";

    /*
    Copy the entire style sheet and insert the temporary layer just before the end.
    This is done rather than adding an extra style sheet because extra style sheets
    cannot refer to macros defined in the main style sheet.
    */
    std::string style_sheet { m_style_sheet_text };
    assert(style_sheet.length() > strlen("</CartoTypeStyleSheet>"));
    size_t n = style_sheet.rfind("</CartoTypeStyleSheet>");
    style_sheet.insert(n,temp_layer);

    m_legend.Clear();
    m_legend.SetMainStyleSheet((const uint8_t*)style_sheet.data(),style_sheet.length());
    m_legend.SetMinLineHeight(1,"cm");
    m_legend.SetBackgroundColor(CartoType::KTransparentBlack);
    m_legend.SetBorder(CartoType::KTransparentBlack,0,0,"px");

    CartoType::CString string_attrib("Sample");
    string_attrib.SetAttribute("ref","123");

    m_legend.AddMapObjectLine(map_object_type,"_temp",nullptr,0,string_attrib,"");
    CartoType::TResult error;
    std::unique_ptr<CartoType::CBitmap> bitmap { m_legend.CreateLegend(error,2,"cm",scale,scale) };
    if (!error)
        {
        CartoType::TRect bounds;
        CartoType::CBitmap trimmed_bitmap { bitmap->Trim(bounds) };
        if (trimmed_bitmap.Width())
            {
            QImage image(trimmed_bitmap.Width(),trimmed_bitmap.Height(),QImage::Format_ARGB32_Premultiplied);
            CopyBitmapToImage(trimmed_bitmap,image);
            m_image.reset(new QPixmap(QPixmap::fromImage(std::move(image))));
            }
        }
    }

std::string CStyleItem::Xml(bool aIndent)
    {
    return XmlHelper(aIndent,0);
    }

std::string CStyleItem::XmlHelper(bool aIndent,int aIndentLevel)
    {
    if (m_svg_text.size())
        return m_svg_text;

    CartoType::CMemoryOutputStream s;
    if (aIndent && aIndentLevel)
        {
        for (int i = 0; i < aIndentLevel; i++)
            s.WriteString("    ");
        }
    s.WriteString("<");
    s.WriteString(m_element_name.c_str());
    for (const auto& p : m_attributes)
        {
        s.WriteString(" ");
        s.WriteString(p.first.c_str());
        s.WriteString("='");
        s.WriteXmlText(CartoType::CString(p.second));
        s.WriteString("'");
        }
    std::string xml((const char*)s.Data(),s.Length());
    if (m_child_list.count())
        {
        xml += ">\n";
        for (const auto c : m_child_list)
            xml += c->XmlHelper(aIndent,aIndentLevel + 1);
        if (aIndent && aIndentLevel)
            {
            for (int i = 0; i < aIndentLevel; i++)
                xml += "    ";
            }
        xml += "</";
        xml += m_element_name;
        xml += ">";
        }
    else
        xml += "/>";
    if (aIndent)
        xml += "\n";
    return xml;
    }

/** Create images for this style item and its children. */
void CStyleItem::CreateImages()
    {
    CreateImage();
    for (auto p : m_child_list)
        p->CreateImages();
    }

CStyleItemDelegate::CStyleItemDelegate(QObject* aParent):
    QItemDelegate(aParent)
    {
    }

void CStyleItemDelegate::paint(QPainter* aPainter,
                               const QStyleOptionViewItem& aOption,
                               const QModelIndex& aIndex) const
    {
    QStyleOptionViewItem option { aOption };
    option.font.setPointSize(9);

    QString text = aIndex.model()->data(aIndex,Qt::DisplayRole).toString();
    QPixmap image = qvariant_cast<QPixmap>(aIndex.model()->data(aIndex,Qt::DecorationRole));
    drawBackground(aPainter,option,aIndex);
    drawDisplay(aPainter,option,option.rect,text);
    drawDecoration(aPainter,option,option.rect,image);
    }

QSize CStyleItemDelegate::sizeHint(const QStyleOptionViewItem& /*aOption*/,
                                   const QModelIndex& aIndex) const
    {
    return aIndex.model()->data(aIndex,Qt::SizeHintRole).toSize();
    }
