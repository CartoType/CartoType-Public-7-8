/*
cartotype_legend.h
Copyright (C) 2015-2021 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_LEGEND_H__
#define CARTOTYPE_LEGEND_H__

#include <cartotype_graphics_context.h>
#include <cartotype_framework_observer.h>
#include <vector>
#include <memory>

namespace CartoType
{

class CLegendObjectParam;
class CFramework;
class CThreadSafeNavigationState;

/**
The CLegend class creates bitmaps showing sample map objects, with optional text lines and a scale,
to be used as a map legend or for visualisation in the style sheet editor.

Dimensions are specified with units, which may be "pt" (point), "pc" (pica), "cm" (centimetre),
"mm" (millimetre), "in" (inch), or "px" (pixel). An empty or unrecognised unit is taken as "px".

It's relatively expensive to construct a CLegend object, so it is best to keep such an object in
existence rather than creating one as a temporary object in a draw loop.
*/
class CLegend: public MFrameworkObserver
    {
    public:
    CLegend(CFramework& aFramework);
    ~CLegend();
    /** Creates a CLegend object by copying another one. */
    CLegend(const CLegend& aOther);
    /** The assignment operator: copies aOther to this CLegend. */
    CLegend& operator=(const CLegend& aOther);
    
    std::unique_ptr<CBitmap> CreateLegend(TResult& aError,double aWidth,const char* aUnit,double aScaleDenominator,double aScaleDenominatorInView,const TBlendStyleSet* aBlendStyleSet = nullptr);
    void Clear();
    int32_t Pixels(double aDimension,const char* aUnit);

    void AddMapObjectLine(TMapObjectType aType,const CString& aLayer,const char* aOsmType,int32_t aIntAttrib,const CString& aStringAttrib,const CString& aLabel);
    void AddTextLine(const CString& aText);
    void AddScaleLine(bool aMetricUnits);
    void AddTurnLine(bool aMetricUnits,bool aAbbreviate);

    void SetMainStyleSheet(const uint8_t* aData,size_t aLength);
    void SetExtraStyleSheet(const uint8_t* aData,size_t aLength);
    void SetBackgroundColor(TColor aColor);
    void SetBorder(TColor aColor,double aStrokeWidth,double aRadius,const char* aUnit);
    void SetMarginWidth(double aMarginWidth,const char* aUnit);
    void SetMinLineHeight(double aLineHeight,const char* aUnit);
    void SetLabelWrapWidth(double aWrapWidth,const char* aUnit);
    void SetFontFamily(const CString& aFontFamily);
    void SetFontSize(double aFontSize,const char* aUnit);
    void SetTextColor(TColor aTextColor);
    void SetDiagramColor(TColor aDiagramColor);
    void SetAlignment(TAlign aAlignment);
    void SetPolygonRotation(double aDegrees);
    bool HasTurnInstruction() const;
    void SetTurnInstruction(const MString& aText);
    CString TurnInstruction();
    uint32_t Serial() const;
    std::shared_ptr<CThreadSafeNavigationState> NavigationState() const;
    void SetNavigationState(std::shared_ptr<CThreadSafeNavigationState> aState);

    private:
    std::unique_ptr<CBitmap> CreateLegendInternal(double aWidth,const char* aUnit,double aScaleDenominator,double aScaleDenominatorInView,const TBlendStyleSet* aBlendStyleSet);
    void DrawScale(CGraphicsContext& aGc,const CLegendObjectParam& aParam,int32_t aX,int32_t aY,int32_t aWidth,TColor aBlendColor);
    void Copy(const CLegend& aOther);

    // virtual functions from MFrameworkObserver
    void OnRoute(const CRoute* aRoute) override;
    void OnTurn(const TNavigatorTurn& aFirstTurn,const TNavigatorTurn* aSecondTurn,const TNavigatorTurn* aContinuationTurn) override;
    void OnState(TNavigationState aState) override;

    std::unique_ptr<CFramework> m_framework;
    std::vector<CLegendObjectParam> m_object_array;
    std::shared_ptr<CThreadSafeNavigationState> m_navigation_state;
    TColor m_background_color { KWhite };
    TColor m_border_color { KGray };
    int32_t m_border_width_in_pixels { };
    int32_t m_border_radius_in_pixels { };
    int32_t m_margin_width_in_pixels { };
    int32_t m_min_line_height_in_pixels { };
    int32_t m_label_wrap_width_in_pixels { };
    TFontSpec m_font_spec;
    TColor m_color { KGray };
    TColor m_diagram_color { KGray };
    TAlign m_alignment = TAlign::Left;
    double m_polygon_rotation { 0 };
    double m_metre { };
    double m_pt { };
    double m_inch { };
    };

}

#endif // CARTOTYPE_LEGEND_H__
