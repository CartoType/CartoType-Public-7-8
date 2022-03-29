/*
cartotype_navigation.h
Copyright (C) 2013-2020 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_NAVIGATION_H__
#define CARTOTYPE_NAVIGATION_H__

#include <cartotype_path.h>
#include <cartotype_stream.h>
#include <cartotype_road_type.h>
#include <cartotype_map_object.h>

namespace CartoType
{

class CFramework;
class CMap;
class CMapDataBase;
class CMapObject;
class TRouteProfile;
class CProjection;

namespace Router
    {
    class TJunctionInfo;
    }

/** Types of router; used when selecting a router type. */
enum class TRouterType
    {
    /** This router type causes the default router to be selected: the one for which serialized data is available in the map file, or, failing that, StandardAStar. */
    Default,
    /** The A* router, which gives fast performance but takes a lot of memory and cannot create a route going twice through the same junction. */
    StandardAStar,
    /** A version of the A* router with road nodes and turn arcs; slower than StandardAStar, and uses more memory, but can generate routes going twice through the same junction, for more flexible routing. */
    TurnExpandedAStar,
    /**
    The contraction hierarchy router is intended where less RAM is available: for example with large maps on mobile devices.
    It gives the same routes as StandardAStar, but is a little slower and does not support custom route profiles; the route profile is decided at the time of creating the CTM1 file.
    */
    StandardContractionHierarchy,
    /** Turn-expanded contraction hierarchy. */
    TECH
    };

/**
Turn types on a route. Turns at junctions are classified
by dividing the full circle into 45-degree segments,
except for sharp turns left and right, which encompass everything
up to an actual U-turn, and left and right forks, for which there
are special rules to avoid classifying them as ahead.

The actual turn angle is also given in the TTurn class.
*/
enum class TTurnType
    {
    /** No turn exists or is needed. This turn type is used at the start of a route. */
    None,
    /**
    A turn of less than 15 degrees left or right, unless this is a fork with two choices, in which
    case the turn will be bear right or bear left.
    */
    Ahead,
    /**
    A turn between 15 degrees and 45 degrees right
    or a turn through a smaller angle which is the rightmost of a fork with two choices.
    */
    BearRight,
    /** A turn between 45 degrees and 120 degrees right. */
    Right,
    /** A turn between 120 degrees and 180 degrees right. */
    SharpRight,
    /** This turn type is use for U-turns: turns back along the same road. */
    Around,
    /** A turn between 120 degrees and 180 degrees left. */
    SharpLeft,
    /** A turn between 45 degrees and 120 degrees left. */
    Left,
    /**
    A turn between 15 degrees and 45 degrees left.
    or a turn through a smaller angle which is the leftmost of a fork with two choices.
    */
    BearLeft
    };

/**
Roundabout states; turns involving roundabouts are marked as such so that
exit numbers can be counted
*/
enum class TRoundaboutState
    {
    /** This junction does not involve a roundabout. */
    None,
    /** This junction enters a roundabout. */
    Enter,
    /** This junction continues around a roundabout. */
    Continue,
    /** This junction exits a roundabout. */
    Exit
    };

/** A turn: a choice of route through a node. */
class TTurn
    {
    public:
    /** Sets the TTurn object to its just-constructed state. */
    void Clear()
        {
        *this = TTurn();
        }

    /** Sets the TTurn object using inward and outward direction in degrees (anticlockwise from map east), left and right alternative choices, and fork and turn-off flags. */
    void SetTurn(double aInDir,double aOutDir,int32_t aChoices,int32_t aLeftAlternatives,int32_t aRightAlternatives,bool aIsFork,bool aTurnOff)
        {
        double turn_angle = aInDir - aOutDir;
        if (turn_angle > 180)
            turn_angle -= 360;
        else if (turn_angle < -180)
            turn_angle += 360;
        SetTurn(turn_angle);
        iInDirection = aInDir;
        iOutDirection = aOutDir;
        iChoices = aChoices;
        iLeftAlternatives = aLeftAlternatives;
        iRightAlternatives = aRightAlternatives;
        iIsFork = aIsFork;
        iTurnOff = aTurnOff;
        if (iTurnType == TTurnType::Ahead && (iTurnOff || (iIsFork && iChoices == 2)))
            iTurnType = iLeftAlternatives ? TTurnType::BearRight : TTurnType::BearLeft;
        }

    /** Sets the TTurn object's turn angle and turn type from an angle in degrees. Leaves other date members unchanged. */
    void SetTurn(double aTurnAngle)
        {
        iTurnAngle = aTurnAngle;
        if (iTurnAngle > 120)
            iTurnType = TTurnType::SharpRight;
        else if (iTurnAngle > 45)
            iTurnType = TTurnType::Right;
        else if (iTurnAngle > 15)
            iTurnType = TTurnType::BearRight;
        else if (iTurnAngle > -15)
            iTurnType = TTurnType::Ahead;
        else if (iTurnAngle > -45)
            iTurnType = TTurnType::BearLeft;
        else if (iTurnAngle > -120)
            iTurnType = TTurnType::Left;
        else
            iTurnType = TTurnType::SharpLeft;
        }

    /** Writes the TTurn object in XML format as a turn element. */
    void WriteAsXml(MOutputStream& aOutput) const;

    /** The turn type: ahead, left, right, etc. */
    TTurnType iTurnType = TTurnType::None;
    /** True if this turn is a continuation of the current road and no notification is needed. */
    bool iContinue = true;
    /** The roundabout state. */
    TRoundaboutState iRoundaboutState = TRoundaboutState::None;
    /** The turn angle in degrees: 0 = straight ahead; negative = left, positive = right. */
    double iTurnAngle = 0;
    /** The incoming direction in degrees relative to map east, going anticlockwise. */
    double iInDirection = 0;
    /** The outgoing direction in degrees relative to map east, going anticlockwise. */
    double iOutDirection = 0;
    /** The junction to take, counting the current junction as 0, if this junction is part of a roundabout. */
    int32_t iExitNumber = 0;
    /** The number of choices at this turning if known; 0 if not known. */
    int32_t iChoices = 0;
    /** The number of choices to the left of the turn actually taken; if iChoices is zero, this data is not known. */
    int32_t iLeftAlternatives = 0;
    /** The number of choices to the right of the turn actually taken; if iChoices is zero, this data is not known. */
    int32_t iRightAlternatives = 0;
    /** True if this turn is a fork: that is, there is an alternative within 22.5 degrees of the turn actually taken. */
    bool iIsFork = false;
    /** True if this choice is a turn off, defined as a turn on to a lower-status road. */
    bool iTurnOff = false;
    /** The name of the junction. */
    CString iJunctionName;
    /** The reference code of the junction. */
    CString iJunctionRef;
    };

