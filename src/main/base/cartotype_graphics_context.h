/*
cartotype_graphics_context.h
Copyright (C) 2004-2020 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_GRAPHICS_CONTEXT_H__
#define CARTOTYPE_GRAPHICS_CONTEXT_H__

#include <cartotype_base.h>
#include <cartotype_errors.h>
#include <cartotype_string.h>
#include <cartotype_transform.h>
#include <cartotype_path.h>
#include <cartotype_bitmap.h>

namespace CartoType
{

// forward declarations
class CEngine;
class CTypeface;
class CGlyph;
class CGraphicsContext;
class CFlatPath;
class TGlyphKey;
class TFont;
class CTexture;
class CProjection;

/** A type for an array of dashes and gaps used for drawing a stroke. */
class CDashArray: public std::vector<double>
    {
    public:
    CDashArray() = default;
    CDashArray(const MString& aNumberList);
    };

/** Methods of adding caps to the ends of lines created as envelopes of open paths. */
enum class TLineCap
    {
    /** End a line with a straight line intersecting the end point and normal to the line direction. */
    Butt,
    /** End a line with a half-circle. The center of the circle is the end point of the line. */
    Round,
    /** End a line with a half-square. The center of the square is the end point of the line. */
    Square
    };

/** Methods of joining segments of lines created as path envelopes. */
enum class TLineJoin
    {
    /** Use arcs of circles to join line segments. */
    Round,
    /** Extend line borders till they intersect. */
    Bevel,
    /** The same as Bevel, but if long spikes are produced they are cut off. */
    Miter
    };

/**
A circular pen used for stroking a path.

A line drawn using a circular pen can have various types of end caps and corners.
*/
class TCircularPen
    {
    public:
    /** The width of the pen in pixels. */
    double iPenWidth = 1;

    /** The line cap type. */
    TLineCap iLineCap = TLineCap::Round;

    /** The line join type. */
    TLineJoin iLineJoin = TLineJoin::Round;

    /**
    If the corners are mitered, they are cut off by a straight line if they
    extend more than this fraction of half the pen width from the center of the line.
    */
    double iMiterLimit = 2;

    /**
    If non-null, a pointer to an array giving the pattern of dashed and gaps used to draw strokes; not owned.
    The elements are the dash and gap lengths in pixels. They are used repeatedly, alternating between
    dashes and gaps; the first element is a dash.
    */
    std::shared_ptr<CDashArray> iDashArray;
    };

/** Parameters defining a hachure pattern. */
class THachureParam
    {
    public:
    THachureParam() { }

    /** Constructs a THachureParam object from a color, stroke width, interval between strokes, and stroke angle. */
    THachureParam(TColor aColor,double aStrokeWidth,double aInterval,double aAngle):
        iColor(aColor),
        iStrokeWidth(aStrokeWidth),
        iInterval(aInterval),
        iAngle(aAngle)
        {
        }

    /** The stroke color. */
    TColor iColor;
    /** The stroke width in pixels. */
    double iStrokeWidth = 0;
    /** The interval between strokes in pixels. */
    double iInterval = 0;
    /** The stroke angle in radians. */
    double iAngle = 0;
    };

/**
A paint server supplies a color for any given pixel. Paint servers are used to provide
the correct color when filling shapes using gradients or patterns.
*/
class CPaintServer
    {
    public:
    virtual ~CPaintServer() = default;

    /** Supplies a color for the pixel at (aX,aY) in premultiplied RGBA format. */
    virtual TColor Color(int32_t aX,int32_t aY) = 0;

    /** Returns the texture: a bitmap which is the smallest possible repeating element. Returns null if that is not possible. */
    virtual std::shared_ptr<CBitmap> Texture() = 0;

    /** Creates a new paint server with colors converted to night mode or other color effect by blending with the specified color. */
    virtual std::shared_ptr<CPaintServer> Blended(TColor aBlendColor) = 0;

    /** Returns a pointer to a 256-element color ramp if there is one. */
    virtual const TColor* Ramp() const { return nullptr; }

    /** Returns a pointer to the hachure parameters if this is a hachure. */
    virtual const THachureParam* HachureParam() const { return nullptr; }

    /** The name used in a defs section in a style sheet to refer to the paint server. */
    CString iName;
    };

