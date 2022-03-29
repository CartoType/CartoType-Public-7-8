#ifndef UTIL_H_
#define UTIL_H_

#include <cartotype_framework.h>
#include <QImage>
#include <QLineEdit>

class TParsedDimension
    {
    public:
    /** desired size */
    double m_size = 0;
    /** unit for desired size; if it's 'm@', m_low_scale, m_high_scale_size and m_high_scale are applicable */
    std::string m_size_unit;
    /** if non-zero, low end of scale range for logarithmic scaling */
    double m_low_scale = 0;
    /** if non-zero, size in map meters at high scale */
    double m_high_scale_size = 0;
    /** if non-zero, high end of scale range for logarithmic scaling */
    double m_high_scale = 0;
    /** minimum size */
    double m_min_size = 0;
    /** unit for minimum size */
    std::string m_min_size_unit;
    /** maximum size */
    double m_max_size = 0;
    /** unit for maximum size */
    std::string m_max_size_unit;
    };

void CopyBitmapToImage(const CartoType::TBitmap& aSource,QImage& aDest);
bool ParseDimension(TParsedDimension& aParsedDimension,const std::string& aDimension);
bool PickColor(QWidget& aParent,QLineEdit& aLineEdit,bool aAllowAlpha = true);
std::string PickColor(QWidget& aParent,const std::string aColor,bool aAllowAlpha = true);
bool XmlValid(std::string aXml);
std::string GetSaveFile(QWidget& aParent,std::string aCaption,std::string aDesc,std::string aExtension);

inline void SetString(QString& aDest,const CartoType::MString& aSource)
    {
    aDest.setUtf16(aSource.Text(),(int)aSource.Length());
    }

inline void SetString(QLineEdit& aDest,const CartoType::MString& aSource)
    {
    QString qs;
    SetString(qs,aSource);
    aDest.setText(qs);
    }

inline void SetString(CartoType::MString& aDest,const QLineEdit& aSource)
    {
    QString qs = aSource.text();
    aDest.Set(qs.utf16(),qs.length());
    }

#endif // UTIL_H__