/** Information about a route segment. */
class CRouteSegment
    {
    public:
    CRouteSegment():
        iRoadType(TRoadType::UnknownMajor),
        iMaxSpeed(0),
        iDistance(0),
        iTime(0),
        iTurnTime(0),
        iSection(0),
        iRestricted(false)
        {
        }
    /**
    Writes a route segment in XML as a segment element.
    Uses aProjection to convert the points to latitude and longitude.
    */
    void WriteAsXml(MOutputStream& aOutput,const CProjection& aProjection) const;
    /** Creates a copy of this route segment. */
    std::unique_ptr<CRouteSegment> Copy() const;

    /** The road type of the object of which this segment is a part. */
    TRoadType iRoadType;
    /**
    The maximum legal speed in kilometres per hour. A value of zero means there is no known speed limit,
    or the standard speed limit for the type of road applies.
    */
    double iMaxSpeed;
    /** The standard name of the object of which this segment is a part. */
    CString iName;
    /** The road reference of the object of which this segment is a part. */
    CString iRef;
    /** The distance in metres. */
    double iDistance;
    /** The estimated time in seconds, including iTurnTime, taken to traverse the segment. */
    double iTime;
    /** The estimated time in seconds taken to navigate the junction at the start of the segment. */
    double iTurnTime;
    /** The path of this segment in map units. */
    COnCurveContour iPath;
    /**
    The section number.
    Routes are divided into sections between waypoints. A simple route has one section,
    a route with an intermediate waypoint has two sections, and so on. Sections
    are numbered from zero upwards.
    */
    int32_t iSection;
    /** Information about how to navigate the junction at the start of this segment. */
    TTurn iTurn;
    /** True if this segment is restricted: for example, a private access road. */
    bool iRestricted;
    };

/** Information about the nearest road (in fact, the nearest routable map object) to a certain point. */
class TNearestRoadInfo
    {
    public:

    /** The road type. */
    TRoadType iRoadType = TRoadType::UnknownMajor;
    /**
    The maximum legal speed in kilometres per hour. A value of zero means there is no known speed limit,
    or the standard speed limit for the type of road applies.
    */
    double iMaxSpeed = 0;
    /** The standard name of the road. */
    CString iName;
    /** The road reference of the road. */
    CString iRef;
    /** The nearest point on the road, in map coordinates, to the chosen point. */
    TPoint iNearestPoint;
    /** The distance from the chosen point to iNearestPoint in meters. */
    double iDistance = 0;
    /** The heading of the nearest segment in degrees. */
    double iHeadingInDegrees = 0;
    /** The heading of the nearest segment as a unit vector. */
    TPointFP iHeadingVector;
    /** The road's geometry in map coordinates. The road is guaranteed to be traversible in the direction of iPath. */
    CContour iPath;
    /** True if the road is one-way. */
    bool iOneWay = false;
    };

/** Information about the nearest route segment to a point. */
class TNearestSegmentInfo
    {
    public:
    /** The index of the segment in the CRoute object, or -1 if there were no segments. */
    int32_t iSegmentIndex = -1;
    /**
    The index of the line within the segment's path:
    line N goes from point N to point N + 1.
    */
    int32_t iLineIndex = 0;
    /** The point in the segment's path nearest to the other point, in map coordinates. */
    TPointFP iNearestPoint;
    /** The distance from the other point to iNearestPoint in meters. */
    double iDistanceToRoute = 0;
    /** The distance of the nearest point along the route in meters. */
    double iDistanceAlongRoute = 0;
    /** The distance within the current segment in meters. */
    double iDistanceAlongSegment = 0;
    /** The estimated time of the nearest point, along the route, in seconds. */
    double iTimeAlongRoute = 0;
    /** The estimated time within the current segment, in seconds. */
    double iTimeAlongSegment = 0;
    /** The heading of the nearest line as a map angle taken anti-clockwise from rightwards. */
    double iHeading = 0;
    };

/**
Information about a path from the start or
end of the route to the nearest non-trivial junction.
Used when exporting traffic information.
*/
class CPathToJunction
    {
    public:
    CPathToJunction(): iStartRoadType(TRoadType::UnknownMajor), iEndRoadType(TRoadType::UnknownMajor), iDistance(0) { }
    /** Sets the object to its just-contructed state. */
    void Clear()
        {
        iPath.Clear();
        iStartRoadType = iEndRoadType = TRoadType::UnknownMajor;
        iDistance = 0;
        }
    /** Reverses the path. */
    void Reverse()
        {
        iPath.Reverse();
        TRoadType temp(iStartRoadType);
        iStartRoadType = iEndRoadType;
        iEndRoadType = temp;
        }
    /** Copies the data froma Other to this object. */
    void Set(const CPathToJunction& aOther)
        {
        iStartRoadType = aOther.iStartRoadType;
        iEndRoadType = aOther.iEndRoadType;
        iDistance = aOther.iDistance;
        iPath.Clear();
        iPath.AppendContour(aOther.iPath);
        }

    /** The path between the junction and the start or end of the route. */
    CContour iPath;
    /** The road type at the start of the path. */
    TRoadType iStartRoadType;
    /** The road type at the start of the path. */
    TRoadType iEndRoadType;
    /** The length of the path in meters. */
    double iDistance;
    };

// Bit flags and masks describing an arc, for use in TRouteProfile.

/** A bit mask for road types: used by routers. */
constexpr uint32_t KArcRoadTypeMask = 0x1F;                // 5 bits for the road type
/** A bit mask for road gradents: used by routers. */
constexpr uint32_t KArcGradientMask = 0xE0;                // 3 bits for the average road gradient
/** A bit flag for road types: used by routers. */
constexpr uint32_t KArcGradientDirectionFlag = 0x80;
/** The number of bits by which the gradient field is shifted: used by routers. */
constexpr uint32_t KArcGradientShift = 5;
/** A bit mask for road directions: used by routers. */
constexpr uint32_t KArcRoadDirectionMask = 0x300;
/** The road direction value for drive-on-right: used by routers. */
constexpr uint32_t KArcDriveOnRightRoadDirection = 0;
/** The road direction value for drive-on-left: used by routers. */
constexpr uint32_t KArcDriveOnLeftRoadDirection = 0x300;
/** The road direction value for one-way forward: used by routers. */
constexpr uint32_t KArcOneWayForwardRoadDirection = 0x100;
/** The road direction value for one-way backward: used by routers. */
constexpr uint32_t KArcOneWayBackwardRoadDirection = 0x200;
/** A bit flag for roundabouts: used by routers. */
constexpr uint32_t KArcRoundaboutFlag = 0x400;
/** A bit flag for toll roads: used by routers. */
constexpr uint32_t KArcTollFlag = 0x800;
/** A bit mask for the speed limit in kph if known (zero means no known speed limit): used by routers. */
constexpr uint32_t KArcSpeedLimitMask = 0xFF000;           // 8 bits for the speed limit in kph if known
/** The number of bits by which the speed limit field is shifted: used by routers. */
constexpr uint32_t KArcSpeedLimitShift = 12;

