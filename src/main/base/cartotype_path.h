/*
cartotype_path.h
Copyright (C) 2013-2020 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_PATH_H__
#define CARTOTYPE_PATH_H__

#include <cartotype_types.h>
#include <cartotype_stream.h>
#include <algorithm>
#include <utility>

namespace CartoType
{
class MContour;
class CContour;
class TContour;
class COutline;
class CEngine;
class CMapObject;
class TCircularPen;
class CProjection;
class TClipRegion;
class TTransform;

/** A type to label different relationships a clip rectangle has with a path, to decide what sort of clipping is needed. */
enum class TClipType
    {
    /** The path is completely inside the clip rectangle. */
    Inside,
    /** The path is not completely inside the clip rectangle, and has no curves. */
    MayIntersectAndHasNoCurves,
    /** The path is not completely inside the clip rectangle, and has curves. */
    MayIntersectAndHasCurves
    };

/** Types of clipping done by the general clip function COutline MPath::Clip(TClipOperation aClipOperation,const MPath& aClip) const. */
enum class TClipOperation // must be semantically the same as ClipperLib::ClipType
    {
    /** Returns the intersection of two paths; commutative. */
    Intersection,
    /** Returns the union of two paths; commutative. */
    Union,
    /** Returns the difference of two paths; non-commutative. */
    Difference,
    /** Returns the exclusive-or of the two paths; that is, any regions which are in neither path; commutative. */
    Xor
    };

/**
Traverses a sequence of TPoint objects, extracting lines and curves and calling functions to process them.

The functions called are

To start a new contour, move to aPoint without drawing a line and set the current point to aPoint:

void MoveTo(const TPoint& aPoint)

To draw a line from the current point to aPoint and set the current point to aPoint:

void LineTo(const TPoint& aPoint)
*/
template<class MTraverser> void Traverse(MTraverser& aTraverser,const TPoint* aPoint,size_t aPoints,bool aClosed)
    {
    if (aPoints < 2)
        return;

    const TPoint* point = aPoint;
    const TPoint* last = aPoint + aPoints - 1;
    aTraverser.MoveTo(*point++);
    while (point <= last)
        aTraverser.LineTo(*point++);
    if (aClosed && *aPoint != *last)
        aTraverser.LineTo(*aPoint);
    }

/**
Traverses a sequence of outline point objects, extracting lines and curves and calling functions to process them.

The functions called are

To start a new contour, move to aPoint without drawing a line and set the current point to aPoint:

void MoveTo(const TPoint& aPoint)

To draw a line from the current point to aPoint and set the current point to aPoint:

void LineTo(const TPoint& aPoint)

To draw a quadratic spline from the current point to aPoint2, using aPoint1 as the off-curve control point,
and set the current point to aPoint2:

void QuadraticTo(const TPoint& aPoint1,const TPoint& aPoint2)

To draw a cubic spline from the current point to aPoint3, using aPoint1 and aPoint2 as the
off-curve control points, and set the current point to aPoint3:

void CubicTo(const TPoint& aPoint1,const TPoint& aPoint2,const TPoint& aPoint3)
*/
template<class traverser_t,class point_t> void Traverse(traverser_t& aTraverser,const point_t* aPoint,size_t aPoints,bool aClosed)
    {
    if (aPoints < 2)
        return;

    // A contour cannot start with a cubic control point.
    assert(aPoint->iType != TPointType::Cubic);

    size_t last = aPoints - 1;
    const point_t* limit = aPoint + last;
    point_t v_start = aPoint[0];
    point_t v_last = aPoint[last];
    const point_t* point = aPoint;

    /* Check first point to determine origin. */
    if (point->iType == TPointType::Quadratic)
        {
        /* First point is conic control. Yes, this happens. */
        if (aPoint[last].iType == TPointType::OnCurve)
            {
            /* start at last point if it is on the curve */
            v_start = v_last;
            limit--;
            }
        else
            {
            /*
            If both first and last points are conic,
            start at their middle and record its position
            for closure.
            */
            v_start.iX = (v_start.iX + v_last.iX) / 2;
            v_start.iY = (v_start.iY + v_last.iY) / 2;
            v_last = v_start;
            }
        point--;
        }

    aTraverser.MoveTo(v_start);

    while (point < limit)
        {
        point++;
        switch (point->iType)
            {
            case TPointType::OnCurve:
                aTraverser.LineTo(*point);
                continue;
                
            case TPointType::Quadratic:
                {
                const point_t* v_control = point;
                while (point < limit)
                    {
                    point++;
                    const point_t* cur_point = point;
                    if (point->iType == TPointType::OnCurve)
                        {
                        aTraverser.QuadraticTo(*v_control,*cur_point);
                        break;
                        }
                    if (point->iType != TPointType::Quadratic)
                        return; // invalid outline
                    point_t v_middle((v_control->iX + cur_point->iX) / 2,(v_control->iY + cur_point->iY) / 2);
                    aTraverser.QuadraticTo(*v_control,v_middle);
                    v_control = cur_point;
                    }
                if (aPoint == limit)
                    {
                    aTraverser.QuadraticTo(*v_control,v_start);
                    return;
                    }
                }
                continue;

            default: // cubic
                {
                if (point + 1 > limit || point->iType != TPointType::Cubic)
                    return; // invalid outline
                const point_t* vec1 = point++;
                const point_t* vec2 = point++;
                if (point <= limit)
                    {
                    aTraverser.CubicTo(*vec1,*vec2,*point);
                    continue;
                    }
                aTraverser.CubicTo(*vec1,*vec2,v_start);
                return;
                }
            }
        }

    // Close the contour with a line segment.
    if (aClosed && v_last != v_start)
        aTraverser.LineTo(v_start);
    }

/** An interface class to transform a point. */
class MPointTransformer
    {
    public:
    /** A virtual function to transform a point representing pixels using integers with a certain number of fractional bits. */
    virtual TDrawResult Transform(TPoint& aPoint,int32_t aFractionalBits) = 0;
    };

/** The ways two paths can intersect. */
enum class TPathIntersectionType
    {
    /** The intersection type is unknown. */
    Unknown,
    /** The paths do not intersect. */
    Separate,
    /* The paths intersect or are identical. */
    Intersects,
    /** The first path contains the second. */
    Contains,
    /** The second path contains the first. */
    Contained
    };