/**
A paint server that draws a bitmap as a repeating pattern of rectangular tiles.
Successive rows or columns can optionally be offset to give a less square appearance.
*/
class CPattern: public CPaintServer
    {
    public:
    /** Creates a pattern referring to a certain bitmap. Does not take ownership of the bitmap. */
    CPattern(std::shared_ptr<CBitmap> aBitmap,int32_t aXOffset,int32_t aYOffset):
        iBitmap(aBitmap),
        iColorFunction(aBitmap->ColorFunction()),
        iXOffset(aXOffset),
        iYOffset(aYOffset)
        {
        }
    TColor Color(int32_t aX,int32_t aY) override;
    std::shared_ptr<CBitmap> Texture() override;
    std::shared_ptr<CPaintServer> Blended(TColor /*aBlendColor*/) override { return std::make_shared<CPattern>(*this); }

    private:
    std::shared_ptr<CBitmap> iBitmap;
    TBitmap::TColorFunction iColorFunction;
    int32_t iXOffset;
    int32_t iYOffset;
    };

/** A paint source containing a color and an optional pointer to a paint server. */
class TPaint
    {
    public:
    TPaint() = default;
    /** Creates a TPaint object with a specified color. */
    TPaint(uint32_t aValue):
        iColor(aValue) { }
    /** Creates a TPaint object with a specified color. */
    TPaint(TColor aColor):
        iColor(aColor) { }
    /** Creates a TPaint object with a specified paint server and alpha (opacity) value. */
    TPaint(std::shared_ptr<CPaintServer> aPaintServer,int32_t aAlpha = 255):
        iColor(0,0,0,aAlpha),
        iPaintServer(aPaintServer)
        {
        }
    /**
    Return true if this paint source is null,
    defined as having a null paint server and a completely transparent color.
    */
    bool IsNull() const { return !iPaintServer && !(iColor.iValue & 0xFF000000); }
    
    /** Converts the color to night mode or other color blend. */
    void SetBlendColor(const TColor& aBlendColor)
        {
        if (aBlendColor.IsNull())
            return;
        iColor.Blend(aBlendColor);
        if (iPaintServer)
            iPaintServer = iPaintServer->Blended(aBlendColor);
        }

    /** The paint color. */
    TColor iColor;
    /** If non-null, the paint server. */
    std::shared_ptr<CPaintServer> iPaintServer;
    };

/** Parameters used by a graphics context. */
class TGraphicsParam
    {
    public:
    /** The clipping rectangle, in pixels. */
    TRect iClip;

    /** The paint. */
    TPaint iPaint;

    /** The circular pen used by DrawStroke. */
    TCircularPen iPen;

    /**
    The color used to inhibit texture drawing.
    Textures are never drawn to pixels of this color.
    The purpose is to prevent terrain textures from being drawn in sea or lake areas.
    The texture mask is set to the map background color when a map is drawn.
    */
    TColor iTextureMask = KTransparentBlack;
    };

/** The data for a typeface is either a filename or a pointer to memory. */
class TTypefaceData
    {
    public:
    TTypefaceData() = default;

    /** Creates a typeface data object representing a filename. */
    explicit TTypefaceData(const MString& aName):
        iName(aName)
        {
        }

    /** Creates a typeface data object representing a filename in UTF-8. */
    explicit TTypefaceData(const char* aName):
        iName(aName)
        {
        }

    /**
    Creates a typeface data object for data in memory. If aCopyData is true
    the data is copied, otherwise only the pointer is stored.
    */
    TTypefaceData(const uint8_t* aData,size_t aLength,bool aCopyData):
        iData(aData),
        iLength(aLength)
        {
        assert(aData != nullptr);
        if (aCopyData)
            {
            iOwnData = std::make_shared<std::vector<uint8_t>>(aLength);
            memcpy(iOwnData->data(),aData,aLength);
            iData = iOwnData->data();
            }
        }

    /** Returns true if the data is in memory. */
    bool InMemory() const { return iData != nullptr; }
    /** Returns the name of the file, if the data is in a file. */
    const MString& Name() const { return iName; }
    /** Gets a pointer to the in-memory data and its length in bytes. */
    void GetData(const uint8_t*& aData,size_t& aLength) const { aData = iData; aLength = iLength; }

    private:
    CString iName;
    const uint8_t* iData = nullptr;
    std::shared_ptr<std::vector<uint8_t>> iOwnData;
    size_t iLength = 0;
    };