/** A bit mask used by access restrictions in a route profile, for access restrictions not including the 'other access' flag. */
constexpr uint32_t KArcAccessMask = 0x7FF00000;            
/** A bit flag used by access restrictions in a route profile: this arc goes the wrong way along a one-way road but it can be used by pedestrians and emergency vehicles. */
constexpr uint32_t KArcWrongWayFlag = 0x00100000;
/** A bit flag used by access restrictions in a route profile: no access for bicycles. */
constexpr uint32_t KArcBicycleAccessFlag = 0x00200000;
/** A bit flag used by access restrictions in a route profile: no access for motorcycles. */
constexpr uint32_t KArcMotorCycleAccessFlag = 0x00400000;
/** A bit flag used by access restrictions in a route profile: no access for cars. */
constexpr uint32_t KArcCarAccessFlag = 0x00800000;
/** A bit flag used by access restrictions in a route profile: no access for high occupancy vehicles. */
constexpr uint32_t KArcHighOccupancyAccessFlag = 0x01000000;
/** A bit flag used by access restrictions in a route profile: no access for light goods vehicles. */
constexpr uint32_t KArcLightGoodsAccessFlag = 0x02000000;
/** A bit flag used by access restrictions in a route profile: no access for heavy goods vehicles. */
constexpr uint32_t KArcHeavyGoodsAccessFlag = 0x04000000;
/** A bit flag used by access restrictions in a route profile: no access for buses. */
constexpr uint32_t KArcBusAccessFlag = 0x08000000;
/** A bit flag used by access restrictions in a route profile: no access for taxis. */
constexpr uint32_t KArcTaxiAccessFlag = 0x10000000;
/** A bit flag used by access restrictions in a route profile: no access for pedestrians. */
constexpr uint32_t KArcPedestrianAccessFlag = 0x20000000;
/** A bit flag used by access restrictions in a route profile: no access for emergency vehicles. */
constexpr uint32_t KArcEmergencyAccessFlag = 0x40000000;
/** A bit flag used by access restrictions in a route profile: other access restrictions exist such as weight, length, width, height or for vehicles carrying hazardous materials. */
constexpr uint32_t KArcOtherAccessFlag = 0x80000000;
/** A combination of bit flags used by access restrictions in a route profile: bicycle access only. */
constexpr uint32_t KArcBicycleAccessOnly = KArcAccessMask & ~(KArcWrongWayFlag | KArcBicycleAccessFlag);
/** A combination of bit flags used by access restrictions in a route profile: access flags affect all motor vehicles. */
constexpr uint32_t KArcAllMotorVehicles = KArcAccessMask & ~(KArcWrongWayFlag | KArcPedestrianAccessFlag | KArcBicycleAccessFlag);
/** A combination of bit flags used by access restrictions in a route profile: bicycle and pedestrian access only; synonym of KArcAllMotorVehicles. */
constexpr uint32_t KArcBicycleAndPedestrianAccessOnly = KArcAllMotorVehicles;
/** A combination of bit flags used by access restrictions in a route profile: no vehicular access. */
constexpr uint32_t KArcNoVehicularAccess = KArcAccessMask & ~(KArcWrongWayFlag | KArcPedestrianAccessFlag);
/** A combination of bit flags used by access restrictions in a route profile: access flags affect all vehicles; synonym of KArcNoVehicularAccess. */
constexpr uint32_t KArcAllVehicles = KArcNoVehicularAccess;

// Values for use in KArcRoadTypeMask and as indexes into the speed and bonus arrays in TRouteProfile.

/** The index for motorways in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcMotorway = 0;
/** The index for motorway links (ramps) in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcMotorwayLink = 1;
/** The index for trunk roads in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcTrunkRoad = 2;
/** The index for trunk road links (ramps) in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcTrunkRoadLink = 3;
/** The index for primary roads in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcPrimaryRoad = 4;
/** The index for primary road links (ramps) in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcPrimaryRoadLink = 5;
/** The index for secondary roads in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcSecondaryRoad = 6;
/** The index for secondary road links (ramps) in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcSecondaryRoadLink = 7;
/** The index for tertiary roads in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcTertiaryRoad = 8;
/** The index for unclassified roads in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcUnclassifiedRoad = 9;
/** The index for residential roads in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcResidentialRoad = 10;
/** The index for tracks in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcTrack = 11;
/** The index for service roads and access roads in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcServiceRoad = 12;
/** The index for pedestrian roads in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcPedestrianRoad = 13;
/** The index for vehicular ferries in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcVehicularFerry = 14;
/** The index for passenger ferries in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcPassengerFerry = 15;
/** The index for living streets in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcLivingStreet = 16;
/** The index for cycleways in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcCycleway = 17;
/** The index for paths in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcPath = 18;
/** The index for footways in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcFootway = 19;
/** The index for bridleways in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcBridleway = 20;
/** The index for steps in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcSteps = 21;
/** The index for roads of unknown type in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcUnknownRoadType = 22;
/** The index for unpaved (unsurfaced) roads in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcUnpavedRoad = 23;
/** The index for other road type 0 in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcOtherRoadType0 = 24;
/** The index for other road type 1 in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcOtherRoadType1 = 25;
/** The index for other road type 2 in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcOtherRoadType2 = 26;
/** The index for other road type 3 in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcOtherRoadType3 = 27;
/** The index for other road type 4 in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcOtherRoadType4 = 28;
/** The index for other road type 5 in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcOtherRoadType5 = 29;
/** The index for other road type 6 in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcOtherRoadType6 = 30;
/** The index for other road type 7 in the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcOtherRoadType7 = 31;
/** The number of road types and the size of the speed and bonus arrays in TRouteProfile. */
constexpr uint32_t KArcRoadTypeCount = 32;

// Index values for use in the gradient arrays in TRouteProfile.