/** Information about the intersection of two paths and their distance apart. */
class TPathIntersectionInfo
    {
    public:
    /** The intersection type. */
    TPathIntersectionType iType = TPathIntersectionType::Unknown;
    /**
    The distance between the paths. The units are meters for functions involving
    map objects, otherwise they are the units used by the paths.
    */
    double iDistance = 0;
    /** 
    The nearest point on the first path. The units are latitude and longitude for functions involving
    map objects, otherwise they are the units used by the paths.
    */
    TPointFP iNearest1;
    /**
    The nearest point on the second path. The units are latitude and longitude for functions involving
    map objects, otherwise they are the units used by the paths.
    */
    TPointFP iNearest2;
    };

/**
Path objects, which are sequences of contours, must implement the
MPath interface class.
*/
class MPath
    {
    public:
    /**
    A virtual destructor: needed in case paths returned by ClippedPath
    are not the same as the path passed in and must therefore be deleted by the caller.
    */
    virtual ~MPath() { }

    /** Returns the number of contours. */
    virtual size_t Contours() const = 0;

    /** Returns the contour indexed by aIndex. */
    virtual TContour Contour(size_t aIndex) const = 0;

    /** Returns true if the path may have off-curve points. */
    virtual bool MayHaveCurves() const = 0;

    template<class MPathTraverser> void Traverse(MPathTraverser& aTraverser,const TRect& aClip) const;
    template<class MPathTraverser> void Traverse(MPathTraverser& aTraverser,const TRect* aClip = nullptr) const;

    bool operator==(const MPath& aOther) const;
    TRect CBox() const;
    bool CBoxBiggerThan(int32_t aSize) const;
    bool IsContainedIn(const TRect& aRect) const;
    bool Contains(double aX,double aY) const;
    /** Returns true if this path contains aPoint. */
    bool Contains(const TPoint& aPoint) const { return Contains(aPoint.iX,aPoint.iY); }
    /** Returns true if this path contains aPoint. */
    bool Contains(const TPointFP& aPoint) const { return Contains(aPoint.iX,aPoint.iY); }
    /** Returns true if this path contains aPath. */
    bool MayIntersect(const TRect& aRect) const;
    bool MayIntersect(const TRect& aRect,int32_t aBorder) const;
    bool Intersects(const TRect& aRect) const;
    bool Intersects(const MPath& aPath,const TRect* aBounds = nullptr) const;
    int32_t MaxDistanceFromOrigin() const;
    TPathIntersectionType IntersectionType(const MPath& aOther,double* aDistance = nullptr,TPointFP* aNearest1 = nullptr,TPointFP* aNearest2 = nullptr) const;
    TPathIntersectionInfo IntersectionInfo(const MPath& aOther) const;
    double DistanceFrom(const MPath& aOther,TPointFP* aNearest1 = nullptr,TPointFP* aNearest2 = nullptr,TPathIntersectionType* aType = nullptr) const;
    double DistanceFromPoint(const TPointFP& aPoint,TPointFP* aNearest = nullptr,size_t* aContourIndex = nullptr,size_t* aLineIndex = nullptr,double* aFractionaLineIndex = nullptr,bool aTreatAsOpen = false) const;
    bool IsClippingNeeded(const TRect& aClip) const;
    COutline Copy() const;
    COutline ClippedPath(const TRect& aClip) const;
    COutline ClippedPath(const MPath& aClip) const;
    COutline ClippedPath(const TClipRegion& aClip) const;
    COutline Clip(TClipOperation aClipOperation,const MPath& aClip) const;
    COutline Envelope(double aOffset) const;
    bool IsSmoothingNeeded() const;
    COutline SmoothPath() const;
    COutline FlatPath(double aMaxDistance) const;
    COutline TruncatedPath(double aStart,double aEnd) const;
    COutline OffsetPath(double aOffset) const;
    COutline TransformedPath(const TTransform& aTransform) const;
    COutline TransformedPath(TDrawResult& aError,MPointTransformer& aTransformer,int32_t aFractionalBits) const;
    std::array<TLine,3> GetHorizontalPaths(int32_t aPathLength,int32_t aLabelHeight,const TPointFP& aUpVector,const TRect* aBounds,const TRect* aClip) const;
    TPointFP CenterOfGravity() const;
    void GetCenterOfGravity(TPoint& aCenter) const;
    double Length() const;
    double Area() const;
    TPointFP PointAtLength(double aPos) const;
    std::pair<TPoint,bool> End() const;
    void Write(TDataOutputStream& aOutput) const;
    bool IsEmpty() const;
    bool IsPoint() const;
    bool IsGridOrientedRectangle(TRect* aRect = nullptr) const;
    void GetSphericalAreaAndLength(const CProjection& aProjection,double* aArea,double* aLength) const;
    TClipType ClipType(const TRect& aRect) const;
    bool HasCurves() const;

    /** A constant iterator class to iterate over the contours of a path; used by begin() and end(). */
    class ConstIter
        {
        public:
        /** Creates a constant iterator over a path, starting at contour number aIndex. */
        ConstIter(const MPath& aPath,size_t aIndex):
            m_path(aPath),
            m_index(aIndex)
            {
            }
        /** Increments this iterator to refer to the next contour. */
        ConstIter operator++() { m_index++; return *this; }
        /** The inequality operator. */
        bool operator!=(const ConstIter& aOther) const { return m_index != aOther.m_index; }
        TContour operator*() const;

        private:
        const MPath& m_path;
        size_t m_index;
        };

    /** Returns a constant iterator positioned at the start of the contours. */
    ConstIter begin() const { return ConstIter(*this,0); }
    /** Returns a constant iterator positioned at the end of the contours. */
    ConstIter end() const { return ConstIter(*this,Contours()); }

    private:
    double LengthHelper(double aPos,TPointFP* aPoint) const;
    bool SmoothPathHelper(COutline* aNewPath) const;
    };

/**
Flags used for appending ellipses to determine the angle between
start and end point or start and end angle.
*/
enum class TEllipseAngleType
    {
    /** Select the shortest arc between start and end. */
    Shortest,
    /** Select the longest arc between start and end. */
    Longest,
    /** Select the arc from start to end in positive direction. */
    Positive,
    /** Select the arc from start to end in negative direction. */
    Negative
    };