/*
Constants used to refer to scripts in TTypefaceAttrib, etc.
There are only 32 constants and all are assigned, because they are
used as flags in a 32-bit word to show which scripts are supported by a typeface.

Scripts not encoded, such as Syriac, Thaana, Cherokee, Runic,
etc., are represented by KOtherScript, except for symbol sets and
'pi fonts', which are indicated by KSymbolScript.

There is a reserved code, KReservedScript, which must not be used.
*/

/** A bit flag used in typeface matching: the Latin script. */
const uint32_t KLatinScript = 1;
/** A bit flag used in typeface matching: the Greek script. */
const uint32_t KGreekScript = 2;
/** A bit flag used in typeface matching: the Cyrillic script. */
const uint32_t KCyrillicScript = 4;
/** A bit flag used in typeface matching: the Armenian script. */
const uint32_t KArmenianScript = 8;
/** A bit flag used in typeface matching: the Hebrew script. */
const uint32_t KHebrewScript = 0x10;
/** A bit flag used in typeface matching: the Arabic script. */
const uint32_t KArabicScript = 0x20;
/** A bit flag used in typeface matching: the Devanagari script. */
const uint32_t KDevanagariScript = 0x40;
/** A bit flag used in typeface matching: the Bengali script. */
const uint32_t KBengaliScript = 0x80;
/** A bit flag used in typeface matching: the Gurmukhi script. */
const uint32_t KGurmukhiScript = 0x100;
/** A bit flag used in typeface matching: the Gujarati script. */
const uint32_t KGujaratiScript = 0x200;
/** A bit flag used in typeface matching: the Oriya script. */
const uint32_t KOriyaScript = 0x400;
/** A bit flag used in typeface matching: the Tamil script. */
const uint32_t KTamilScript = 0x800;
/** A bit flag used in typeface matching: the Telugu script. */
const uint32_t KTeluguScript = 0x1000;
/** A bit flag used in typeface matching: the Kannada script. */
const uint32_t KKannadaScript = 0x2000;
/** A bit flag used in typeface matching: the Malayalam script. */
const uint32_t KMalayalamScript = 0x4000;
/** A bit flag used in typeface matching: the Sinhala script. */
const uint32_t KSinhalaScript = 0x8000;
/** A bit flag used in typeface matching: the Thai script. */
const uint32_t KThaiScript = 0x10000;
/** A bit flag used in typeface matching: the Lao script. */
const uint32_t KLaoScript = 0x20000;
/** A bit flag used in typeface matching: the Tibetan script. */
const uint32_t KTibetanScript = 0x40000;
/** A bit flag used in typeface matching: the Myanmar script. */
const uint32_t KMyanmarScript = 0x80000;
/** A bit flag used in typeface matching: the Georgian script. */
const uint32_t KGeorgianScript = 0x100000;
/** A bit flag used in typeface matching: the Hangul script. */
const uint32_t KHangulScript = 0x200000;
/** A bit flag used in typeface matching: the Ethiopic script. */
const uint32_t KEthiopicScript = 0x400000;
/** A bit flag used in typeface matching: the Khmer script. */
const uint32_t KKhmerScript = 0x800000;
/** A bit flag used in typeface matching: the Mongolian script. */
const uint32_t KMongolianScript = 0x1000000;
/** A bit flag used in typeface matching: the Japanese Hiragana script. */
const uint32_t KHiraganaScript = 0x2000000;
/** A bit flag used in typeface matching: the Japanese Katakana script. */
const uint32_t KKatakanaScript = 0x4000000;
/** A bit flag used in typeface matching: the Bopomofo script. */
const uint32_t KBopomofoScript = 0x8000000;
/** A bit flag used in typeface matching: the Han script. */
const uint32_t KHanScript = 0x10000000;
/** A bit flag used in typeface matching: this value is reserved and must not be used. */
const uint32_t KReservedScript = 0x20000000;
/** A bit flag used in typeface matching: Symbols and other non-alphabetic characters. */
const uint32_t KSymbolScript = 0x40000000;
/** A bit flag used in typeface matching: other scripts. */
const uint32_t KOtherScript = 0x80000000;

/**
Constants used to refer to styles in TTypefaceAttrib, etc.
These are flags used in a 32-bit word.
*/

