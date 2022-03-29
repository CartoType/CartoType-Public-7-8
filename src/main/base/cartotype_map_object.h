/*
cartotype_map_object.h
Copyright (C) 2013-2021 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_MAP_OBJECT_H__
#define CARTOTYPE_MAP_OBJECT_H__

#include <cartotype_string.h>
#include <cartotype_path.h>
#include <cartotype_address.h>
#include <cartotype_transform.h>
#include <cartotype_geometry.h>

#include <map>

namespace CartoType
{
class CBasicMapObject;
class CMapAttributeTest;
class CMapObject;
class TMapTransform;
class CProjection;
class TBitmap;
class TAttribSet;

/** A group of map objects with the same name returned by a search function. */
class CMapObjectGroup
    {
    public:
    CMapObjectGroup() = default;

    /** Creates a CMapObjectGroup by moving data from another one. */
    CMapObjectGroup(CMapObjectGroup&& aOther) noexcept
        {
        if (this == &aOther)
            return;
        iName = std::move(aOther.iName);
        iMapObjectArray = std::move(aOther.iMapObjectArray);
        }

    /** Assigns data to a CMapObjectGroup by moving data from another one. */
    CMapObjectGroup& operator=(CMapObjectGroup&& aOther) noexcept
        {
        if (this == &aOther)
            return *this;
        iName = std::move(aOther.iName);
        iMapObjectArray = std::move(aOther.iMapObjectArray);
        return *this;
        }

    /** The name, summary address or other common attribute of all the objects. */
    CString iName;
    /** The map objects. */
    CMapObjectArray iMapObjectArray;
    };

/** A type for arrays of map object groups returned by search functions. */
using CMapObjectGroupArray = std::vector<CMapObjectGroup>;