/** The index for a very slight uphill gradient, or no gradient, in the gradient arrays in TRouteProfile. */
constexpr uint32_t KArcGradientUp0Index = 0;
/** The index for a gentle uphill gradient in the gradient arrays in TRouteProfile. */
constexpr uint32_t KArcGradientUp1Index = 1;
/** The index for a steep uphill gradient in the gradient arrays in TRouteProfile. */
constexpr uint32_t KArcGradientUp2Index = 2;
/** The index for a very steep uphill gradient in the gradient arrays in TRouteProfile. */
constexpr uint32_t KArcGradientUp3Index = 3;
/** The index for a very slight downhill gradient, or no gradient, in the gradient arrays in TRouteProfile. */
constexpr uint32_t KArcGradientDown0Index = 4;
/** The index for a gentle downhill gradient in the gradient arrays in TRouteProfile. */
constexpr uint32_t KArcGradientDown1Index = 5;
/** The index for a steep downhill gradient in the gradient arrays in TRouteProfile. */
constexpr uint32_t KArcGradientDown2Index = 6;
/** The index for a very steep downhill gradient in the gradient arrays in TRouteProfile. */
constexpr uint32_t KArcGradientDown3Index = 7;

/** The size of the gradient arrays in TRouteProfile. */
constexpr uint32_t KArcGradientCount = 8;

/** The names of the different gradient ranges, as used in serialized XML rout profiles. */
const char * const KArcGradientName[KArcGradientCount] =
    {
    "up0", "up1", "up2", "up3", "down0", "down1", "down2", "down3"
    };

/** Constants used to select frequently-used route profiles. */
enum class TRouteProfileType
    {
    /** A profile type for driving in a private car (synonym of Car). */
    Drive,
    /** A profile type for driving in a private car (synonym of Drive). */
    Car = Drive,
    /** A profile type for walking. */
    Walk,
    /** A profile type for cycling (synonym of Bicycle). */
    Cycle,
    /** A profile type for cycling (synonym of Cycle). */
    Bicycle = Cycle,
    /** A profile type for walking, preferring off-road paths. */
    Hike
    };

/** A vehicle type used in routing. */
class TVehicleType
    {
    private:
    auto Tuple() const { return std::forward_as_tuple(iAccessFlags,iWeight,iAxleLoad,iDoubleAxleLoad,iTripleAxleLoad,iHeight,iWidth,iLength,iHazMat); }

    public:
    /** The equality operator. */
    bool operator==(const TVehicleType& aOther) const { return Tuple() == aOther.Tuple(); }
    /** True if this vehicle type obeys one-way restrictions. Pedestrian types usually do not. */
    bool ObeysOneWay() const { return (iAccessFlags & KArcWrongWayFlag) != 0; }
    /** True if walking (access to pedestrian routes) is allowed. */
    bool WalkingAllowed() const { return (iAccessFlags & KArcPedestrianAccessFlag) != 0; }
    /** True if cycling (access to cycle routes) is allowed. */
    bool CyclingAllowed() const { return (iAccessFlags & KArcBicycleAccessFlag) != 0; }

    /** Normalizes the vehicle type: ensures that KArcOtherAccessFlag is set if and only if vehicle details are given, and that weights and dimensions are non-negative. */
    void Normalize()
        {
        if (iWeight > 0 ||
            iAxleLoad > 0 ||
            iDoubleAxleLoad > 0 ||
            iTripleAxleLoad > 0 ||
            iHeight > 0 ||
            iWidth > 0 ||
            iLength > 0 ||
            iHazMat)
            iAccessFlags |= KArcOtherAccessFlag;
        else
            iAccessFlags &= ~KArcOtherAccessFlag;

        if (iWeight < 0) iWeight = 0;
        if (iAxleLoad < 0) iAxleLoad = 0;
        if (iDoubleAxleLoad < 0) iDoubleAxleLoad = 0;
        if (iTripleAxleLoad < 0) iTripleAxleLoad = 0;
        if (iHeight < 0) iHeight = 0;
        if (iWidth < 0) iWidth = 0;
        if (iLength < 0) iLength = 0;
        }

    /**
    Flags taken from KArcWrongWayFlag ... KArcOtherAccessFlag indicating the vehicle type.
    Arcs with restrictions matching any of these flags will not be taken.
    */
    uint32_t iAccessFlags = KArcCarAccessFlag | KArcWrongWayFlag;

    /** The vehicle's weight in metric tons. Values of zero or less mean 'unknown'. */
    double iWeight = 0;

    /** The vehicle's axle load in metric tons. Values of zero or less mean 'unknown'. */
    double iAxleLoad = 0;

    /** The vehicle's double axle load in metric tons. Values of zero or less mean 'unknown'. */
    double iDoubleAxleLoad = 0;

    /** The vehicle's triple axle weight in metric tons. Values of zero or less mean 'unknown'. */
    double iTripleAxleLoad = 0;

    /** The vehicle's height in meters. Values of zero or less mean 'unknown'. */
    double iHeight = 0;

    /** The vehicle's width in meters. Values of zero or less mean 'unknown'. */
    double iWidth = 0;

    /** The vehicle's length in meters. Values of zero or less mean 'unknown'. */
    double iLength = 0;

    /** True if the vehicle is carrying hazardous materials. */
    bool iHazMat = false;
    };