/**
An MContour is an abstract interface to a sequence of integer points representing a line or polygon. The points may be on-curve or off-curve.
Off-curve points are control points for quadratic or cubic spline curves. A contour may be open or closed.
*/
class MContour: public MPath
    {
    public:
    /** Returns the number of points in the contour. */
    virtual size_t Points() const = 0;
    /** Returns a point selected by its index. */
    virtual TOutlinePoint Point(size_t aIndex) const = 0;
    /** Returns true if this is a closed contour. */
    virtual bool Closed() const = 0;

    operator TContour() const;

    // virtual function from MPath
    size_t Contours() const { return 1; };

    /** Returns the last point of the contour. */
    TOutlinePoint LastPoint() const { return Point(Points() - 1); }
    TOutlinePoint FractionalPoint(double aIndex) const;
    bool IsGridOrientedRectangle(TRect* aRect = nullptr) const;
    bool Anticlockwise() const;
    bool Contains(double aX,double aY) const;
    /** Returns true if this contour contains aPoint. */
    bool Contains(const TPoint& aPoint) const { return Contains(aPoint.iX,aPoint.iY); }
    COutline ClippedContour(const TRect& aClip) const;
    void AppendClippedContour(COutline& aDest,const TRect& aClip) const;
    size_t AppendSplitContour(COutline& aDest,const TPointFP& aLineStart,const TPointFP& aLineVector);
    CContour TruncatedContour(double aStart,double aEnd) const;
    CContour SubContourBetweenFractionalPoints(double aStartIndex,double aEndIndex) const;
    CContour SubContourBetweenNearestPoints(const TPointFP* aStartPoint,const TPointFP* aEndPoint) const;
    CContour CentralPath(std::shared_ptr<CEngine> aEngine,const TRect& aClip,bool aFractionalPixels,
                         TLine& aFallbackLine,bool aFallbackMustBeHorizontal) const;
    CContour Smooth(double aRadius) const;
    bool MayIntersect(const TRect& aRect) const;
    bool Intersects(const TRect& aRect) const;
    TPathIntersectionType IntersectionType(const MContour& aOther,double* aDistance = nullptr,TPointFP* aNearest1 = nullptr,TPointFP* aNearest2 = nullptr) const;
    TPathIntersectionInfo IntersectionInfo(const MContour& aOther) const;
    double DistanceFrom(const MContour& aOther,TPointFP* aNearest1 = nullptr,TPointFP* aNearest2 = nullptr,TPathIntersectionType* aType = nullptr) const;
    double DistanceFromPoint(const TPointFP& aPoint,TPointFP* aNearest = nullptr,
                             double* aNearestLength = nullptr,bool* aLeft = nullptr,size_t* aLineIndex = nullptr,
                             double* aFractionalLineIndex = nullptr,
                             bool aTreatAsOpen = false) const;
    TPointFP PointAtLength(double aLength,double aOffset = 0,int32_t* aLineIndex = nullptr) const;
    void GetOrientation(const TPoint& aCenter,TPoint& aOrientation) const;
    void GetPrincipalAxis(TPointFP& aCenter,TPointFP& aVector) const;
    void Write(TDataOutputStream& aOutput) const;
    void GetAngles(double aDistance,double& aStartAngle,double& aEndAngle);

    private:
    void GetLongestLineIntersection(const TPoint& aLineStart,const TPoint& aLineEnd,TPoint& aStart,TPoint& aEnd,const TRect& aClip) const;
    };

