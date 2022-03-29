#include "util.h"
#include <rapidxml.hpp>
#include <QColorDialog>
#include <QFileDialog>

void CopyBitmapToImage(const CartoType::TBitmap& aSource,QImage& aDest)
    {
    assert(aDest.width() == aSource.Width() &&
           aDest.height() == aSource.Height() &&
           aDest.format() == QImage::Format_ARGB32_Premultiplied &&
           aSource.Type() == CartoType::TBitmapType::RGBA32);

    uint32_t* dest_row = (uint32_t*)aDest.bits();
    int dest_stride = aDest.bytesPerLine() / 4;
    const uint8_t* source_row = aSource.Data();
    int source_stride = aSource.RowBytes();
    int h = aDest.height();
    int w = aDest.width();
    for (int y = 0; y < h; y++, dest_row += dest_stride, source_row += source_stride)
        {
        uint32_t* dest_ptr = dest_row;
        const uint8_t* source_ptr = source_row;
        for (int x = 0; x < w; x++, dest_ptr++, source_ptr += 4)
            {
            // source is ABGR, dest is ARGB
            *dest_ptr = ((uint32_t)(source_ptr[0]) << 24) |
                        ((uint32_t)(source_ptr[1])) |
                        ((uint32_t)(source_ptr[2]) << 8) |
                        ((uint32_t)(source_ptr[3]) << 16);
            }
        }
    }

/**
Parses a style sheet dimension. Returns true if parsing succeeds.

A dimension has the parts size{,min{,max}} and each of the three parts
is of the form <number><unit>.

The following units are allowed for all parts.

'pt' (point: 1/72in), 'pc' (pica: 1/12in), 'cm' (centimeter), 'mm' (millimeter), 'in' (inch), 'px' (pixel),
'm' (map meter); '%' (percentage).

The first part ('size') may alternatively be of the form <y1>m@<x1>_<y2>m@<x2>,
where x1...x2 is a range of scales and y1...y2 is a range of sizes in metres.
For example, '10m@5000_25m@25000' creates a function that yields 10m at a scale of 1:5000,
25m at a scale of 1:25000, and scales between them logarithmically.
*/
bool ParseDimension(TParsedDimension& aParsedDimension,const std::string& aDimension)
    {
    aParsedDimension = TParsedDimension();

    // Get up to three dimension-unit pairs, separated by commas.
    const char* p = aDimension.data();
    const char* end = p + aDimension.length();
    for (int i = 0; i < 3; i++)
        {
        // Skip spaces; disallow an empty dimension-unit pair.
        while (p < end && *p == ' ')
            p++;
        if (p == end)
            return false;

        // Get the number; disallow a zero-length number.
        char* number_end;
        double n = strtod(p,&number_end);
        if (number_end == p)
            return false;

        // Get the unit and validate it.
        p = number_end;
        std::string unit;
        while (p < end && *p != ',')
            unit.append(1,*p++);
        if (unit != "pt" &&
            unit != "pc" &&
            unit != "cm" &&
            unit != "mm" &&
            unit != "in" &&
            unit != "px" &&
            unit != "m" &&
            unit != "%" &&
            !(i == 0 && unit.substr(0,2) != "m@"))
            return false;

        if (i == 0)
            {
            aParsedDimension.m_size = n;
            aParsedDimension.m_size_unit = unit;
            }
        else if (i == 1)
            {
            aParsedDimension.m_min_size = n;
            aParsedDimension.m_min_size_unit = unit;
            }
        else
            {
            aParsedDimension.m_max_size = n;
            aParsedDimension.m_max_size_unit = unit;
            }
        if (p == end)
            break;
        p++;
        }

    /*
    If the first unit starts with m@, parse it as a range of sizes in the form <y1>m@<x1>_<y2>m@<x2>,
    where x1...x2 is a range of scales and y1...y2 is a range of sizes in metres.
    */
    if (aParsedDimension.m_size_unit.substr(0,2) == "m@")
        {
        double low_scale, high_scale_size, high_scale;
        if (sscanf(aParsedDimension.m_size_unit.c_str(),"m@%lf_%lfm@%lf",&low_scale,&high_scale_size,&high_scale) != 3)
            return false;
        aParsedDimension.m_size_unit = "m@";
        aParsedDimension.m_low_scale = low_scale;
        aParsedDimension.m_high_scale_size = high_scale_size;
        aParsedDimension.m_high_scale = high_scale;
        }

    return true;
    }

std::string PickColor(QWidget& aParent,const std::string aColor,bool aAllowAlpha)
    {
    CartoType::TColor color(aColor);
    QColor qcolor(color.Red(),color.Green(),color.Blue(),aAllowAlpha ? color.Alpha() : 255);
    QColorDialog color_dialog(qcolor,&aParent);
    if (aAllowAlpha)
        color_dialog.setOption(QColorDialog::ShowAlphaChannel);

    // Add NCS colours.
    color_dialog.setCustomColor(0,QColor(196, 2, 51));
    color_dialog.setCustomColor(1,QColor(0, 159, 127));
    color_dialog.setCustomColor(2,QColor(0, 135, 189));
    color_dialog.setCustomColor(3,QColor(255, 211, 0));

    if (color_dialog.exec() == QDialog::Accepted)
        {
        qcolor = color_dialog.currentColor();
        char buffer[64];
        if (aAllowAlpha)
            sprintf(buffer,"#%.2X%.2X%.2X%.2X",qcolor.red(),qcolor.green(),qcolor.blue(),qcolor.alpha());
        else
            sprintf(buffer,"#%.2X%.2X%.2X",qcolor.red(),qcolor.green(),qcolor.blue());
        return buffer;
        }

    return std::string();
    }

bool PickColor(QWidget& aParent,QLineEdit& aLineEdit,bool aAllowAlpha)
    {
    std::string color = PickColor(aParent,aLineEdit.text().toStdString(),aAllowAlpha);
    if (!color.empty())
        {
        aLineEdit.setText(QString(color.c_str()));
        return true;
        }
    return false;
    }

bool XmlValid(std::string aXml)
    {
    rapidxml::xml_document<> xml_document;
    std::string xml_copy { aXml };
    try
        {
        xml_document.parse<0>(&xml_copy[0]);
        }
    catch (rapidxml::parse_error e)
        {
        return false;
        }
    return true;
    }

std::string GetSaveFile(QWidget& aParent,std::string aCaption,std::string aDesc,std::string aExtension)
    {
    QString filter(aDesc.c_str());
    filter += " (*.";
    filter += aExtension.c_str();
    filter += ")";
    QFileDialog file_dialog(&aParent,aCaption.c_str(),"",filter);
    file_dialog.setAcceptMode(QFileDialog::AcceptSave);
    file_dialog.setDefaultSuffix(aExtension.c_str());
    file_dialog.setFileMode(QFileDialog::AnyFile);

    std::string filename;
    if (file_dialog.exec() == QDialog::DialogCode::Accepted)
        {
        QStringList file_list = file_dialog.selectedFiles();
        if (file_list.size())
            filename = file_list.begin()->toStdString();
        }
    return filename;
    }