/** The bit flag used to select bold face in TTypefaceAttrib::Style, etc. */
const uint32_t KBoldStyle = 1;
/** The bit flag used to select italics in styles in TTypefaceAttrib::Style, etc. */
const uint32_t KItalicStyle = 2;
/** The bit flag used to select a serif font in TTypefaceAttrib::Style, etc. */
const uint32_t KSerifStyle = 4;
/** The bit flag used to select a cursive font in TTypefaceAttrib::Style, etc. */
const uint32_t KCursiveStyle = 8;
/**
The bit flag used to select a 'fantasy' font (as defined in
http://www.w3.org/TR/REC-CSS2/fonts.html#generic-font-families)
in TTypefaceAttrib::Style, etc.
*/
const uint32_t KFantasyStyle = 16;
/** The bit flag used to select a monospace font in TTypefaceAttrib::Style, etc. */
const uint32_t KMonospaceStyle = 32;

/** The fixed attributes of a typeface. */
class TTypefaceAttrib
    {
    public:
    /** The equality operator. */
    bool operator==(const TTypefaceAttrib& aAttrib) const
        { return iStyle == aAttrib.iStyle && iScripts == aAttrib.iScripts && iName == aAttrib.iName; }
    /** The inequality operator. */
    bool operator!=(const TTypefaceAttrib& aAttrib) const
        { return !(*this == aAttrib); }

    /** The style: bold, italic, etc. */
    uint32_t iStyle = 0;
    /** Flags broadly indicating the supported scripts. */
    uint32_t iScripts = 0;
    /** The name of the typeface. */
    CString iName;
    };

/**
The changeable attributes of a typeface that determine
how it creates glyphs.
*/
class TTypefaceInstance
    {
    public:
    /** The equality operator. */
    bool operator==(const TTypefaceInstance& aInstance) const
        { return iFlags == aInstance.iFlags && iSize == aInstance.iSize && iTransform == aInstance.iTransform; }
    /** The inequality operator. */
    bool operator!=(const TTypefaceInstance& aInstance) const
        { return !(*this == aInstance); }
    /** Sets a typeface instance's size in pixels per em to aSize and resets the font transform to identity. */
    void SetToSize(double aSize);

    /** A flag used in iFlags. */
    static constexpr uint32_t KAntiAlias = 1;

    /** The size in pixels per em, before any transform is applied. */
    double iSize = 12;
    /**
    The transform used to apply rotation
    and slant. This transform also affects the baseline.
    */
    TTransform iTransform;
    /** Flags controlling anti-aliasing and glyph effects. */
    uint32_t iFlags = KAntiAlias;
    };

/** A font specification is used to select the nearest match for typeface and instance. */
class TFontSpec
    {
    public:
    /** The equality operator. */
    bool operator==(const TFontSpec& aFontSpec) const
        { return iInstance == aFontSpec.iInstance && iAttrib == aFontSpec.iAttrib && iColor == aFontSpec.iColor; }
    /** The inequality operator. */
    bool operator!=(const TFontSpec& aFontSpec) const
        { return !(*this == aFontSpec); }
    /** Sets the typeface name to aName. */
    void SetName(const CString& aName) { iAttrib.iName.Set(aName); }
    /**
    Sets the em size to aSize and the typeface instance's transformation to
    identity, removing any slant, skew, stretch or rotation.
    */
    void SetToSize(double aSize) { iInstance.SetToSize(aSize); }
    /** Returns the em size before any transformation by the font transform. */
    double Size() const { return iInstance.iSize; }

    /** The typeface attributes: name, style, and required scripts. */
    TTypefaceAttrib iAttrib;
    /** The instance specification: size (expressed as a transform) and glyph rendering method (e.g., anti-alias versus monochrome). */
    TTypefaceInstance iInstance;
    /**
    If non-null, the text color; otherwise the graphic context's color is used.
    Font colors are supported for labels only, when using embedded font selectors.
    */
    TColor iColor = KTransparentBlack;
    };

/** The style of a text box used in labels. The meanings of iPadding and iMargin are as in HTML. */
class TTextBoxStyle
    {
    public:
    /** The width in pixels. */
    double iWidth = 0;
    /** The padding in pixels: the gap inside the border, between the border and the textual content. */
    double iPadding = 0;
    /** The margin in pixels: the gap outside the border, between the border and other elements. */
    double iMargin = 0;
    /** The background color. The background is not drawn if the background color is completely transparent. */
    TColor iBackgroundColor = KTransparentBlack;
    /** The background color. The border is not drawn if the border color is completely transparent. */
    TColor iBorderColor = KTransparentBlack;
    /** The border width in pixels. No border is drawn if the value is zero or less. */
    double iBorderWidth = 0;
    /** The radius of curved corners on the border. If it is zero or less the corners are square. */
    double iBorderRadius = 0;
    };