/**
A routing profile: parameters determining the type of route,
including road speeds, bonuses and penalties, and vehicle type.
A road type will not normally be used if its speed plus bonus is equal to or less than zero.

However, if the speed is positive and the bonus is negative, and their sum is zero,
this type of road will be allowed at the beginning or end of a route, allowing
travel over farm tracks, for instance, to leave or to arrive at a destination.

Similarly, restricted road types such as private roads are allowed at the beginning
or end of a route.
*/
class TRouteProfile
    {
    public:
    /** Creates a route profile. If the profile type is not supplied the default value is the car profile type. */
    TRouteProfile(TRouteProfileType aProfileType = TRouteProfileType::Car);
    /** Creates a route profile from XML format by reading a CartoTypeRouteProfile element. Throws an exception on error. */
    explicit TRouteProfile(MInputStream& aInput);
    /** Reads a route profile from XML format by reading a CartoTypeRouteProfile element.. */
    TResult ReadFromXml(MInputStream& aInput);

    /** Writes the route profile in XML format as a CartoTypeRouteProfile element. */
    TResult WriteAsXml(MOutputStream& aOutput) const;

    /** The equality operator. */
    bool operator==(const TRouteProfile& aOther) const;
    
    /** The inequality operator. */
    bool operator!=(const TRouteProfile& aOther) const { return !(*this == aOther); }

    /** The optional name of the profile. */
    std::string iName;

    /** The vehicle type, defined using access flags, dimensions, weight, etc. */
    TVehicleType iVehicleType;

    /** Speeds along roads in kilometres per hour. */
    std::array<double,KArcRoadTypeCount> iSpeed = { };

    /**
    Bonuses or penalties in notional km per hour to be added to road types to make them more or less likely to be used.
    For example, a penalty of 1kph is applied to walking routes along major roads because it is pleasanter to walk along quieter minor roads.
    */
    std::array<double,KArcRoadTypeCount> iBonus = { };

    /**
    This array of bit masks allows restrictions to be overridden for certain
    types of road. For example, to allow routing of heavy goods vehicles along
    tracks, even if they are forbidden to motor vehicles, set
    iRestrictionOverride[KArcTrack] to KArcHeavyGoodsAccessFlag.
    */
    std::array<uint32_t,KArcRoadTypeCount> iRestrictionOverride = { };

    /**
    The estimated time in seconds taken for any turn at a junction that is not
    a slight turn or simply going straight ahead.
    */
    int32_t iTurnTime = 4;

    /**
    The estimated time in seconds taken for a U-turn, defined as a turn
    very close to 180 degrees (within 1/32 of a circle = 11.75 degrees).
    */
    int32_t iUTurnTime = 300;

    /**
    The estimated time in seconds taken for a turn across the traffic:
    that is, a left turn in drive-on-right countries,
    or a right turn in drive-on-left countries.
    */
    int32_t iCrossTrafficTurnTime = 12;

    /**
    The estimated delay in seconds caused by traffic lights.
    */
    int32_t iTrafficLightTime = 10;

    /** Set this flag to true to get the shortest route by distance or time, ignoring weightings. Whether distance or time is used depends on the setting of iShortestByTime. */
    bool iShortest = false;

    /** Set this flag to true if iShortest should produce the shortest route by time rather than distance. */
    bool iShortestByTime = false;

    /**
    The penalty applied to toll roads as a number between zero and one.
    The value 1 means that no toll roads will be used, 0.5 makes toll roads half as desirable as non-toll roads
    of the same road type, and so on. The value 0 means that no penalty is applied to toll roads.
    Values outside the range 0...1 are clamped to that range.
    */
    double iTollPenalty = 0;

    /**
    Speeds added to roads with a particular gradient.
    These values are intended for cycling, walking, hiking and running profiles,
    where speeds are generally the same for different types of road, but
    can change according to the gradient.
    */
    std::array<double,KArcGradientCount> iGradientSpeed = { };

    /** Bonuses applied to roads with a particular gradient. */
    std::array<double,KArcGradientCount> iGradientBonus = { };

    /**
    Flags indicating which roads are affected by gradient speeds and bonuses; normally steps, etc., are excluded.
    Each bit is (1 << road type).
    */
    uint32_t iGradientFlags = 0xFFFFFFFF & ~((1 << KArcSteps) | (1 << KArcPassengerFerry) | (1 << KArcVehicularFerry));
    };

/** Information about an entire route. */
class CRoute
    {
    public:
    CRoute();
    /**
    Creates a route by reading an XML CartoTypeRoute element.
    Uses aProjection to project the points from latitude and longitude.
    */
    CRoute(MInputStream& aInput,const CProjection& aProjection);
    /** Creates an empty route with a given route profile and point scale. */
    explicit CRoute(const TRouteProfile& aProfile,double aPointScale);
    /** Creates a Geometry object containing the points of a route. */
    std::unique_ptr<CGeometry> Geometry() const;
    /** Sets the route to empty. Sets the route profile to a default value (clears it) if aClearProfile is true. */
    void Clear(bool aClearProfile = true);
    /** Returns true if this route has no route segments. */
    bool Empty() const { return iRouteSegment.empty(); }
    /**
    Writes a route as an XML CartoTypeRoute element.
    Uses aProjection to inverse-project the points back to latitude and longitude.
    */
    TResult WriteAsXml(MOutputStream& aOutput,const CProjection& aProjection) const;
    /** Writes a route in GPX format. */
    TResult WriteAsGpx(MOutputStream& aOutput,const CProjection& aProjection) const;
    /**
    Gets information about the nearest route segment to a point given in map coordinates.

    The parameter aSection gives the current route section (a part of the route between waypoints;
    simple routes have a single section). The returned segment will always be in the current
    section or a following section. It is theoretically possible for a section to be completely skipped if it is very short or
    of zero length, so there is no constraint that section 0 must be followed by section 1 and not by section 2, etc.
    If aSection is negative it is ignored and the returned segment may be in any section.

    The parameter aPreviousDistanceAlongRoute should be provided (set to a value greater than zero)
    for multi-section routes, so that the correct route segment can be chosen if the route doubles back on itself.
    Positions further along the route will be given preference over those earlier in the route.
    */
    void GetNearestSegment(const TPoint& aPoint,TNearestSegmentInfo& aInfo,int32_t aSection,double aPreviousDistanceAlongRoute) const;
    /** Gets information about the point a certain distance along a route. */
    void GetPointAtDistance(double aDistanceInMeters,TNearestSegmentInfo& aInfo) const;
    /** Gets information about the point a certain estimated time along a route. */
    void GetPointAtTime(double aTimeInSeconds,TNearestSegmentInfo& aInfo) const;
    /** Appends one route to another. */
    void Append(const CRoute& aRoute);
    /** Copies a route. */
    std::unique_ptr<CRoute> Copy() const;
    /** Copies a route but does not copy restricted segments (e.g., private roads). */
    std::unique_ptr<CRoute> CopyWithoutRestrictedSegments() const;
    /** Returns textual instructions for a route. For internal use only. */
    CString Instructions(CMap& aMap,const char* aLocale,bool aMetricUnits,bool aAbbreviate) const;
    /** Returns the total distance in metres of the parts of the route that are on toll roads. */
    double TollRoadDistance() const;
    /** Appends a segment to a route. For internal use only. */
    void AppendSegment(const Router::TJunctionInfo& aBestArcInfo,const CString& aJunctionName,const CString& aJunctionRef,const CContour& aContour,
                       const CString& aName,const CString& aRef,TRoadType aRoadType,double aMaxSpeed,double aDistance,double aTime,int32_t aSection,bool aRestricted);

    /** An array of route segments representing the route. */
    std::vector<std::unique_ptr<CRouteSegment>> iRouteSegment;
    /** The distance of the route in meters. */
    double iDistance = 0;
    /** The estimated time taken to traverse the route in seconds. */
    double iTime = 0;
    /** The path along the entire route in map units. */
    COnCurveContour iPath;
    /**
    The scale used to convert map units from fractional to whole units.
    For example, if the map unit is 32nds of meters this will be 1/32.
    */
    double iPointScale = 1;
    /** The path to the first non-trivial junction before the route: used when creating OpenLR location data. */
    CPathToJunction iPathToJunctionBefore;
    /** The path to the first non-trivial junction after the route: used when creating OpenLR location data. */
    CPathToJunction iPathToJunctionAfter;
    /** The profile used to create the route. */
    TRouteProfile iProfile;

    private:
    void GetPointAlongRouteHelper(const TPoint* aPoint,double* aDistance,double* aTime,
                                  TNearestSegmentInfo& aInfo,int32_t aSection,double aPreviousDistanceAlongRoute) const;
    };