/**
A TContour is a concrete class implementing MContour and designed to be returned
by implementations of MPath. 
*/
class TContour: public MContour
    {
    public:
    /** Creates an empty contour. */
    TContour()
       {
       }

    /** Creates a contour from an array of TOutlinePoint objects. */
    TContour(const TOutlinePoint* aPoint,size_t aPoints,bool aClosed):
        m_point(aPoint),
        m_points(aPoints),
        m_closed(aClosed),
        m_has_off_curve_points(true)
        {
        }

    /** Creates a contour from an array of TPoint objects. */
    TContour(const TPoint* aPoint,size_t aPoints,bool aClosed):
        m_point(aPoint),
        m_points(aPoints),
        m_closed(aClosed),
        m_has_off_curve_points(false)
        {
        }

    // Virtual functions from MPath.
    TContour Contour(size_t /*aIndex*/) const override { return *this; }
    bool MayHaveCurves() const override { return m_has_off_curve_points; }

    // virtual functions from MContour
    size_t Points() const override { return m_points; }
    TOutlinePoint Point(size_t aIndex) const override
        {
        assert(aIndex < m_points);
        if (m_has_off_curve_points) return ((const TOutlinePoint*)m_point)[aIndex];
        return TOutlinePoint(m_point[aIndex]);
        }
    bool Closed() const override { return m_closed; }

    /** Returns a sub-contour of this contour, starting at the point indexed by aStart, and containing aPoints points. */
    TContour SubContour(size_t aStart,size_t aPoints)
        {
        assert(aStart <= m_points && aStart + aPoints <= m_points);
        TContour c = *this;
        c.m_points = aPoints;
        if (m_has_off_curve_points)
            c.m_point = (TPoint*)((TOutlinePoint*)m_point + aStart);
        else
            c.m_point += aStart;
        return c;
        }
    /** Returns a pointer to start of the TPoint data if this contour has only on-curve points, otherwise returns null. */
    const TPoint* PointData() const { return m_has_off_curve_points ? nullptr : m_point; }
    /** Returns a pointer to start of the TOutlinePoint data if this contour may have off-curve points, otherwise returns null. */
    const TOutlinePoint* OutlinePointData() const { return m_has_off_curve_points ? (const TOutlinePoint*)m_point : nullptr; }

    template<class MTraverser> void Traverse(MTraverser& aTraverser,const TRect* aClip = nullptr) const;

    /** An iterator to traverse the points of a contour. */
    class Iter
        {
        public:
        /** Creates an iterator over a sequence of points in a contour. */
        Iter(const TPoint* aPoint,size_t aIndex,bool aHasOffCurvePoints):
            m_ptr(aHasOffCurvePoints ? ((TOutlinePoint*)aPoint) + aIndex : aPoint + aIndex),
            m_has_off_curve_points(aHasOffCurvePoints)
            {
            }
        /** Increments this iterator to refer to the next point. */
        Iter& operator++() { if (m_has_off_curve_points) m_ptr = ((TOutlinePoint*)m_ptr) + 1; else m_ptr++; return *this; }
        /** The inequality operator. */
        bool operator!=(const Iter& aOther) const { return m_ptr != aOther.m_ptr; }
        /** Returns a reference to the point to which this iterator refers. */
        TOutlinePoint operator*() const { if (m_has_off_curve_points) return *((TOutlinePoint*)m_ptr); else { return TOutlinePoint(*m_ptr); } }

        private:
        const TPoint* m_ptr = nullptr;
        bool m_has_off_curve_points = false;
        };

    /** Returns a constant iterator positioned at the start of the points. */
    Iter begin() const { return Iter(m_point,0,m_has_off_curve_points); }
    /** Returns a constant iterator positioned at the end of the points. */
    Iter end() const { return Iter(m_point,m_points,m_has_off_curve_points); }

    /** An iterator to traverse the lines (sections between successive points) of a contour. */
    class LineIter
        {
        public:
        /** Creates an iterator over the lines in a contour. */
        LineIter(const TContour& aContour,size_t aIndex):
            m_contour(aContour),
            m_index(aIndex)
            {
            if (m_index == 0)
                {
                if (m_contour.Points() < 2)
                    {
                    m_index = m_contour.Points();
                    return;
                    }
                m_line.iStart = m_contour.LastPoint().Point();
                m_line.iEnd = m_contour.Point(0).Point();
                if (!m_contour.Closed() || m_line.iStart == m_line.iEnd)
                    ++(*this);
                }
            }
        /** Increments this iterator to refer to the next line. */
        LineIter& operator++() { if (++m_index < m_contour.Points()) { m_line.iStart = m_line.iEnd; m_line.iEnd = m_contour.Point(m_index).Point(); } return *this; }
        /** The inequality operator. */
        bool operator!=(const LineIter& aOther) const { return m_index != aOther.m_index; }
        /** Returns a reference to the line to which this iterator refers. */
        const TLine& operator*() const { return m_line; }
        /** Returns a pointer to the line to which this iterator refers. */
        const TLine* operator->() const { return &m_line; }
        /** Returns the number of lines remaining to be traversed by this iterator. */
        size_t LinesRemaining() const { return m_contour.Points() - m_index; }

        private:
        const TContour& m_contour;
        size_t m_index;
        TLine m_line;
        };

    /** Returns a constant iterator positioned at the start of the lines. */
    LineIter begin_lines() const { return LineIter(*this,0); }
    /** Returns a constant iterator positioned at the end of the lines. */
    LineIter end_lines() const { return LineIter(*this,m_points); }

    private:
    const TPoint* m_point = nullptr;
    size_t m_points = 0;
    bool m_closed = false;
    bool m_has_off_curve_points = false;
    };

/** Returns a TContour object representing this contour. */
inline MContour::operator TContour() const
    {
    return Contour(0);
    }

/** An interface class for writable contour data. */
class MWritableContour: public MContour
    {
    public:
    /** The assignment operator. */
    MWritableContour& operator=(const MContour& aContour)
        {
        SetClosed(aContour.Closed());
        ReduceSizeTo(0);
        AppendContour(aContour);
        return *this;
        }

    /** Sets a point. */
    virtual void SetPoint(size_t aIndex,const TOutlinePoint& aPoint) = 0;
    /** Reduces the number of points to aPoints. The address of the points must not change. */
    virtual void ReduceSizeTo(size_t aPoints) = 0;
    /** Sets the number of points to aPoints. The address of the points may change. */
    virtual void SetSize(size_t aPoints) = 0;
    /** Sets this contour's closed attribute. Does nothing if that is not possible. */
    virtual void SetClosed(bool aClosed) = 0;
    /** Appends a point. */
    virtual void AppendPoint(const TOutlinePoint& aPoint) = 0;
    /** Returns a writable pointer to TOutlinePoint data if possible. */
    virtual TOutlinePoint* OutlinePointData() = 0;
    /** Returns a writable pointer to TPoint data if possible. */
    virtual TPoint* PointData() = 0;

    /** Offsets all the points by (aDx,aDy). */
    void Offset(int32_t aDx,int32_t aDy)
        {
        size_t i = 0;
        size_t points = Points();
        while (i < points)
            {
            auto p = Point(i);
            p.iX += aDx;
            p.iY += aDy;
            SetPoint(i++,p);
            }
        }

    void AppendContour(const TContour& aContour);
    /** Sets the last point to aPoint. */
    void SetLastPoint(const TOutlinePoint& aPoint) { SetPoint(Points() - 1,aPoint); }
    void Simplify(double aResolutionArea);
    void AppendCircularArc(const TPoint& aCenter,const TPoint& aStart,
                           const TPoint& aEnd,TEllipseAngleType aAngleType = TEllipseAngleType::Shortest,bool aAppendStart = false);
    void AppendHalfCircle(const TPoint& aCenter,const TPoint& aStart,
                          const TPoint& aEnd,TEllipseAngleType aAngleType = TEllipseAngleType::Shortest,bool aAppendStart = false);
    void AppendHalfCircle(double aCx,double aCy,double aSx,double aSy,double aEx,double aEy,
                          double aRadius,bool aAppendStart,bool aIsExactHalfCircle,bool aClockwise);
    void AppendQuadrant(double aCx,double aCy,double aSx,double aSy,double aEx,double aEy,double aRadius,
                        bool aAppendStart,bool aIsExactQuadrant,bool aClockwise);
    };