/** Baselines used for aligning text. */
enum class TTextBaseline
    {
    /** The baseline for Latin and similar scripts. It is at or near the bottom of 'A'. */
    Alphabetic,

    /** The bottom edge of Han ideographic characters. */
    Ideographic,

    /** The top edge of characters in hanging Indic scripts like Devanagari. */
    Hanging,

    /** The baseline used for mathematical symbols. */
    Mathematical,

    /**
    A baseline half way between the leading and trailing edges
    (top and bottom, for horizontal setting) of the em square.
    */
    Central,

    /**
    A baseline that is offset from the alphabetic baseline upwards by 1/2 the value of
    the x-height.
    */
    Middle,

    /** The top edge of the em box. */
    TextBeforeEdge,

    /** The bottom edge of the em box. */
    TextAfterEdge,

    /**
    A baseline half way between the ascent line and the alphabetic baseline,
    suitable for text that is wholly in capital letters.
    */
    CentralCaps
    };

/** Alignments used for multi-line text. */
enum class TAlign
    {
    /** Center the text. */
    Center,
    /** Align to the left for left-to-right text, to the right for right-to-left text. */
    Standard,
    /** Align to the right for left-to-right text, to the left for right-to-left text. */
    Reverse,
    /** Align to the left and leave space on the right. */
    Left,
    /** Align to the right and leave space on the left. */
    Right
    };

/** Parameters to control the way text is drawn or measured. */
class TTextParam
    {
    public:
    TTextParam():
        iTraversePathBackwards(false),
        iCopyFit(false),
        iRoundAdvances(false)
        {
        }

    /**
    If the text cannot be fitted into iMaxWidth, measured
    along the baseline in pixels, it is truncated.
    */
    int32_t iMaxWidth = INT32_MAX;

    /**
    If iFlatPath is non-null the text is drawn along the path,
    which is relative to the origin passed to DrawText.
    */
    const CFlatPath* iFlatPath = nullptr;

    /** If drawing along a path, the distance along the path to start at. */
    double iPathStart = 0;

    /**
    If drawing on a path, the sine of the maximum tolerated angle between
    baselines of successive characters. The default is .707 = sin(45 degrees).
    */
    double iMaxTurnSine = 0.707106781;

    /** The type of the baseline used to align the text. */
    TTextBaseline iBaseline = TTextBaseline::Alphabetic;

    /**
    The baseline offset in pixels. This value is added to the baseline's y coordinate.
    It can be used to shift the text away from the path it is drawn along, for example
    when drawing a road label beside the road instead of along the road.
    */
    double iBaselineOffset = 0;

    /**
    Letter spacing in pixels. This value is added to all character advances.
    */
    double iLetterSpacing = 0;

    /**
    Word spacing in pixels. This value is added to the advance of word spaces.
    */
    double iWordSpacing = 0;

    /**
    The greatest factor by which a string can be expanded when
    copy-fitting by adding letter spacing and word spacing.
    The default value is 4.
    */
    double iMaxCopyFitExpansion = 4;

    /**
    The number of fonts in iFont.
    An array of fonts can be supplied to override the
    TFont object used to draw or measure the text, and thus
    allow rich text to be drawn.
    */
    size_t iFonts = 0;

    /**
    A pointer to an array of fonts, to be selected
    using special characters embedded in the text.
    This pointer must be non-null if iFonts is greater than zero.
    */
    const TFont* iFont = nullptr;

    /** If drawing text on a path, traverse the path backwards. */
    bool iTraversePathBackwards : 1;
    /**
    If drawing text on a path, stretch it out to fit the path
    by adding letter spacing and word spacing.
    */
    bool iCopyFit : 1;
    /**
    Round advances to integer values, which is normally
    done if drawing horizontally or vertically, to maximise glyph cache hits.
    */
    bool iRoundAdvances : 1;
    };