/** Data on the cost of creating a route. */
class TRouteCreationData
    {
    public:
    /** The time take to calculate the route, in seconds. */
    double iRouteCalculationTime = 0;
    /** The time taken to expand the raw route into a route object, in seconds. */
    double iRouteExpansionTime = 0;
    /** The number of node cache queries, if relevant. */
    int32_t iNodeCacheQueries = 0;
    /** The number of node cache misses, if relevant. */
    int32_t iNodeCacheMisses = 0;
    /** The number of arc cache queries, if relevant. */
    int32_t iArcCacheQueries = 0;
    /** The number of arc cache misses, if relevant. */
    int32_t iArcCacheMisses = 0;
    };

/** States of the navigation system. */
enum class TNavigationState
    {
    /** No route has been created, or navigation is disabled. */
    None,
    /** There is a route, and navigation is enabled, but no position has been supplied. */
    NoPosition,
    /** The current position is on the route and turn information is available. */
    Turn,
    /** The current position is off the route. */
    OffRoute,
    /**
    Re-routing is needed. This state is used only when automatic re-routing is turned
    off, for instance when using a slower routing algorithm.
    */
    ReRouteNeeded,
    /** A new route has been calculated after a period off route. */
    ReRouteDone,
    /** The latest position is on the route but a U-turn is needed. */
    TurnRound,
    /** The latest position is on the route and very close to the destination. */
    Arrival
    };

/** Turn information for navigation: the base Turn class plus the distance to the turn, road names and turn instruction. */
class TNavigatorTurn: public TTurn
    {
    public:
    TNavigatorTurn() = default;

    /** Creates a turn from aPrevSegment to aCurSegment with a distance in meters and a time in seconds. */
    TNavigatorTurn(const CRouteSegment& aPrevSegment,
                   const CRouteSegment& aCurSegment,
                   double aDistance,double aTime):
        TTurn(aCurSegment.iTurn),
        iDistance(aDistance),
        iTime(aTime),
        iFromName(aPrevSegment.iName),
        iFromRef(aPrevSegment.iRef),
        iFromRoadType(aPrevSegment.iRoadType),
        iToName(aCurSegment.iName),
        iToRef(aCurSegment.iRef),
        iToRoadType(aCurSegment.iRoadType)
        {
        auto end = aPrevSegment.iPath.End();
        if (end.second)
            iPosition = end.first;
        }

    /** Creates a turn from aPrevSegment, optionally to aCurSegment if it is non-null, with a distance in meters and a time in seconds. */
    TNavigatorTurn(const CRouteSegment& aPrevSegment,const CRouteSegment* aCurSegment,double aDistance,double aTime):
        TTurn(),
        iDistance(aDistance),
        iTime(aTime),
        iFromName(aPrevSegment.iName),
        iFromRef(aPrevSegment.iRef),
        iFromRoadType(aPrevSegment.iRoadType)
        {
        if (aCurSegment)
            *this = TNavigatorTurn(aPrevSegment,*aCurSegment,aDistance,aTime);
        else
            {
            auto end = aPrevSegment.iPath.End();
            if (end.second)
                iPosition = end.first;
            }
        }

    /** Sets the turn to its just-constructed state. */
    void Clear()
        {
        TTurn::Clear();
        iDistance = 0;
        iFromRoadType = TRoadType::UnknownMajor;
        iToRoadType = TRoadType::UnknownMajor;
        iFromName.Clear();
        iFromRef.Clear();
        iToName.Clear();
        iToRef.Clear();
        iPosition.iX = iPosition.iY = 0;
        }

    /** Determines if two turns can be merged. */
    bool CanMerge(const TNavigatorTurn& aFollowingTurn,const TRouteProfile& aProfile) const;
    /** Merges a turn with a following turn if possible. */
    bool MergeIfPossible(const TNavigatorTurn& aFollowingTurn,const TRouteProfile& aProfile);
    /** Merges a turn with a following turn. */
    void Merge(const TNavigatorTurn& aFollowingTurn,const TRouteProfile& aProfile);
    /** Creates instructions for this turn and sets them in the turn object. */
    void CreateInstructions(std::shared_ptr<CEngine> aEngine,const TRouteProfile& aProfile,const char* aLocale,bool aMetricUnits,bool aAbbreviate);
    /** Returns instructions for this turn. */
    CString Instructions(std::shared_ptr<CEngine> aEngine,const TRouteProfile& aProfile,const char* aLocale,bool aMetricUnits,bool aAbbreviate,TNavigationState aState = TNavigationState::Turn,const TNavigatorTurn* aNextTurn = nullptr) const;
    /**
    Creates a diagram of the turn: either a line with a bend in it,
    or two lines connected to a circle representing a roundabout.
    The parameter aEngine is needed to create a graphics context.
    The parameter aSizeInPixels is the width and height of a square enclosing the
    diagram. It is clamped to a minimum of 12.
    aColor is the color of the lines in the diagram.
    If the route profile obeys one-way restrictions a roundabout circle is drawn when the turn enters a roundabout.
    That rule prevents roundabouts from being drawn for walking routes, which may go the wrong way round them.
    */
    CBitmap Diagram(std::shared_ptr<CEngine> aEngine,const TRouteProfile& aProfile,int32_t aSizeInPixels,TColor aColor) const;

    /** The distance in meters from the current position to the turn or the arrival point. */
    double iDistance = 0;
    /** The estimated time in seconds from the current position to the turn or arrival point. */
    double iTime = 0;
    /** The name, if known, of the road before the turn. */
    CString iFromName;
    /** The reference code, if known, of the road before the turn. */
    CString iFromRef;
    /** The type of the road before the turn. */
    TRoadType iFromRoadType = TRoadType::UnknownMajor;
    /** The name, if known, of the road after the turn. */
    CString iToName;
    /** The reference code, if known, of the road after the turn. */
    CString iToRef;
    /** The type of the road after the turn. */
    TRoadType iToRoadType = TRoadType::UnknownMajor;
    /** The position of the turn in map coordinates. */
    TPoint iPosition;
    /** The localized turn instructions. */
    CString iInstructions;
    };

