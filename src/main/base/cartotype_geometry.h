/*
cartotype_geometry.h
Copyright (C) CartoType Ltd 2019-2021.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_GEOMETRY__
#define CARTOTYPE_GEOMETRY__

#include <cartotype_path.h>

namespace CartoType
{

class CMapObject;

/** A template class to hold geometry objects containing various types of point. The point type must be derived from or trivially convertible to TOutlinePointFP. */
template<class point_t> class CGeneralGeometry
    {
    public:
    /** The type for contours: an array of points. */
    using contour_t = std::vector<point_t>;

    /** Creates an empty open geometry object using map coordinates. */
    CGeneralGeometry() { }
    /** Creates a geometry object with given coordinates and open/closed status. */
    explicit CGeneralGeometry(TCoordType aCoordType,bool aClosed = false):
        m_coord_type(aCoordType),
        m_closed(aClosed)
        {
        }
    /**  Creates a geometry object by copying a path, using given coordinates and open/closed status. Ignores the open/closed status of individual contours in the path. */
    CGeneralGeometry(const MPath& aPath,TCoordType aCoordType,bool aClosed):
        m_coord_type(aCoordType),
        m_closed(aClosed)
        {
        for (size_t i = 0; i < aPath.Contours(); i++)
            {
            TContour contour = aPath.Contour(i);
            BeginContour();
            auto& c = m_contour_array.back();
            for (const auto p : contour)
                c.push_back(point_t(p));
            }
        }
    /** Creates a closed geometry object from an axis-aligned rectangle. */
    CGeneralGeometry(const TRectFP& aRect,TCoordType aCoordType):
        m_coord_type(aCoordType),
        m_closed(true)
        {
        AppendPoint(aRect.iTopLeft);
        AppendPoint(aRect.BottomLeft());
        AppendPoint(aRect.iBottomRight);
        AppendPoint(aRect.TopRight());
        }
    /** Creates an open geometry object containing a single point. */
    CGeneralGeometry(const point_t& aPoint,TCoordType aCoordType):
        m_coord_type(aCoordType),
        m_closed(false)
        {
        AppendPoint(aPoint);
        }
    /** Creates a geometry object representing a single contour of another geometry object. */
    CGeneralGeometry(const CGeneralGeometry& aOther,size_t aContourIndex): CGeneralGeometry(aOther.m_coord_type,aOther.m_closed) { m_contour_array[0] = aOther.m_contour_array[aContourIndex]; }

    /** Returns a COutline object containing this geometry. */
    operator COutline() const
        {
        COutline outline;
        TOutlinePoint p;
        for (const auto& contour : m_contour_array)
            {
            CContour& c = outline.AppendContour();
            c.SetClosed(m_closed);
            for (const auto& point: contour)
                {
                TOutlinePointFP q { point };
                if (q.iX < INT32_MIN || q.iX > INT32_MAX || q.iY < INT32_MIN || q.iY > INT32_MAX)
                    throw KErrorOverflow;
                p.iX = Round(q.iX);
                p.iY = Round(q.iY);
                p.iType = q.iType;
                c.AppendPointEvenIfSame(p);
                }
            }
        return outline;
        }

    /** Deletes all points, leaving the geometry with a single empty contour and its original coordinate type and open/closed state. */
    void Clear() { m_contour_array.resize(1); m_contour_array[0].resize(0); }
    /** Returns the coordinate type used for all points in this geometry. */
    TCoordType CoordType() const { return m_coord_type; }
    /** Returns the number of contours. */
    size_t ContourCount() const { return m_contour_array.size(); }
    /** Returns a contour selected by its index as a vector of points. */
    const contour_t& Contour(size_t aIndex) const { return m_contour_array[aIndex]; }
    /** Returns true if a contour is an outer contour, defined as being anti-clockwise, that is, having a positive area when treating all points as on-curve. Useful only for closed geometries. */
    bool ContourIsOuter(size_t aIndex) const { return Area(m_contour_array[aIndex]) > 0; }
    /** Returns the number of points in a given contour. */
    size_t PointCount(size_t aContourIndex) const { return m_contour_array[aContourIndex].size(); }
    /** Returns a point identified by its contour index and point index. */
    const point_t& Point(size_t aContourIndex,size_t aPointIndex) const { return m_contour_array[aContourIndex][aPointIndex]; }
    /** Returns a non-const reference to a point identified by its contour index and point index. */
    point_t& Point(size_t aContourIndex,size_t aPointIndex) { return m_contour_array[aContourIndex][aPointIndex]; }
    /** Returns true if the geometry is empty. */
    bool IsEmpty() const { return m_contour_array[0].empty(); }
    /** Returns true if the geometry is closed (formed of closed paths). */
    bool IsClosed() const { return m_closed; }
    /** Sets the open/closed status. */
    void SetClosed(bool aClosed) { m_closed = aClosed; }
    /** Returns a writable coordinate set referring to this geometry. */
    TWritableCoordSet CoordSet(size_t aContourIndex)
        {
        return TWritableCoordSet(m_contour_array[aContourIndex]);
        }
    /** Returns a coordinate set referring to this geometry. */
    TCoordSet CoordSet(size_t aContourIndex) const
        {
        return TCoordSet((const contour_t&)m_contour_array[aContourIndex]);
        }
    /** Returns the bounds as an axis-aligned rectangle. */
    TRectFP Bounds() const
        {
        TRectFP bounds;
        if (m_contour_array[0].size())
            bounds.iTopLeft = bounds.iBottomRight = m_contour_array[0][0];
        for (const auto& c : m_contour_array)
            for (const auto& p : c)
                bounds.Combine(p);
        return bounds;
        }
    /** Appends a point to the current (last) contour in this geometry. */
    void AppendPoint(const point_t& aPoint) { m_contour_array.back().push_back(aPoint); }
    /** Appends a point to the last contour in this geometry. */
    void AppendPoint(double aX,double aY) { m_contour_array.back().push_back(point_t(aX,aY)); }
    /** Appends a point to the last contour in this geometry, specifying the point type. */
    void AppendPoint(double aX,double aY,TPointType aPointType) { m_contour_array.back().push_back(point_t(aX,aY,aPointType)); }
    /** Starts a new contour. Does nothing if the current (last) contour is empty. */
    void BeginContour() { if (!m_contour_array.back().empty()) m_contour_array.emplace_back(); }
    /** Reverses the order of the contours and the order of the points in each contour. */
    void Reverse()
        {
        std::reverse(m_contour_array.begin(),m_contour_array.end());
        for (auto& c : m_contour_array)
            std::reverse(c.begin(),c.end());
        }
    /** Converts the coordinates to aToCoordType using the function aConvertFunction. For internal use only. */
    void ConvertCoords(TCoordType aToCoordType,std::function<void(TWritableCoordSet& aCoordSet)> aConvertFunction)
        {
        if (m_coord_type == aToCoordType)
            return;
        m_coord_type = aToCoordType;
        size_t contour_count = ContourCount();
        for (size_t i = 0; i < contour_count; i++)
            {
            TWritableCoordSet cs { CoordSet(i) };
            aConvertFunction(cs);
            }
        }

    private:
    std::vector<contour_t> m_contour_array = std::vector<contour_t>(1);
    TCoordType m_coord_type = TCoordType::Map;
    bool m_closed = false;
    };