/**
The abstract base map object class. A map object is a path that
also possesses cartographic attributes.
*/
class CMapObject: public MPath
    {
    public:
    virtual ~CMapObject()
        {
        }

    /**
    Creates a copy of a map object.
    The base implementation works for all types of map object except array map objects.
    */
    virtual std::unique_ptr<CMapObject> Copy(TResult& aError) const;
    
    /**
    Returns all the string attributes, null-separated, as key-value pairs separated
    by an equals sign. The first key must be empty, and means the label or name.
    */
    virtual TText StringAttributes() const = 0;
    
    /** Returns the default label or name of the object: the first, unnamed, string attribute. */
    virtual TText Label() const = 0;

    /** Returns a writable interface to the contour indexed by aIndex. */
    virtual TWritableContour WritableContour(size_t aIndex) = 0;

    /** Returns a pointer to writable string attributes if possible; used by makemap. */
    virtual MString* WritableStringAttributes() { return nullptr; }

    /**
    If the object contains array data that can be represented as
    a bitmap, returns a pointer to the bitmap.
    The base implementation returns null.
    */
    virtual const TBitmap* Bitmap() const;
    /**
    If the object contains array data that can be represented as
    a bitmap, returns a transform to convert from the bitmap bounds
    to the extent in map coordinates.
    The base implementation returns null.
    */
    virtual const TTransform* BitmapTransform() const;
    /**
    Normalizes a map object if possible by removing point contours with no points,
    polygon contours with fewer than 3 points, and line contours with fewer than 2 points.
    The base implementation does nothing.
    */
    virtual void Normalize();
    /**
    Simplifies a map object, if possible, by re-sampling it at a specified resolution.
    The base implementation does nothing.
    */
    virtual void Simplify(int32_t aResolution);
    /**
    Splits a line object into objects with no more than aMaxPoints points each.
    Puts any extra objects created into aExtraObjectArray.
    */
    void Split(size_t aMaxPoints,CMapObjectArray& aExtraObjectArray);
 
    /** Returns a pointer to a TAttribSet if possible; used by makemap. */
    virtual TAttribSet* AttribSet() { return nullptr; }
    /** Returns a constant pointer to a TAttribSet if possible; used by makemap. */
    const TAttribSet* AttribSet() const { return const_cast<CMapObject*>(this)->AttribSet(); }

    /** Returns the projection used to convert from degrees of latitude and longitude to the map units used by this object, or null if the projection is not known. */
    virtual std::shared_ptr<CProjection> Projection() const { return nullptr; }

    /** Gets the value of the string attribute with name aName. */
    TText GetStringAttribute(const MString& aName) const
        {
        return StringAttributes().GetAttribute(aName);
        };
    
    /** Gets the value of the string attribute with name aName. */
    TText GetStringAttribute(const CString& aName) const
        {
        return StringAttributes().GetAttribute(aName);
        }
    
    /**
    Gets a string attribute for a given locale by appending a colon then the locale to aName.
    If no such attribute is available use the locale truncated to two letters if it was longer,
    then try the plain name. The empty attribute name is changed to 'name' if a locale is added.
    */
    TText GetStringAttributeForLocale(const MString& aName,const char* aLocale) const;
    
    /**
    Gets a string attribute for a given locale by appending a colon then the locale to aName.
    If no such attribute is available use the locale truncated to two letters if it was longer,
    then try the plain name. The empty attribute name is changed to 'name' if a locale is added.
    */
    TText GetStringAttributeForLocale(const CString& aName,const char* aLocale) const
        {
        return GetStringAttributeForLocale((const MString&)aName,aLocale);
        }

    /**
    Gets a string attribute from a list of semi-colon-separated names in aPath.
    Replaces empty names with aDefault (as for example if aPath is empty and aDefault is 'name:es').
    Returns the first non-empty result.

    Returns true if the returned attribute is a name attribute, defined as the empty attribute,
    or 'name', or any attribute starting with 'name:'.
    */
    bool GetStringAttributeUsingPath(const MString& aPath,const MString& aDefault,TText& aValue) const;

    /** Gets all the string attributes as a std::map from names to values. */
    std::map<CString,CString> GetStringAttributes() const;

    /**	
    Returns the identifier of this object.
    Identifiers are opaque 64-bit numbers that are specific to a database.
    The value zero is reserved and means 'no identifier'.
    */
    uint64_t Id() const { return iId; }

    /** Returns the type of the object. */
    TMapObjectType Type() const { return iType; }
    
    /** Returns the integer attribute. */
    uint32_t IntAttribute() const { return iIntAttribute; }

    /** Returns the name of the layer this object belongs to. */
    const MString& LayerName() const { return *iLayer; }

    /** Returns the reference-counted name of the layer. */
    CRefCountedString LayerRef() const { return iLayer; }

    /**
    Creates a generic name for a map object, using the supplied locale if possible.
    For example "restaurant", "fuel station" or "minor road".

    Note: locales are not yet implemented. All category names are in English.
    */
    CString GenericName(const char* aLocale = nullptr) const;

    /**
    Creates a UTF-8 string describing a map object; used for debugging.
    The string consists of the object's string attributes, separated by | characters,
    then the layer, OSM type and object type in square brackets. The object type
    is encoded as a single character: . - @ # for point, line, polygon and array.
    */
    std::string Description() const;
    
    /** Creates a verbose English-language description of a map object as a UTF-8 string. */
    std::string VerboseDescription() const;

    /**
    Returns the center of a map object in map coordinates.
    The center is defined as the center of gravity for polygons and arrays,
    a point half way along the path for lines, and the point itself for points.
    */
    TPointFP Center() const;

    /**
    Returns the center of a map object in degrees.
    The center is defined as the center of gravity for polygons and arrays,
    a point half way along the path for lines, and the point itself for points.
    Because the center is calculated using map coordinates then converted to degrees,
    it depends on the map projection used by the map object.
    */
    TPointFP CenterInDegrees(TResult& aError) const;

    /** Returns the bounding box of a map object in degrees. */
    TRectFP BoundsInDegrees(TResult& aError) const;

    /** Returns the geometry of a map object in degrees. */
    CGeometry GeometryInDegrees(TResult& aError) const;

    /**
    Gets the height of a point in metres if this map object contains terrain heights.
    The argument aPoint is in map coordinates.
    The return value is true if a height is successfully obtained.

    There is no check for the validity or sanity of the data. The argument aHaveMetres
    indicates whether the object contains 16-bit metre values, or bytes encoding feet.

    Feet are encoded as follows: 0 for unknown heights (sea or unspecified);
    1...15 for -1400ft to sea level in intervals of 100ft;
    16...195 for 100ft to 18,000ft in intervals of 100ft;
    and 196...255 for 18,200ft to 30,000ft in intervals of 200ft.

    If aInverseTransform is supplied, it is the inverse of the transform
    used to convert from bitmap coordinates to map coordinates. If it is not
    supplied, it is calculated from the bitmap.
    */
    bool GetHeight(TPoint aPoint,bool aHaveMetres,const TTransform* aInverseTransform,int32_t& aHeight) const;
    /** Returns the geocode type of a map object. */
    TGeoCodeType GetGeoCodeType() const;
    /** Gets information used when geocoding a map object. */
    void GetGeoCodeItem(CGeoCodeItem& aGeoCodeItem,const char* aLocale) const;
    /**
    If the string attribute '_s' exists, treats it as a maximum speed
    represented according to OpenStreetMap conventions and returns
    the value in kilometres per hour.

    The speed limit is stored as either a plain number denoting kph,
    a number followed by 'mph' denoting miles per hour,
    or a country code followed by a category as defined in
    http://wiki.openstreetmap.org/wiki/Speed_limits.
    */
    double MaxSpeed() const;
    /**
    Gets the top of an object in metres by reading it
    from the _t attribute. Return 0 if no top is specified.
    The _t attribute is a copy of the OpenStreetMap 'height' attribute.
    */
    double Top() const;
    /**
    Gets the bottom of an object in metres by reading it
    from the _b attribute. Return 0 if no bottom is specified.
    The _b attribute is a copy of the OpenStreetMap 'min_height' attribute.
    */
    double Bottom() const;
    /** Gets the value of a OpenStreetMap dimension attribute in meters. */
    double DimensionAttribute(const MString& aKey) const;
    /**
    Gets the next string attribute, starting at character position aPos in the string attributes,
    and updates aPos. Returns true if an attribute is found.

    This function can be used to enumerate string attributes by passing 0 as aPos and repeating
    until a value of false is returned.
    */
    bool NextStringAttribute(size_t& aPos,TText& aName,TText& aValue) const;
    /** Returns true if this map object can be merged with aOther. */
    bool Mergeable(const CMapObject& aOther) const;
    /** Projects all the points in a map object to 64ths of pixels using the specified projection. */
    void Project(const TMapTransform& aProjection);
    /** Projects all the points in a map object using the specified projection. */
    void Project(const CProjection& aProjection);
    /** Smooths a map object by converting sequences of straight lines to curves. */
    void Smooth();
    /** Offsets all points in the map object by the specified amounts. */
    void Offset(int32_t aDx,int32_t aDy);
    /**
    Clips a map object to the rectangle aClip.

    If the object was clipped the function returns true and puts the clipped object, which may be null, in aClippedObject.

    If aExtraClippedObjectArray is non-null, line objects with a single contour may be clipped into multiple objects to preserve their single-line nature,
    in which case the extra objects are appended to aExtraClippedObjectArray. All other objects are clipped to a single object.

    If aLinesMustBeSingleContours is true, line objects are never converted to multiple contours, even if aExtraClippedObjectArray is non-null. They are not clipped
    if that is necessary.

    If the object was not clipped (i.e., it was entirely contained in aClip) the function returns false.
    */
    bool Clip(const TRect& aClip,std::unique_ptr<CMapObject>& aClippedObject,CMapObjectArray* aExtraClippedObjectArray = nullptr,bool aLinesMustBeSingleContours = false) const;
    /**
    Clips a map object to the clip region aClip.

    If the object was clipped the function returns true and puts the clipped object, which may be null, in aClippedObject.

    If aExtraClippedObjectArray is non-null, line objects with a single contour may be clipped into multiple objects to preserve their single-line nature,
    in which case the extra objects are appended to aExtraClippedObjectArray. All other objects are clipped to a single object.

    If aLinesMustBeSingleContours is true, line objects are never converted to multiple contours, even if aExtraClippedObjectArray is non-null. They are not clipped
    if that is necessary.

    If the object was not clipped (i.e., it was entirely contained in aClip) the function returns false.
    */
    bool Clip(const TClipRegion& aClip,std::unique_ptr<CMapObject>& aClippedObject,CMapObjectArray* aExtraClippedObjectArray = nullptr,bool aLinesMustBeSingleContours = false) const;
    using MPath::Clip;
    /**
    Returns a version of a map object with all curved line segments
    flattened by being replaced by short straight line segments.
    Returns null if no flattening is needed.
    */
    std::unique_ptr<CMapObject> Flatten(double aDistance) const;

    /**
    Creates a new map object, interpolating extra points so that the maximum difference between
    coordinates doesn't exceed aGranularity. Chooses the points to
    be interpolated using the coordinate modulo aGranularity, so
    that adjacent polygons have the same interpolated points and thus
    meet without gaps.
    */
    std::unique_ptr<CMapObject> Interpolate(int32_t aGranularity) const;

    /** Sets the map object type. */
    void SetType(TMapObjectType aType) { iType = aType; }
    /**
    Writes a map object to an output stream in serialized form.
    Writes the geometry as degrees lat/long.
    (For internal use only : if the map object doesn't store its projection, which is true of stack-allocated CTM1 objects, the projection must be supplied.)
    */
    TResult Write(TDataOutputStream& aOutput,const CProjection* aProjectionFromLatLong = nullptr) const;
    /**
    Calculates the area of a map object in square meters, using the projection stored in the map object.
    Returns 0 if the object is a point or line object.
    */
    double Area(TResult& aError) const;
    /**
    Calculates the length or perimeter of a map object in meters, using the projection stored in the map object.
    Returns 0 if the object is a point object.
    */
    double LengthOrPerimeter(TResult& aError) const;
    /** Sets the ID. */
    void SetId(uint64_t aId) { iId = aId; }
    /** Sets the 32-bit integer attribute, which contains the road type if the object is a road. */
    void SetIntAttribute(uint32_t aValue) { iIntAttribute = aValue; }
    /** Sets the layer. */
    void SetLayer(CRefCountedString aLayer) { iLayer = aLayer; }

    /** Information returned by GetMatch. */
    class CMatch
        {
        public:
        /** The name of the attribute in which the matched text was found. */
        CString iKey;
        /** The value of the attribute in which the matched text was found. */ 
        CString iValue;
        /** The start position of the matched text within the value. */
        size_t iStart;
        /** The end position of the matched text within the value. */
        size_t iEnd;
        };
    
    /**
    Finds the first string attribute matching aText, using aMatchMethod, and returns
    information about it in aMatch.
    If aAttributes is non-null, examines only attributes in the supplied comma-separated list,
    otherwise examines all attributes except those starting with an underscore, which by convention are non-textual.
    Attribute names may contain the wild cards ? and *.
    If aPhrase is true (the default), allows matches of phrases within the attribute.
    */
    TResult GetMatch(CMatch& aMatch,const MString& aText,TStringMatchMethod aMatchMethod,const MString* aAttributes = nullptr,bool aPhrase = true) const;

    /** A type for different qualities of text match. */
    enum class TMatchType
        {
        /** No match. */
        None,
        /** A substring of the text matches, but the substring is not aligned to token boundaries. */
        Substring,
        /** There is a fuzzy match: a small number of extra characters, missing characters, or transpositions. */
        Fuzzy,
        /** There is a phrase match: a substring match aligned to token boundaries. */
        Phrase,
        /** A full match, ignoring letter case and accents. */
        Full
        };

    /**
    Finds out whether a map object is a partial or full match for some text.

    If the text matches the whole of a name attribute, or
    the whole of a sub-part of a name attribute (using spaces, hyphens, single quotes, colons, semicolons and slashes as separators)
    ignoring case and accents, it's a phrase match or a fuzzy match.

    If it matches part of the attribute it's a phrase match.
    */
    TMatchType MatchType(const MString& aText) const;

    /** A type for arbitrary user-defined data associated with a map object. */
    union TUserData
        {
        TUserData() { memset(this,0,sizeof(TUserData)); }
        /** The integer value. */
        int64_t iInt;
        /** The pointer value. */
        void* iPtr;
        };

    /** Sets the user data to a 64-bit integer. */
    void SetUserData(int64_t aData) { iUserData.iInt = aData; }
    /** Returns the user data as a 64-bit integer. */
    int64_t UserData() const { return iUserData.iInt; }
    /** Sets the user data to a pointer. */
    void SetUserDataToPointer(void* aData) { iUserData.iPtr = aData; }
    /** Returns the user data as a pointer. */
    void* UserDataAsPointer() const { return iUserData.iPtr; }
    /** Sets the user data to a union */
    void SetUserDataToUnion(TUserData aUserData) { iUserData = aUserData; }
    /** Returns the user data as a union. */
    TUserData UserDataAsUnion() const { return iUserData; }

    private:
    bool ClipHelper(COutline& aClippedOutline,std::unique_ptr<CMapObject>& aClippedObject,CMapObjectArray* aExtraClippedObjectArray,bool aLinesMustBeSingleContours) const;

    protected:
    /** Creates a map object with a specified layer and type. */
    CMapObject(CRefCountedString aLayer,TMapObjectType aType):
        iLayer(aLayer),
        iType(aType)
        {
        assert(aLayer);
        }
    /** Creates a map object with a specified layer, type and user data. */
    CMapObject(CRefCountedString aLayer,TMapObjectType aType,TUserData aUserData):
        iLayer(aLayer),
        iType(aType),
        iUserData(aUserData)
        {
        assert(aLayer);
        }
    /** Creates a map object by copying another one. */
    CMapObject(const CMapObject& aOther) = default;
    /** The assignment operator. */
    CMapObject& operator=(const CMapObject& aOther) = default;

    /** The layer this object belongs to. */
    CRefCountedString iLayer;
    /** The object's identifier. */
    uint64_t iId = 0;
    /** The type of the object: point, line, polygon, etc. */
    TMapObjectType iType;
    /** The integer attribute, used for road types, routing flags, or sub-types of layers. */
    uint32_t iIntAttribute = 0;
    /** User data; can hold an integer or a pointer; can be used to associate any value with a map object. */
    TUserData iUserData;
    };