/**
Create an object of a class derived from MNavigatorObserver to handle
navigation events like turn instructions.
*/
class MNavigatorObserver
    {
    public:
    virtual ~MNavigatorObserver() { }

    /** This message updates the current route. */
    virtual void OnRoute(const CRoute* /*aRoute*/) {  }

    /**
    This message supplies up to three turns.

    aFirstTurn is the first significant turn after the current position, ignoring 'ahead' and 'continue' turns.
    If its type is TTurnType::None there are no remaining significant turns.
    aFirstTurn.iDistance is the distance from the current position to the first turn or the arrival point.

    aSecondTurn, if non-null, indicates that there is a significant turn 100 metres or less after aFirstTurn.
    aSecondTurn->iDistance is the distance from the first turn to the second turn.

    aContinuationTurn, if non-null, signals that there is an 'ahead' or 'continue' turn before aFirstTurn, so that
    the navigation system can show that no turn is required at the next junction or adjoining minor road.
    aContinuationTurn->iDistance is the distance from the current position to the continuation turn.
    */
    virtual void OnTurn(const TNavigatorTurn& /*aFirstTurn*/,const TNavigatorTurn* /*aSecondTurn*/,const TNavigatorTurn* /*aContinuationTurn*/) { }

    /** This message updates the state. */
    virtual void OnState(TNavigationState /*aState*/) { }
    };

/**
Basic data received from a navigation device such as a GPS receiver.
The only essential piece of data is the position. The time is set
automatically if not supplied, and the speed and course are calculated
from successive positions. The height (elevation in meters) may be supplied
but is used only when tracking (storing a log of positions) and is
not essential for that function.
*/
class TNavigationData
    {
    public:
    /** A bit value for flags in iValidity indicating that the time is valid. */
    static constexpr uint32_t KTimeValid = 1;
    /** A bit value for flags in iValidity indicating that the position is valid. */
    static constexpr uint32_t KPositionValid = 2;
    /** A bit value for flags in iValidity indicating that the speed is valid. */
    static constexpr uint32_t KSpeedValid = 4;
    /** A bit value for flags in iValidity indicating that the course is valid. */
    static constexpr uint32_t KCourseValid = 8;
    /** A bit value for flags in iValidity indicating that the time is valid. */
    static constexpr uint32_t KHeightValid = 16;
    
    /** Flags indicating validity or availability of data. */
    uint32_t iValidity = 0;
    /** The time in seconds: the number of seconds since 00:00 on the 1st January 1970, UTC. */
    double iTime = 0;
    /** Position in degrees latitude and longitude. */
    TPointFP iPosition;
    /** Speed in kilometres per hour. */
    double iSpeed = 0;
    /** True course in degrees. */
    double iCourse = 0;
    /** Height above sea level in meters. */
    double iHeight = 0;
    };

/** Parameters governing navigation behavior. */
class TNavigatorParam
    {
    public:
    /** Minimum distance between location fixes in metres that is taken as an actual move. */
    int32_t iMinimumFixDistance = 5;
    /** Maximum distance from the route in metres before the vehicle is deemed off-route. */
    int32_t iRouteDistanceTolerance = 20;
    /** Maximum time off route in seconds before a new route needs to be calculated. */
    int32_t iRouteTimeTolerance = 30;
    /**
    True if route is recalculated automatically when the vehicle goes off route (the default).
    If not, the state TNavigationState::ReRouteNeeded is entered and the route is recalculated the next time
    the user provides a navigation fix. This gives the application time to
    issue a warning if the routing algorithm is likely to be slow.
    */
    bool iAutoReRoute = true;
    /**
    If true, and if there is a route, the position on route is updated and turn information is created, when a position update arrives.
    If false, the vehicle position and speed are updated but other behaviour is the same as if there is no route.
    */
    bool iNavigationEnabled = true;
    };

/** Parameters used when matching a road or other feature to a location. */
class TLocationMatchParam
    {
    public:
    /** Converts zeros to default values and clamps other values to legal ranges. */
    void Normalize();
    /** Returns a normalized version of this object. */
    TLocationMatchParam Normalized() const { TLocationMatchParam p = *this; p.Normalize(); return p; }

    /**
    The accuracy of a location fix given as a range error with 95% probability.
    It is clamped to the range 1 ... 1000.
    A value of zero indicates the default value: 8.
    */
    double iLocationAccuracyInMeters = 0;
    /**
    The accuracy of a heading or course given as an angular error in degrees with 95% probability.
    It is clamped to the range 1 ... 90.
    A value of zero indicates the default value: 22.5.
    */
    double iHeadingAccuracyInDegrees = 0;
    /**
    The expected maximum distance of a road from the current location.
    It is clamped to the range 5 ... 10,000.
    A value of zero indicates the default value: 100.
    */
    double iMaxRoadDistanceInMeters = 0;
    };

/** A point on a route with heading and location match parameters. */
class TRoutePoint
    {
    public:
    /** The point. */
    TPointFP iPoint;
    /** The heading in degrees clockwise from north. */
    double iHeading = 0;
    /** True if the heading is known. */
    bool iHeadingKnown = false;
    /** Parameters used when matching the point to a road or other routable segment. */
    TLocationMatchParam iLocationMatchParam;
    };

/**
A set of points for creating a route, with optional heading and accuracy information.
Headings are used where available, and where possible, to decide between
roads or choose the direction of travel on a road.
*/
class TRouteCoordSet
    {
    public:
    /** Creates a TRouteCoordSet with coordinates in degrees. */
    TRouteCoordSet() { }
    /** Creates a TRouteCoordSet with a specified coordinate type. */
    explicit TRouteCoordSet(TCoordType aCoordType): iCoordType(aCoordType) {  }
    /** Creates a TRouteCoordSet by copying another one. */
    TRouteCoordSet(const TRouteCoordSet& aOther) = default;
    /** Creates a TRouteCoordSet by moving data from another one. */
    TRouteCoordSet(TRouteCoordSet&& aOther) = default;
    /** Creates a TRouteCoordSet from a TCoordSet. */
    TRouteCoordSet(const TCoordSet& aCs,TCoordType aCoordType,const TLocationMatchParam& aParam);
    /** Creates a TRouteCoordSet from a std::vector of points. */
    TRouteCoordSet(const std::vector<TPointFP>& aPointArray,TCoordType aCoordType,const TLocationMatchParam& aParam);
    /** Creates a TRouteCoordSet from an array of points. */
    TRouteCoordSet(const TPointFP* aPointArray,size_t aCount,TCoordType aCoordType,const TLocationMatchParam& aParam);
    /** The assignment operator. */
    TRouteCoordSet& operator=(const TRouteCoordSet& aOther) = default;
    /** The move assignment operator. */
    TRouteCoordSet& operator=(TRouteCoordSet&& aOther) = default;

    /** The coordinate type of the route points. */
    TCoordType iCoordType = TCoordType::Degree;
    /** The route points. */
    std::vector<TRoutePoint> iRoutePointArray;
    };