/**
Text metrics objects are filled in by TFont::DrawText and provide the advance,
bounding box, number of characters drawn, etc.
*/
class TTextMetrics
    {
    public:
    /** Clear the metrics, including any rectangle pointed to by aBounds. */
    void Clear()
        {
        iAdvance = TPointFP();
        iLength = 0;
        if (iBounds)
            *iBounds = TRect();
        iCharacters = 0;
        }

    /** The amount by which the origin is moved by drawing the text. */
    TPointFP iAdvance;

    /**
    The linear distance by which the origin is moved. If text is drawn on
    a curved path this is not the same as the vector length of iAdvance.
    */
    double iLength = 0;

    /** If this is non-null it receives the bounding rectangle of the pixels drawn. */
    TRect* iBounds = nullptr;

    /** The number of characters drawn. */
    size_t iCharacters = 0;
    };

/**
The metrics of a font: that is a typeface rendered using a certain instance.
Metrics such as ascent are in pixels relative to the baseline, with y increasing downwards.
*/
class TFontMetrics
    {
    public:
    /**
    Returns the vertical offset in pixels applied to the baseline for this set
    of font metrics and the specified baseline type.
    */
    double BaselineOffset(TTextBaseline aBaseline) const;

    /** Design size in pixels per em. */
    int32_t iSize = 0;
    /** Offset of top of Latin capital H (U+0048) from baseline in pixels (normally negative) */
    int32_t iAscent = 0;
    /** Offset of bottom of Latin lowercase p (U+0070) from baseline in pixels (normally positive) */
    int32_t iDescent = 0;
    /** Offset of top of Latin lowercase x (U+0078) from baseline in pixels (normally negative) */
    int32_t iXHeight = 0;
    /** Maximum offset of top of any character from baseline in pixels (normally negative) */
    int32_t iMaxAscent = 0;
    /** Maximum offset of bottom of any character from baseline in pixels (normally positive) */
    int32_t iMaxDescent = 0;
    };

/** A font associates a font specification with a matching typeface. */
class TFont
    {
    public:
    /**
    Construct a null font. It cannot be used until another font has been
    assigned to it.
    */
    TFont() = default;
    ~TFont();

    /** Construct a font with default attributes. */
    TFont(std::shared_ptr<CEngine> aEngine):
        iEngine(aEngine)
        {
        }

    /** Construct a font that is the best available match for aFontSpec. */
    TFont(std::shared_ptr<CEngine> aEngine,const TFontSpec& aFontSpec):
        iEngine(aEngine),
        iFontSpec(aFontSpec)
        {
        }

    /**
    Set the em size to aSize and set other components of the typeface instance's transformation to a
    plain scaling transformation, removing any slant, skew, stretch or rotation.
    */
    void SetToSize(double aSize) { iFontSpec.SetToSize(aSize); }
    /** Return the font specification used to construct this font. */
    const TFontSpec& FontSpec() const { return iFontSpec; }
    /** Set the text color; if it's null (completely transparent) the graphic context's current color is used. */
    void SetColor(TColor aColor) { iFontSpec.iColor = aColor; }
    /** Get the text color; if it's null (completely transparent) the graphic context's current color is used. */
    TColor Color() const { return iFontSpec.iColor; }
    /**
    Draws or measures text. If aGc is null the text is measured, otherwise it is drawn into aGc.
    Drawing is controlled by aParam. Metrics are returned in aMetrics.
    */
    TDrawResult DrawText(CGraphicsContext* aGc,const MString& aText,const TPoint& aOrigin,const TTextParam& aParam,TTextMetrics& aMetrics);
    /** Gets the size of a font in pixels per em. */
    int32_t Size();
    /**
    Returns the metrics of the current instance (size, etc.,) of the font.
    If no instance has been set null metrics are returned.
    */
    const TFontMetrics& Metrics();
    /** Return true if this is a null font, which cannot be used until another font has been assigned to it. */
    bool IsNull() const { return iEngine == nullptr; }

    private:
    std::shared_ptr<CGlyph> Glyph(const TGlyphKey& aKey);

    std::shared_ptr<CEngine> iEngine;       // the graphics engine: needed for finding typefaces and glyphs
    TFontSpec iFontSpec;		            // the ideal font specification
    std::shared_ptr<CTypeface> iTypeface;   // the typeface that best matches iFontSpec
    std::shared_ptr<CTypeface> iAltTypeface;// the typeface currently used for characters not found in iTypeface
    };

/** A functor class to handle labels drawn separately from the map. */
class MLabelHandler
    {
    public:
    virtual ~MLabelHandler() { }
    /** Handle a label, passed as a 32bpp RGBA bitmap to be drawn at aTopLeft. The label is associated with the position aHotSpot. */
    virtual void operator()(const TBitmap& aLabelBitmap,const TPoint& aTopLeft,const TPoint& aHotSpot) = 0;
    };