/** The data for a contour. The simplest implementation of writable contour data. */
class TSimpleContourData: public MWritableContour
    {
    public:
    TSimpleContourData() = default;
    /** Creates a TSimpleContourData object. */
    TSimpleContourData(TPoint* aPoint,size_t aPoints,bool aClosed): iPoint(aPoint), iPoints(aPoints), iClosed(aClosed) {  }

    TContour Contour(size_t /*aIndex*/) const override { return TContour(iPoint,iPoints,iClosed); }
    bool MayHaveCurves() const override { return false; }
    TOutlinePoint Point(size_t aIndex) const override { assert(aIndex < iPoints); return iPoint[aIndex]; }
    /** Returns a pointer to the first point. */
    TPoint* PointData() override { return iPoint; }
    TOutlinePoint* OutlinePointData() override { return nullptr; }
    /** Returns the number of points. */
    size_t Points() const override { return iPoints; }
    /** Reduces the number of points. */
    void ReduceSizeTo(size_t aPoints) override { assert(aPoints <= iPoints); iPoints = aPoints; }
    /** Sets the number of points. */
    void SetSize(size_t aPoints) override { assert(aPoints <= iPoints); iPoints = aPoints; }
    /** Returns true if the contour is closed. */
    bool Closed() const override { return iClosed; }
    /** Changes whether the contour is closed. */
    void SetClosed(bool aClosed) override { iClosed = aClosed; } // not possible
    /** Appends a point; does nothing in fact because the operation is not possible for this class. */
    void AppendPoint(const TOutlinePoint&) override { } // not possible
    void SetPoint(size_t aIndex,const TOutlinePoint& aPoint) override { assert(aIndex < iPoints); assert(aPoint.iType == TPointType::OnCurve); iPoint[aIndex] = aPoint.Point(); }

    /** A pointer to the start of the array of points. */
    TPoint* iPoint = nullptr;
    /** The number of points in the contour. */
    size_t iPoints = 0;
    /** True if the contour is closed. */
    bool iClosed = false;
    };

/** An abstract base class for the writable contour classes CContour and COnCurveContour. */
template<class T> class CContourBase: public MWritableContour
    {
    public:
    // virtual functions from MPath

    /** Returns the number of contours, which is one. */
    size_t Contours() const override { return 1; }

    /** Returns the single contour of this object. */
    TContour Contour(size_t /*aIndex*/) const override { return TContour(iPoint.data(),iPoint.size(),iClosed); }

    // virtual functions from MContour

    /** Returns the number of points. */
    size_t Points() const override { return iPoint.size(); }

    /** Returns a point selected by its index. */
    TOutlinePoint Point(size_t aIndex) const override { return iPoint[aIndex]; }

    /** Returns true if this contour is closed. */
    bool Closed() const override { return iClosed; }

    // virtual functions from MWritableContour

    /** Sets a point. */
    void SetPoint(size_t aIndex,const TOutlinePoint& aPoint) override { iPoint[aIndex] = aPoint; }

    /** Reduces the number of points to aPoints. The address of the points does not change. */
    void ReduceSizeTo(size_t aPoints) override
        {
        assert(aPoints <= iPoint.size());
        iPoint.resize(aPoints);
        }

    /** Sets the number of points to aPoints. The address of the points may change. */
    void SetSize(size_t aSize) override { iPoint.resize(aSize); }

    /** Sets this contour's closed attribute. Does nothing if that is not possible. */
    void SetClosed(bool aClosed) override { iClosed = aClosed; }

    /** Appends a point. */
    void AppendPoint(const TOutlinePoint& aPoint) override = 0;

    /** Appends a point to the contour whether or not it differs from the previous point. */
    void AppendPointEvenIfSame(const T& aPoint)
        {
        iPoint.push_back(aPoint);
        }
    /** Appends a point to the contour whether or not it differs from the previous point. */
    void AppendPoint(double aX,double aY)
        {
        iPoint.emplace_back(Round(aX),Round(aY));
        }
    /** Appends some points to the contour. */
    void AppendPoints(const T* aPoint,size_t aPoints) { iPoint.insert(iPoint.end(),aPoint,aPoint + aPoints); }
    /** Inserts a point at the specified index. */
    void InsertPoint(const T& aPoint,size_t aIndex) { iPoint.insert(iPoint.begin() + aIndex,aPoint); }
    /** Inserts some points at the specified index. */
    void InsertPoints(const T* aPoint,size_t aPoints,size_t aIndex) { iPoint.insert(iPoint.begin() + aIndex,aPoint,aPoint + aPoints); }
    /** Sets the path to its newly constructed state: empty and open. */
    void Clear() { iPoint.clear(); iClosed = false; }
    /** Reduces the number of points to zero. */
    void SetSizeToZero() { iPoint.clear(); }
    /** Removes a point specified by an index. */
    void RemovePoint(size_t aIndex) { iPoint.erase(iPoint.begin() + aIndex); }
    /** Removes a series of aCount points starting at aIndex. */
    void RemovePoints(size_t aIndex,size_t aCount) { iPoint.erase(iPoint.begin() + aIndex, iPoint.begin() + aIndex + aCount); }

    /** Applies an offset to every point in a contour. */
    void Offset(int32_t aDx,int32_t aDy)
        {
        for (auto& p : iPoint)
            {
            p.iX += aDx;
            p.iY += aDy;
            }
        }

    /**
    Pre-allocates enough space to hold at least aCount points. This function has no effect on behavior
    but may increase speed.
    */
    void ReservePoints(size_t aCount) { iPoint.reserve(aCount); }
    /** Reverses the order of the points in the contour. */
    void Reverse() { std::reverse(iPoint.begin(),iPoint.end()); }

    /** Returns an iterator pointing to the first point of the contour. */
    using vector_t = std::vector<T>;

    /** Returns an iterator pointing to the first point of the contour. */
    typename vector_t::iterator begin() { return iPoint.begin(); }
    /** Returns an iterator pointing just after the last point of the contour. */
    typename vector_t::iterator end() { return iPoint.end(); }
    /** Returns a constant iterator pointing to the first point of the contour. */
    typename vector_t::const_iterator begin() const { return iPoint.begin(); }
    /** Returns a constant iterator pointing just after the last point of the contour. */
    typename vector_t::const_iterator end() const { return iPoint.end(); }

    protected:
    /** An array containing the points of the contour. */
    vector_t iPoint;
    /** True if the contour is closed. */
    bool iClosed = false;
    };