/** An iterator allowing a route to be traversed. */
class TRouteIterator
    {
    public:
    /** Creates a route iterator referring to a certain route and sets it to the start of the route. */
    TRouteIterator(const CRoute& aRoute);
    /** Moves forward by aDistance. Returns false if the end of the path is reached, true if not. */
    bool Forward(double aDistance) { return iPathIter.Forward(aDistance); }
    /** Returns the current position in map coordinates. */
    const TPoint& Position() const { return iPathIter.Position(); }
    /** Returns the current direction in radians, clockwise from straight up, as an angle on the map, not a geodetic azimuth. */
    double Direction() const { return iPathIter.Direction(); }

    private:
    TPathIterator iPathIter;
    };

/**
Traffic information.
This information is normally used in combination with a location reference.
*/
class CTrafficInfo
    {
    public:
    /** Writes this object in XML as a TrafficInfo element. */
    void WriteAsXml(MOutputStream& aOutput) const;

    /** Speeds of this value or greater mean there is no speed limit. */
    static constexpr double KNoSpeedLimit = 255;

    /** Vehicle types affected by this information, taken from the KArc... bit definitions; use KArcAllVehicles to select all vehicle types. */
    uint32_t iVehicleTypes = KArcAllVehicles;
    /** Permitted or expected speed in kph: KNoSpeedLimit or greater means no speed limit; 0 or less means the route is forbidden. */
    double iSpeed = KNoSpeedLimit;
    };

/** The side of the road: used in traffic information. */
enum class TSideOfRoad
    {
    None,
    Right,
    Left,
    Both
    };

/** The orientation of a path along a road: used in traffic information. */
enum class TRoadOrientation
    {
    None,
    Forwards,
    Backwards,
    Both,
    };

/** The type of a location reference used for traffic information. */
enum class TLocationRefType
    {
    /** A line in the route network. */
    Line,
    /** A point on the earth's surface. */
    GeoCoordinate,
    /** A point on a line in the route network. */
    PointAlongLine,
    /** A point on the route network providing access to a nearby POI: the first point is the point on the line, the second is the POI. */
    PointWithAccessPoint,
    /** A circle defined as a point and a radius. */
    Circle,
    /** A rectangle aligned to the grid of latitude and longitude: it is defined using two points at opposite corners. */
    Rectangle,
    /** A polygon defined using a set of points. */
    Polygon,
    /** A closed line in the route network. */
    ClosedLine
    };

/** Parameters used for a location reference when writing traffic information. */
class CLocationRef
    {
    public:
    /** Creates a location reference with the specified reference type and coordinate type. */
    CLocationRef(TLocationRefType aType,TCoordType aCoordType):
        iType(aType),
        iGeometry(aCoordType),
        iRadiusInMeters(0),
        iSideOfRoad(TSideOfRoad::None),
        iRoadOrientation(TRoadOrientation::None)
        {
        }

    void WriteAsXml(CFramework& aFramework,MOutputStream& aOutput) const;

    /** The type of this location reference. */
    TLocationRefType iType;
    /** The arbitrary ID of the location reference. */
    CString iId;
    /** The point or points. */
    CGeometry iGeometry;
    /** The radius, if this is a circle. */
    double iRadiusInMeters;
    /** The side of the road, if relevant. */
    TSideOfRoad iSideOfRoad;
    /** The road orientation, if relevant. */
    TRoadOrientation iRoadOrientation;
    };

/** A matrix of route times and distances between sets of points. */
class CTimeAndDistanceMatrix
    {
    public:
    /** The default constructot. */
    CTimeAndDistanceMatrix() = default;
    /** The copy constructor */
    CTimeAndDistanceMatrix(CTimeAndDistanceMatrix&) = default;
    /** The move constructor. */
    CTimeAndDistanceMatrix(CTimeAndDistanceMatrix&&) = default;
    /** Constructs the object from from-count, to-count and raw matrix. */
    CTimeAndDistanceMatrix(size_t aFromCount,size_t aToCount,std::vector<uint32_t>&& aMatrix):
        m_from_count(aFromCount),
        m_to_count(aToCount),
        m_matrix(std::move(aMatrix))
        {
        if (aFromCount * aToCount * 2 != m_matrix.size())
            throw KErrorInvalidArgument;
        }
    /** The copy assignment operator */
    CTimeAndDistanceMatrix& operator=(const CTimeAndDistanceMatrix&) = default;
    /** The move assignment operator. */
    CTimeAndDistanceMatrix& operator=(CTimeAndDistanceMatrix&&) = default;
    /** Returns the number of 'from' points. */
    size_t FromCount() const { return m_from_count; }
    /** Returns the number of 'to' points. */
    size_t ToCount() const { return m_to_count; }
    /** Returns the time in seconds taken to travel on the best route from one of the 'from' points to one of the 'to' points. */
    uint32_t Time(size_t aFromIndex,size_t aToIndex) const { return m_matrix[(aFromIndex * m_to_count + aToIndex) * 2]; }
    /** Returns the distance in metres along the best route from one of the 'from' points to one of the 'to' points. */
    uint32_t Distance(size_t aFromIndex,size_t aToIndex) const { return m_matrix[(aFromIndex * m_to_count + aToIndex) * 2 + 1]; }

    private:
    size_t m_from_count = 0;
    size_t m_to_count = 0;
    std::vector<uint32_t> m_matrix;
    };

/**
The accessibility of a point for routing purposes.
The CFramework::RouteAccess function, if implemented for the current router,
returns a value of this type.

A point is defined as accessible if it is connected to at least 1000 other route segments;
thus accessibility is not defined correctly for very small maps.
*/
enum class TRouteAccess
    {
    /** The accessibility of the point is unknown. */
    Unknown,
    /** The point is accessible for outward routing. */
    Accessible,
    /**
    Routes cannot leave the point. The usual cause is
    an error in the map data like a one-way road that is a dead end.
    */
    Isolated,
    /** There are no routable roads or tracks near the point. */
    NoRoad
    };

}

#endif