/**
Tile bitmap parameters are used in the various tile drawing functions to control
whether map objects and labels are drawn, and whether labels are passed to an external handler.
*/
class TTileBitmapParam
    {
    public:
    /** If true, draw the map objects. */
    bool iDrawMapObjects = true;
    /** If true, draw the labels. */
    bool iDrawLabels = true;
    /** If true, draw the background. */
    bool iDrawBackground = true;
    /** If iLabelHandler is non-null, and iDrawLabels is true, labels are passed to iLabelHandler as bitmaps, not drawn on the map. */
    MLabelHandler* iLabelHandler = nullptr;
    };

/**
The base graphics context class.
A graphics context draws to a raster drawing surface with square pixels.
*/
class CGraphicsContext
    {
    public:
    CGraphicsContext(std::shared_ptr<CEngine> aEngine,const TRect& aBounds,std::unique_ptr<CBitmap> aBitmap);
    virtual ~CGraphicsContext() { }

    /**
    Draws a bitmap. If the bitmap has no color information, uses the current color.
    Combines any alpha information from the bitmap with the current alpha level.
    The coordinates are whole pixels.
    */
    virtual TDrawResult DrawBitmap(const TBitmap& aBitmap,const TPoint& aTopLeft) = 0;

    /** Draws a filled shape in the current color. The coordinates are in 64ths of pixels. */
    virtual TDrawResult DrawShape(const MPath& aPath) = 0;

    /** Draws a texture at aTopLeft, transforming it by aTransform. */
    virtual TDrawResult DrawTexture(const CTexture& aTexture,const TPointFP& aTopLeft,const TTransform& aTransform) = 0;

    /**
    Clears the drawing area to transparent black.
    If colors are not stored sets all pixels to zero intensity.
    If transparency levels are not stored sets all pixels to black.
    */
    virtual void Clear() = 0;

    /** Returns a pointer to the bitmap, if any. */
    virtual TBitmap* Bitmap();
    virtual TDrawResult DrawRect(const TRect& aRect);
    virtual bool TransformsPoints();
    [[nodiscard]] virtual TDrawResult Transform2D(TPointFP& aPoint,int32_t aFractionalBits);
    [[nodiscard]] virtual TDrawResult Transform3D(TPoint3FP& aPoint);
    virtual void InverseTransform2D(TPointFP& aPoint,int32_t aFractionalBits);
    virtual bool EnableTransform(bool aEnable);
    virtual TRect UntransformedBounds();
    virtual TDrawResult DrawStroke(const MPath& aPath,const TTransform* aTransform = nullptr,bool aClip = false);
    virtual void SetGlow(TColor aColor,double aWidth,const TPointFP& aOffset);
    virtual void GetGlow(TColor& aColor,double& aWidth,TPointFP& aOffset);
    virtual TDrawResult DrawBitmapMonochrome(const TBitmap& aBitmap,const TPoint& aTopLeft);

    TDrawResult DrawShapeAndStroke(const MPath& aPath,const TPaint& aStrokePaint,const TTransform* aTransform = nullptr);
    void SetClip(const TRect& aClip);
    void SetColor(TColor aColor);
    void SetPaintServer(std::shared_ptr<CPaintServer> aPaintServer);
    void SetPaint(const TPaint& aPaint);
    void SetAlpha(int32_t aAlpha);
    void SetTextureMask(TColor aColor);
    void SetParam(TGraphicsParam& aParam);
    void SetPen(const TCircularPen& aPen);
    [[nodiscard]] TDrawResult Transform(TPoint& aPoint,int32_t aFractionalBits);

    /** Swaps the bitmap with another one. */
    void SwapBitmap(std::unique_ptr<CBitmap>& aBitmap) { std::swap(aBitmap,iBitmap); }

    /** Returns a pointer to the graphics engine. */
    std::shared_ptr<CEngine> Engine() { return iEngine; }

    /** Returns the clipping rectangle. */
    const TRect& Clip() const { return iParam.iClip; }

    /** Returns the current paint. */
    const TPaint& Paint() const { return iParam.iPaint; }

    /** Returns the current color as an RGBA value including the alpha channel. */
    TColor Color() const { return iParam.iPaint.iColor; }

    /** Returns the current alpha (opacity) level as a number in the range 0 ... 255. */
    int32_t Alpha() const { return iParam.iPaint.iColor.Alpha(); }

    /** Return the rectangle that the graphics context draws to if no clipping is done. */
    const TRect& Bounds() const { return iBounds; }

    /** Returns the pen used for drawing strokes. */
    const TCircularPen& Pen() const { return iParam.iPen; }

    /** Sets the array of dash and gap sizes used for dashed strokes. */
    void SetDashArray(std::shared_ptr<CDashArray> aDashArray) { iParam.iPen.iDashArray = aDashArray; }

    /** Returns the current dash array. */
    std::shared_ptr<CDashArray> DashArray() const { return iParam.iPen.iDashArray; }

    /** Returns the current graphics parameters. */
    const TGraphicsParam& Param() const { return iParam; }

    /** Multiplies two intensities, treated as fractions in the range 0...255. */
    static uint8_t MultiplyIntensities(int aIntensity1,int aIntensity2)
        {
        return (uint8_t)((aIntensity1 * aIntensity2 + 255) >> 8);
        }

    /**
    Blends aForeground and aBackground in the proportion aAlpha.
    All three numbers are intensities in the range 0...255.
    */
    static uint8_t AlphaBlend(int aForeground,int aBackground,int aAlpha)
        {
        return (uint8_t)(aBackground + (((aForeground - aBackground) * aAlpha + 255) >> 8));
        }

    protected:
    TDrawResult DrawDashedStroke(const MPath& aPath,const TTransform* aTransform);

    /** The engine object, which provides the shape rendering engines and other shared resources. */
    std::shared_ptr<CEngine> iEngine;
    /** The drawing parameters such as the color and clip rectangle. */
    TGraphicsParam iParam;
    /** The bounds, in pixels, of the device drawn into. */
    TRect iBounds;

    /** A flag used in iChangeFlags: the clip rectangle has changed. */
    static constexpr int32_t KClipChanged = 1;
    /** A flag used in iChangeFlags: the paint has changed. */
    static constexpr int32_t KPaintChanged = 2;
    /** A flag used in iChangeFlags: the glow color, if any, has changed. */
    static constexpr int32_t KGlowColorChanged = 4;
    /** A flag used in iChangeFlags: the texture mask has changed. */
    static constexpr int32_t KTextureMaskChanged = 8;
    /** A flag used in iChangeFlags: all graphics parameters have changed; the internal state must be completely initialised. */
    static constexpr int32_t KAllChanged = -1;

    /**
    Flags that are set when graphics parameters are changed,
    allowing derived classes to synchronize their internal states if necessary.
    */
    int32_t iChangeFlags = KAllChanged;

    /** If non-null, the bitmap owned by the graphics context. */
    std::unique_ptr<CBitmap> iBitmap;

    private:
    void SetStrokeClip();

    /** The clip rectangle, in 64ths, used for clipping strokes. It is wider by the pen width than the ordinary clip rectangle, */
    TRect iStrokeClip;
    };