/**
A contour consisting of owned points with integer coordinates, which may be on-curve or off-curve.
Off-curve points are control points for quadratic or cubic spline curves. A contour may be open or closed.
*/
class CContour: public CContourBase<TOutlinePoint>
    {
    public:
    CContour() = default;

    /** Creates a contour by copying another contour. */
    explicit CContour(const MContour& aContour)
        {
        iClosed = aContour.Closed();
        AppendContour(aContour);
        }

    /** The assignment operator. */
    CContour& operator=(const MContour& aContour) { MWritableContour::operator=(aContour); return *this; }

    // virtual functions from MPath
    bool MayHaveCurves() const override { return true; }

    // virtual functions from MContour
    TPoint* PointData() override { return nullptr; }
    TOutlinePoint* OutlinePointData() override { return iPoint.data(); }

    /** Returns a constant pointer to the start of the points. */
    const TOutlinePoint* OutlinePointData() const { return iPoint.data(); }

    /** Appends a point to the contour, but only if it differs from the previous point or is a control point. */
    void AppendPoint(const TOutlinePoint& aPoint) override
        {
        if (iPoint.size() && aPoint.iType == TPointType::OnCurve && aPoint == iPoint.back())
            return;
        iPoint.push_back(aPoint);
        }
    /** Appends a point to the contour whether or not it differs from the previous point. */
    void AppendPoint(double aX,double aY,TPointType aPointType = TPointType::OnCurve)
        {
        iPoint.emplace_back(Round(aX),Round(aY),aPointType);
        }
    void AppendTransformedPoint64ths(const TTransform& aTransform,const TPointFP& aPoint,TPointType aPointType);
    /** Converts a point from floating-point to point in 64ths and appends it. */
    void AppendPoint64ths(const TPointFP& aPoint,TPointType aPointType)
        {
        iPoint.emplace_back(aPoint.Rounded64ths(),aPointType);
        }

    void MakeCircle(double aX,double aY,double aRadius);
    void MakePolygon(double aX,double aY,double aRadius,int32_t aSides);
    void MakeRoundedRectangle(double aX1,double aY1,double aX2,double aY2,double aWidth,double aRadius);
    static CContour RoundedRectangle(const TPoint& aTopLeft,double aWidth,double aHeight,double aRX,double aRY);
    static CContour Rectangle(const TPoint& aTopLeft,double aWidth,double aHeight);
    static CContour Read(TDataInputStream& aInput);
    };

/**
A contour consisting of owned points with integer coordinates, which are all on the curve; the contour thus consists of straight line segments.
For contours with off-curve points use CContour.
*/
class COnCurveContour: public CContourBase<TPoint>
    {
    public:
    COnCurveContour() = default;

    /** Creates a contour by copying another contour. */
    explicit COnCurveContour(const MContour& aContour)
        {
        iClosed = aContour.Closed();
        AppendContour(aContour);
        }

    /** Creates a contour from an axis-aligned rectangle. */
    COnCurveContour(const TRect& aRect)
        {
        *this = aRect;
        }

    /** The assignment operator. */
    COnCurveContour& operator=(const MContour& aContour) { MWritableContour::operator=(aContour); return *this; }

    /** An assignment operator which takes an axis-aligned rectangle. */
    COnCurveContour& operator=(const TRect& aRect)
        {
        iClosed = true;
        iPoint.resize(4);
        iPoint[0] = aRect.iTopLeft;
        iPoint[1] = aRect.TopRight();
        iPoint[2] = aRect.iBottomRight;
        iPoint[3] = aRect.BottomLeft();
        return *this;
        }

    // virtual functions from MPath
    bool MayHaveCurves() const override { return false; }

    // virtual functions from MContour
    TPoint* PointData() override { return iPoint.data(); }
    TOutlinePoint* OutlinePointData() override { return nullptr; }

    /** Returns a constant pointer to the point data. */
    const TPoint* PointData() const { return iPoint.data(); }

    /** Appends a point to the contour, but only if it differs from the previous point. */
    void AppendPoint(const TOutlinePoint& aPoint) override
        {
        if (iPoint.size() && aPoint.Point() == iPoint.back())
            return;
        iPoint.push_back(aPoint);
        }
    /** Appends a point to the contour whether or not it differs from the previous point. */
    void AppendPoint(double aX,double aY)
        {
        iPoint.emplace_back(Round(aX),Round(aY));
        }
    void AppendTransformedPoint64ths(const TTransform& aTransform,const TPointFP& aPoint);
    /** Converts a point from floating-point to point in 64ths and appends it. */
    void AppendPoint64ths(const TPointFP& aPoint)
        {
        iPoint.emplace_back(aPoint.Rounded64ths());
        }
    };

/**
A concrete writable contour to be returned by CMapObject::WritableContour. It forwards MWritableContour functions either to an external interface or an owned object.
The latter option makes it possible for small objects holding a single point to return a writable contour object, in which case the point is changeable but the number of
points is not, nor the closed status.
*/
class TWritableContour: public MWritableContour
    {
    public:
    /** Creates a TWritableContour from an MWritableContour. */
    TWritableContour(MWritableContour& aWritableContour): m_ptr(&aWritableContour) {  }
    /** Creates a TWritableContour from a sequence of points. */
    TWritableContour(TPoint* aPoint,size_t aPoints,bool aClosed): m_data(aPoint,aPoints,aClosed), m_ptr(&m_data) {  }

    TContour Contour(size_t /*aIndex*/) const override { return m_ptr->Contour(0); }
    bool MayHaveCurves() const override { return m_ptr->MayHaveCurves(); }

    size_t Points() const override { return m_ptr->Points(); }
    TOutlinePoint Point(size_t aIndex) const override { return m_ptr->Point(aIndex); }
    bool Closed() const override { return m_ptr->Closed(); }

    void SetPoint(size_t aIndex,const TOutlinePoint& aPoint) override { m_ptr->SetPoint(aIndex,aPoint); }
    void ReduceSizeTo(size_t aPoints) override { m_ptr->ReduceSizeTo(aPoints); }
    void SetSize(size_t aPoints) override { m_ptr->SetSize(aPoints); }
    void SetClosed(bool aClosed) override { m_ptr->SetClosed(aClosed); }
    void AppendPoint(const TOutlinePoint& aPoint) override { m_ptr->AppendPoint(aPoint); }
    TOutlinePoint* OutlinePointData() override { return m_ptr->OutlinePointData(); }
    TPoint* PointData() override { return m_ptr->PointData(); }

    private:
    TSimpleContourData m_data;
    MWritableContour* m_ptr = nullptr;
    };