/**
Constructs an integer attribute from an optional three-letter code, placed
in the high 16 bits by encoding each letter in five bits, and a type number placed in
the low 16 bits.

The three-letter code is used only if it consists of exactly three lower-case letters
in the range a...z.

Three-letter codes are used for mnemonic purposes, as for example "pub" for a public house,
and can be tested easily in style sheets.
*/
inline int32_t IntAttribute(const char* aThreeLetterCode,int aType)
    {
    int32_t a = 0;
    if (aThreeLetterCode &&
        strlen(aThreeLetterCode) == 3 &&
        aThreeLetterCode[0] >= 'a' && aThreeLetterCode[0] <= 'z' &&
        aThreeLetterCode[1] >= 'a' && aThreeLetterCode[1] <= 'z' &&
        aThreeLetterCode[2] >= 'a' && aThreeLetterCode[2] <= 'z')
        a = ((((aThreeLetterCode[0])-'a') << 27) | (((aThreeLetterCode[1])-'a') << 22) | (((aThreeLetterCode[2])-'a') << 17));
    a |= (aType & 0xFFFF);
    return a;
    }

/** A comparison function which compares unique pointers to map objects on their user data. Neither pointer can be null. This function can be used in std::sort, etc. */
inline bool MapObjectUserDataLessThan(const std::unique_ptr<CMapObject>& aP,const std::unique_ptr<CMapObject>& aQ)
    {
    return aP->UserData() < aQ->UserData();
    }

}

#endif
