/*
cartotype_road_type.h
Copyright (C) 2013-2019 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_ROAD_TYPE_H__
#define CARTOTYPE_ROAD_TYPE_H__

#include <cartotype_types.h>

namespace CartoType
{

/**
Masks, flags and shifts used for road types.
The level, bridge and tunnel flags are also used in the integer attributes of objects other than roads.

Bit assignments:

bit 0: tunnel;
bit 1: toll;
bit 2: roundabout;
bit 3: restricted turn exists - see the _R string attribute for details;
bit 4: one way forward;
bit 5: one way backward;
bit 6: link;
bit 7: lower grade road;
bits 8...11: major road type;
bits 12...15: level;
bit 16: bridge;
bits 17...31: road access flags;
*/
namespace TRoadTypeFlag
    {
    /** The toll, roundabout, restriction and direction flags are held in bits 1...5. */
    constexpr uint32_t RoutingMask = 0x3E;

    /** The road type is held in bits 6...11. */
    constexpr uint32_t RoadTypeMask = 0xFC0;

    /** The major road type is held in bits 8...11; bits 7 and 6 are used for finer distinctions. */
    constexpr uint32_t MajorRoadTypeMask = 0xF00;

    /**
    The level (0 = surface, positive = above surface, negative = below surface)
    is held as a signed nybble in bits 12...15. Levels above 0 are not necessarily
    bridges; they can be embankments or other raised structures, and levels below 0 are
    not necessarily tunnels. There are separate bits to mark bridges and tunnels:
    BridgeFlag and TunnelFlag.

    The level mask is also used for levels in the 'Type' attribute of non-road objects.
    */
    constexpr uint32_t LevelMask = 0xF000;

    /** A bit flag to identify tunnels. */
    constexpr uint32_t TunnelFlag = 1;

    /** A bit flag to identify bridges. */
    constexpr uint32_t BridgeFlag = 0x10000;

    /** The number of bits by which levels are shifted. */
    constexpr uint32_t LevelShift = 12;

    /** A bit flag to allow access ramps to be marked as sub-types of a major road type. */
    constexpr uint32_t RampFlag = 64;

    /** A synonym for RampFlag. */
    constexpr uint32_t LinkFlag = RampFlag;

    /**
    A bit flag to allow a distinction to be made between grades of road, within the major road type.
    It allows OSM data to distinguish trunk roads from primary roads in a backward-compatible way.
    */
    constexpr uint32_t LowerGradeFlag = 128;

    /**
    A toll must be paid to use this road.
    More details may be available in string attributes.
    */
    constexpr uint32_t TollFlag = 2;

    /** A bit flag indicating that the road is part of a roundabout. */
    constexpr uint32_t RoundaboutFlag = 4;

    /**
    A bit flag indicating that the road has one or more restrictions (e.g., right turn only). The details
    of the restrictions are stored in string attributes.
    */
    constexpr uint32_t RestrictionFlag = 8;

    /** A bit mask for the road direction and driving side. */
    constexpr uint32_t DirectionMask = 16 | 32;

    /** The number of bits by which the road direction and driving side field is shifted. */
    constexpr uint32_t DirectionShift = 4;

    /** A road direction value indicating two-way traffic, driving on the right. */
    constexpr uint32_t DriveOnRight = 0;

    /** A road direction value indicating two-way traffic, driving on the left. */
    constexpr uint32_t DriveOnLeft = 48;

    /** A road direction value indicating that the road is one-way in the direction in which it is defined. */
    constexpr uint32_t OneWayForward = 16;

    /** A road direction value indicating that the road is one-way in the reverse direction to that in which it is defined. */
    constexpr uint32_t OneWayBackward = 32;

    /** The first raised level: the default level for a bridge or overpass. */
    constexpr uint32_t FirstOverLevel = 0x1000;

    /** The first sub-surface level: the default value for an underpass. */
    constexpr uint32_t FirstUnderLevel = 0xF000;

    /** The second sub-surface level: the default value for a tunnel. */
    constexpr uint32_t SecondUnderLevel = 0xE000;

    /**
    Access rules.
    These flags occupy the bits used for the OSM type in non-road layers,
    so may be used only where an OSM type is not used.
    Each flag forbids access to a certain type of vehicle.
    */
    constexpr uint32_t AccessMask = 0xFFFE0000;

    /** Access is forbidden to bicycles.*/
    constexpr uint32_t BicycleAccessFlag = 0x20000;

    /** Access is forbidden to motorcycles.*/
    constexpr uint32_t MotorCycleAccessFlag = 0x40000;

    /** Access is forbidden to motor cars.*/
    constexpr uint32_t MotorCarAccessFlag = 0x80000;

    /** Access is forbidden to high-occupancy vehicles.*/
    constexpr uint32_t HighOccupancyAccessFlag = 0x100000;

    /** Access is forbidden to light goods vehicles.*/
    constexpr uint32_t GoodsAccessFlag = 0x200000;

    /** Access is forbidden to heavy goods vehicles.*/
    constexpr uint32_t HeavyGoodsAccessFlag = 0x400000;

    /** Access is forbidden to buses.*/
    constexpr uint32_t BusAccessFlag = 0x800000;

    /** Access is forbidden to taxis.*/
    constexpr uint32_t TaxiAccessFlag = 0x1000000;

    /** Access is forbidden to pedestrians.*/
    constexpr uint32_t PedestrianAccessFlag = 0x2000000;

    /** Access is forbidden to agricultural vehicles.*/
    constexpr uint32_t AgriculturalAccessFlag = 0x4000000;

    /** Access is forbidden to forestry vehicles.*/
    constexpr uint32_t ForestryAccessFlag = 0x8000000;

    /** Access is forbidden to emergency vehicles.*/
    constexpr uint32_t EmergencyAccessFlag = 0x10000000;

    /** Other access restrictions exist such as weight, length, width, height or for vehicles carrying hazardous materials. */
    constexpr uint32_t OtherAccessFlag = 0x20000000;

    /** Access is forbidden to wheelchairs.*/
    constexpr uint32_t WheelChairAccessFlag = 0x40000000;

    /** Access is forbidden to vehicles with disabled persons' permits.*/
    constexpr uint32_t DisabledAccessFlag = 0x80000000;

    /** Access is forbidden to all vehicles. */
    constexpr uint32_t VehicleAccessMask = 0xFFFE0000 & ~(PedestrianAccessFlag | WheelChairAccessFlag);

    /** Access is forbidden to motor vehicles. */
    constexpr uint32_t MotorVehicleAccessMask = 0xFFFE0000 & ~(PedestrianAccessFlag | BicycleAccessFlag | WheelChairAccessFlag);

    /** Access is forbidden to public service vehicles (buses and taxis). */
    constexpr uint32_t PublicServiceVehicleRoadAccessMask = BusAccessFlag | TaxiAccessFlag;
    }