/** The standard path class. */
class COutline: public MPath
    {
    public:
    COutline() = default;
    COutline(const MPath& aPath);
    COutline(const TRect& aRect);
    COutline& operator=(const MPath& aPath);
    COutline& operator=(const TRect& aRect);

    // virtual functions from MPath
    size_t Contours() const override { return iContour.size(); }
    bool MayHaveCurves() const override { return true; }

    TContour Contour(size_t aIndex) const override
        {
        return iContour[aIndex].Contour(0);
        }

    /** Appends a contour. */
    void AppendContour(CContour&& aContour)
        {
        iContour.push_back(std::move(aContour));
        }

    /** Clears the outline by removing all contours. */
    void Clear() { iContour.clear(); }

    /** Appends a new empty contour to the outline and return it. */
    [[nodiscard]] CContour& AppendContour()
        {
        iContour.emplace_back();
        return iContour.back();
        }

    /** Appends a contour to the outline. */
    void AppendContour(const MContour& aContour)
        {
        auto& c = AppendContour();
        c.SetClosed(aContour.Closed());
        c.AppendContour(aContour);
        }

    /** Return a non-constant reference to a contour, selected by its index. */
    CContour& Contour(size_t aIndex) { return iContour[aIndex]; }

    void MapCoordinatesToLatLong(const CProjection& aProjection,int32_t aLatLongFractionalBits = 16);
    void LatLongToMapCoordinates(const CProjection& aProjection,int32_t aLatLongFractionalBits = 16);
    /** Removes the vector of contours and puts it into aDest. */
    void RemoveData(std::vector<CContour>& aDest)
        {
        aDest = std::move(iContour);
        iContour.clear();
        }
    static COutline Read(TDataInputStream& aInput);

    /** Returns an iterator to the start of the vector of contours. */
    std::vector<CContour>::iterator begin() { return iContour.begin(); }
    /** Returns an iterator to the end of the vector of contours. */
    std::vector<CContour>::iterator end() { return iContour.end(); }
    /** Returns a constant iterator to the start of the vector of contours. */
    std::vector<CContour>::const_iterator begin() const { return iContour.begin(); }
    /** Returns a constant iterator to the end of the vector of contours. */
    std::vector<CContour>::const_iterator end() const { return iContour.end(); }

    private:
    std::vector<CContour> iContour;
    };

/** Returns the contour to which this iterator refers. */
inline TContour MPath::ConstIter::operator*() const
    {
    return m_path.Contour(m_index);
    }

/** Traverses this path, calling the functions defined by aTraverser to handle moves, lines, and curves. Clips the output to aClip. */
template<class MPathTraverser> inline void MPath::Traverse(MPathTraverser& aTraverser,const TRect& aClip) const
    {
    Traverse(aTraverser,&aClip);
    }

/** Traverses this path, calling the functions defined by aTraverser to handle moves, lines, and curves. Clips the output to aClip if aClip is non-null. */
template<class MPathTraverser> inline void MPath::Traverse(MPathTraverser& aTraverser,const TRect* aClip) const
    {
    if (aClip && aClip->IsMaximal())
        aClip = nullptr;
    for (auto c : *this)
        c.Traverse(aTraverser,aClip);
    }

/** Traverses this contour, calling the functions defined by aTraverser to handle moves, lines, and curves. Clips the output to aClip if aClip is non-null. */
template<class MTraverser> inline void TContour::Traverse(MTraverser& aTraverser,const TRect* aClip) const
    {
    if (aClip && !aClip->IsMaximal() && ClipType(*aClip) != TClipType::Inside)
        {
        if (Points() < 2)
            return;
        COutline clipped_contour = ClippedContour(*aClip);
        clipped_contour.Traverse(aTraverser);
        return;
        }
    if (m_has_off_curve_points)
        CartoType::Traverse(aTraverser,(TOutlinePoint*)m_point,m_points,m_closed);
    else
        CartoType::Traverse(aTraverser,m_point,m_points,m_closed);
    }

/**
An iterator to traverse a path.
Limitation: for the moment it works with straight lines only; it
treats all points as on-curve points.
*/
class TPathIterator
    {
    public:
    TPathIterator(const MPath& aPath);
    bool Forward(double aDistance);
    bool NextContour();
    void MoveToNearestPoint(const TPointFP& aPoint);
    /** Returns the current contour index. */
    size_t ContourIndex() const { return iContourIndex; }
    /** Returns the current contour. */
    TContour Contour() const { return iContour; }
    /** Returns the current line index. */
    size_t LineIndex() const { return iLineIndex; }
    /** Returns the current position along the current line. */
    double PositionOnLine() const { return iPositionOnLine; }
    /** Returns the current position. */
    const TPoint& Position() const { return iPosition; }
    /** Returns the current direction in radians, clockwise from straight up, as an angle on the map, not a geodetic azimuth. */
    double Direction() const { return iDirection; }

    private:
    void CalculatePosition();

    const MPath& iPath;
    size_t iContourIndex = 0;
    TContour iContour;
    size_t iLineIndex = 0;
    TPoint iPosition;
    double iPositionOnLine = 0;
    double iLineLength = 0;
    double iDirection = 0;
    double iDx = 0;
    double iDy = 0;
    };

/** A contour with a fixed number of points. */
template<size_t aPointCount,bool aClosed> class TFixedSizeContour: public std::array<TOutlinePoint,aPointCount>, public MContour
    {
    public:
    size_t Points() const override { return aPointCount; }
    TOutlinePoint Point(size_t aIndex) const override { return std::array<TOutlinePoint,aPointCount>::data()[aIndex]; }
    bool Closed() const override { return aClosed; }
    bool MayHaveCurves() const override { return true; }
    TContour Contour(size_t /*aIndex*/) const override { return TContour(std::array<TOutlinePoint,aPointCount>::data(),aPointCount,aClosed); }

    using std::array<TOutlinePoint,aPointCount>::begin;
    using std::array<TOutlinePoint,aPointCount>::end;
    };