/**
A geometry class for creating map objects and specifying view areas.

There are also functions for the determining distance and intersection type between a geometry
object and map objects. Those functions do not work with geometry objects in display coordinates, or map objects with no
known projection, for which they give a path intersection type of Unknown. If the geometry is in map
units or map meters the caller is responsible for ensuring that the units are the same as those
of the map object or objects.
*/
class CGeometry: public CGeneralGeometry<TOutlinePointFP>
    {
    public:
    using CGeneralGeometry::CGeneralGeometry; // inherit constructors

    CGeometry() = default;
    explicit CGeometry(const CMapObject& aMapObject);
    
    TPathIntersectionType IntersectionType(const CMapObject& aMapObject) const;
    TPathIntersectionInfo IntersectionInfo(const CMapObject& aMapObject) const;
    std::vector<TPathIntersectionType> IntersectionType(std::function<const CMapObject*(size_t)> aObject,size_t aObjectCount) const;
    std::vector<TPathIntersectionType> IntersectionType(const CMapObjectArray& aMapObjectArray) const;
    std::vector<TPathIntersectionInfo> IntersectionInfo(std::function<const CMapObject* (size_t)> aObject,size_t aObjectCount) const;
    std::vector<TPathIntersectionInfo> IntersectionInfo(const CMapObjectArray& aMapObjectArray) const;
    /** Returns true if this geometry intersects, contains or is contained by the map object aMapObject. */
    bool Intersects(const CMapObject& aMapObject) const
        {
        auto t = IntersectionType(aMapObject);
        return t != TPathIntersectionType::Separate && t != TPathIntersectionType::Unknown;
        }
    /** Returns true if this geometry contains the map object aMapObject. */
    bool Contains(const CMapObject& aMapObject) const { return IntersectionType(aMapObject) == TPathIntersectionType::Contains; }
    /** Returns true if this geometry is contained by the map object aMapObject. */
    bool Contained(const CMapObject& aMapObject) const { return IntersectionType(aMapObject) == TPathIntersectionType::Contained; }
    CGeometry Clip(TClipOperation aClipOperation,const CMapObject& aMapObject) const;
    std::vector<CGeometry> Clip(TClipOperation aClipOperation,const CMapObjectArray& aMapObjectArray) const;
    std::vector<CGeometry> Clip(TClipOperation aClipOperation,std::function<const CMapObject* (size_t)> aObject,size_t aObjectCount) const;
    static CGeometry Envelope(const CMapObject& aMapObject,double aOffsetInMeters);

    private:
    /** Creates an outline in map units from this geometry using aProjection. */
    COutline ConvertToMapUnits(TResult& aError,const CProjection* aProjection) const;
    /** Create a geometry object in degrees from an path in map units, using aProjection. */
    static CGeometry FromMapUnits(const MPath& aPath,bool aClosed,const CProjection* aProjection);
    };

} // namespace CartoType

#endif // #define CARTOTYPE_GEOMETRY__