/**
Textures are bitmaps that can be drawn using an arbitrary 2D transformation.
They are usually implemented by graphics acceleration systems.
The CTexture class is an abstract base class to be implemented by a
graphics library such as OpenGL.
*/
class CTexture
    {
    public:
    virtual ~CTexture() { }
    /** Returns the bitmap representing the texture, if any. */
    virtual const TBitmap* Bitmap() const = 0;
    /** Returns the bounds of the texture in pixels. */
    virtual void GetBounds(TRect& aRect) const = 0;
    /** Returns true if the texture is empty. */
    virtual bool IsEmpty() const = 0;
    };

/** A bitmap texture class that does not own its bitmap. */
class CBitmapTexture: public CTexture
    {
    public:
    /** Creates a texture referring to a bitmap. */
    CBitmapTexture(const TBitmap* aBitmap):
        iBitmap(aBitmap)
        {
        }
    /** Returns the bitmap representing the texture. */
    const TBitmap* Bitmap() const { return iBitmap; }
    /** Returns the bounds of the texture in pixels. */
    void GetBounds(TRect& aRect) const
        {
        aRect = TRect();
        if (iBitmap)
            aRect.iBottomRight = TPoint(iBitmap->Width(),iBitmap->Height());
        }
    /** Returns true if the texture is empty. */
    bool IsEmpty() const { return iBitmap == nullptr; }

    private:
    const TBitmap* iBitmap;
    };

} // namespace CartoType

#endif