/**
Road types. Note that items of type TRoadType are often combined with
other TRoadTypeFlag values. Therefore it is often necessary to mask a TRoadType with
TRoadTypeFlags::RoadTypeMask to obtain one of the enumerated TRoadType values.
*/
enum class TRoadType
    {
    /** Roads of unknown type. */
    UnknownMajor = 0,

    /** Primary road with limited access (motorway, freeway, etc.). */
    PrimaryLimitedAccess = 0x100,

    /** Primary road with limited access (motorway, freeway, etc.). */
    Motorway = PrimaryLimitedAccess,

    /** Access ramp or link to primary road with limited access (motorway, freeway, etc.). */
    MotorwayLink = PrimaryLimitedAccess | TRoadTypeFlag::LinkFlag,

    /** Primary road: UK trunk road. */
    PrimaryUnlimitedAccess = 0x200,

    /** Primary road: UK trunk road. */
    Trunk = PrimaryUnlimitedAccess,

    /** Access ramp or link to trunk road. */
    TrunkLink = Trunk | TRoadTypeFlag::LinkFlag,

    /** Primary road: UK 'A' road. */
    Primary = PrimaryUnlimitedAccess | TRoadTypeFlag::LowerGradeFlag,

    /** Access ramp or link to primary road. */
    PrimaryLink = Primary | TRoadTypeFlag::LinkFlag,

    /** Secondary road: UK 'B' road. */
    Secondary = 0x300,

    /** Access ramp or link to secondary road. */
    SecondaryLink = Secondary | TRoadTypeFlag::LinkFlag,

    /** Tertiary road: one level above minor and residential roads. */
    Tertiary = Secondary | TRoadTypeFlag::LowerGradeFlag,

    /** Access ramp or link to tertiary road. */
    TertiaryLink = Tertiary | TRoadTypeFlag::LinkFlag,

    /** Local road or town or city street. */
    Minor = 0x400,

    /** Local road or town or city street. */
    Unclassified = Minor,

    /** Narrow road or street with residential housing. */
    Residential = Minor | TRoadTypeFlag::LowerGradeFlag,

    /** Byway: road or track open to motor vehicles. */
    Byway = 0x500,

    /** Byway: road or track open to motor vehicles. */
    Track = Byway,

    /** Access ramp to limited access road. */
    AccessRamp = 0x600,

    /** Service road or access road. */
    Service = 0x700,

    /** Vehicular Ferry route. */
    VehicularFerry = 0x800,

    /** Passenger-only Ferry route. */
    PassengerFerry = 0x900,

    /** Path or walkway for pedestrians. */
    Path = 0xA00,

    /** Path or walkway for pedestrians. */
    Pedestrian = Path,

    /** Stairway or escalator for pedestrians. */
    Stairway = 0xB00,

    /** Cycle path. */
    CyclePath = 0xC00,

    /** Footpath. */
    FootPath = 0xD00,

    /** User-defined road type 0. */
    Other0 = 0xE00,

    /** User-defined road type 1. */
    Other1 = 0xE00 | TRoadTypeFlag::LowerGradeFlag,

    /** User-defined road type 2. */
    Other2 = 0xE00 | TRoadTypeFlag::LinkFlag,

    /** User-defined road type 3. */
    Other3 = 0xE00 | TRoadTypeFlag::LowerGradeFlag | TRoadTypeFlag::LinkFlag,

    /** User-defined road type 4. */
    Other4 = 0xF00,

    /** User-defined road type 5. */
    Other5 = 0xF00 | TRoadTypeFlag::LowerGradeFlag,

    /** User-defined road type 6. */
    Other6 = 0xF00 | TRoadTypeFlag::LinkFlag,

    /** User-defined road type 7. */
    Other7 = 0xF00 | TRoadTypeFlag::LowerGradeFlag | TRoadTypeFlag::LinkFlag,
    };

/** Returns true if a road type stored in an integer represents a one-way road. */
inline bool RoadTypeIsOneWay(uint32_t aRoadType)
    {
    return ((aRoadType & TRoadTypeFlag::OneWayForward) << 1) != (aRoadType & TRoadTypeFlag::OneWayBackward);
    }

}

#endif