/** A contour consisting of non-owned points using floating-point coordinates, all of them being on-curve points. A TContourFP may be open or closed. */
class TContourFP
    {
    public:
    TContourFP() = default;
    /** Creates a TContourFP referring to a certain number of points, specifying whether it is open or closed. */
    TContourFP(const TPointFP* aPoint,size_t aPoints,bool aClosed) :
        m_point(aPoint),
        m_end(aPoint + aPoints),
        m_closed(aClosed)
        {
        }
    /** Returns a constant pointer to the first point. */
    const TPointFP* Point() const noexcept { return m_point; }
    /** Returns the number of points. */
    size_t Points() const noexcept { return m_end - m_point; }
    /** Returns true if this contour is closed. */
    bool Closed() const noexcept { return m_closed; }
    TRectFP Bounds() const noexcept;
    bool Intersects(const TRectFP& aRect) const noexcept;
    bool Contains(const TPointFP& aPoint) const noexcept;

    private:
    const TPointFP* m_point = nullptr;
    const TPointFP* m_end = nullptr;
    bool m_closed = false;
    };

/** A contour with a fixed number of floating-point points. */
template<size_t aPointCount,bool aClosed> class TFixedSizeContourFP: public std::array<TPointFP,aPointCount>
    {
    public:
    /** Returns a TContourFP object representing this object. */
    operator TContourFP() const { return TContourFP(std::array<TPointFP,aPointCount>::data(),aPointCount,aClosed); }
    };

class CPolygonFP;

/** A contour consisting of owned points using floating-point coordinates, all of them being on-curve points. A CContourFP may be open or closed. */
class CContourFP
    {
    public:
    CContourFP() { }
    /** Creates a contour from an axis-aligned rectangle. */
    explicit CContourFP(const TRectFP& aRect)
        {
        iPoint.resize(4);
        iPoint[0] = aRect.iTopLeft;
        iPoint[1] = aRect.BottomLeft();
        iPoint[2] = aRect.iBottomRight;
        iPoint[3] = aRect.TopRight();
        }
    /** Appends a point to this contour. */
    void AppendPoint(const TPointFP& aPoint) { iPoint.push_back(aPoint); }
    CPolygonFP Clip(const TRectFP& aClip);
    /** Returns the bounding box as an axis-aligned rectangle. */
    TRectFP Bounds() const
        { return TContourFP(iPoint.data(),iPoint.size(),false).Bounds(); }

    /** The vector of points representing this contour. */
    std::vector<TPointFP> iPoint;
    };

/** A polygon using floating-point coordinates, made up from zero or more contours represented as CContourFP objects. */
class CPolygonFP
    {
    public:
    TRectFP Bounds() const;

    /** The vector of contours. */
    std::vector<CContourFP> iContour;
    };

/**
A clip region.
This class enables optimisations: detemining whether the clip region is
an axis-aligned rectangle, and getting the bounding box.
*/
class TClipRegion
    {
    public:
    TClipRegion() = default;
    TClipRegion(const TRect& aRect);
    TClipRegion(const MPath& aPath);
    /** Returns the bounding box of this clip region. */
    const TRect& Bounds() const noexcept { return m_bounds; }
    /** Returns the clip region as a path. */
    const COutline& Path() const noexcept { return m_path; }
    /** Returns true if this clip region is an axis-aligned rectangle. */
    bool IsRect() const noexcept { return m_is_rect; }
    /** Returns true if this clip region is empty. */
    bool IsEmpty() const noexcept { return m_bounds.IsEmpty(); }

    private:
    TRect m_bounds;             // the bounds of the clip region as an axis-aligned rectangle
    COutline m_path;            // the clip path
    bool m_is_rect = true;      // true if the clip region is an axis-aligned rectangle
    };

/** Returns the area of a triangle made from the points aA, aB, aC. */
inline double TriangleArea(const TPoint& aA,const TPoint& aB,const TPoint& aC)
    {
    return fabs((double(aA.iX) - double(aC.iX)) * (double(aB.iY) - double(aA.iY)) - (double(aA.iX) - double(aB.iX)) * (double(aC.iY) - double(aA.iY))) / 2.0;
    }

/**
A templated function to simplify a contour of any type. Returns the new number of points, after simplification.

Simplification consists in removing all points with an area less than aResolutionArea.
A point's area is the area of the triangle made by the point and its two neighbours.

The start and end of an open contour are not removed.
Closed contours may be reduced to nothing.
*/
template<class T> size_t SimplifyContour(T* aPointArray,size_t aPointCount,bool aClosed,double aResolutionArea)
    {
    if (aPointCount < 3)
        return aPointCount;
    
    double cur_area = 0;
    if (aClosed)
        {
        T* source_point = aPointArray;
        T* dest_point = source_point;
        T* end_point = aPointArray + aPointCount;
        T* prev_point = end_point - 1;
        while (source_point < end_point)
            {
            T* next_point = source_point + 1;
            if (next_point == end_point)
                next_point = aPointArray;
            cur_area += TriangleArea(*prev_point,*source_point,*next_point);
            if (cur_area >= aResolutionArea)
                {
                prev_point = dest_point;
                *dest_point++ = *source_point;
                cur_area = 0;
                }
            source_point++;
            }
        return dest_point - aPointArray;
        }

    T* source_point = aPointArray + 1;
    T* dest_point = source_point;
    T* end_point = aPointArray + aPointCount - 1;
    while (source_point < end_point)
        {
        cur_area += TriangleArea(dest_point[-1],*source_point,source_point[1]);
        if (cur_area >= aResolutionArea)
            {
            *dest_point++ = *source_point;
            cur_area = 0;
            }
        source_point++;
        }
    *dest_point++ = *end_point;
    return dest_point - aPointArray;
    }

/** Returns the axis-aligned bounding box of a sequence of points, treating control points as ordinary points. */
template<class T> TRect CBox(T* aPointArray,size_t aPointCount)
    {
    if (!aPointCount)
        return TRect();
    const T* p = aPointArray;
    const T* q = p + aPointCount;
    TRect box(INT32_MAX,INT32_MAX,INT32_MIN,INT32_MIN);
    while (p < q)
        {
        int32_t x = p->iX;
        int32_t y = p->iY;
        if (x < box.iTopLeft.iX)
            box.iTopLeft.iX = x;
        if (x > box.iBottomRight.iX)
            box.iBottomRight.iX = x;
        if (y < box.iTopLeft.iY)
            box.iTopLeft.iY = y;
        if (y > box.iBottomRight.iY)
            box.iBottomRight.iY = y;
        p++;
        }
    return box;
    }

}

#endif
