/*
cartotype_address.h
Copyright (C) 2013-2020 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_ADDRESS_H__
#define CARTOTYPE_ADDRESS_H__

#include <cartotype_string.h>
#include <cartotype_path.h>

namespace CartoType
{

class CMapObject;
class CStreetAddressSearchStateImplementation;

/** A structured address for use with FindAddress. */
class CAddress
    {
    public:
    /** Clears an address to its just-constructed state. */
    void Clear();
    /**
    Gets the address as a string.
    If aFull is true supplies the main administrative division
    (state, province, etc.) and country.
    If aLocation is non-null, prefixes that location in degrees to the address.
    */
    CString ToString(bool aFull = true,const TPointFP* aLocation = nullptr) const;
    /**
    Gets the elements of an address.
    Labels each element with its category (e.g., 'building', 'feature', 'street').
    */
    CString ToStringWithLabels() const;
    
    /** The name or number of the building. */
    CString iBuilding;
    /** The name of a feature or place of interest. */
    CString iFeature;
    /** The street, road or other highway. */
    CString iStreet;
    /** The suburb, neighborhood, quarter or other subdivision of the locality. */
    CString iSubLocality;
    /** The village, town or city. */
    CString iLocality;
    /** The name of an island. */
    CString iIsland;
    /**
    The subsidiary administrative area: county, district, etc.
    By preference this is a level-6 area in the OpenStreetMap classification.
    Levels 7, 8 and 5 are used in that order if no level-6 area is found.
    */
    CString iSubAdminArea;
    /**
    The administrative area: state, province, etc.
    By preference this is a level-4 area in the OpenStreetMap classification.
    Level 3 is used if no level-4 area is found.
    */
    CString iAdminArea;
    /** The country. */
    CString iCountry;
    /** The postal code. */
    CString iPostCode;
    };

/**
The state of a street address search.
It is used to iterate through all the objects found satisfying the search parameters.
Each iteration returns all the streets in a certain combination of country, admin area and locality.
*/
using CStreetAddressSearchState = std::unique_ptr<CStreetAddressSearchStateImplementation>;

/** Address part codes used when searching for address parts separately. */
enum class TAddressPart
    {
    /** Building names or numbers. */
    Building,
    /** Features or points of interest. */
    Feature,
    /** Streets or roads. */
    Street,
    /** Suburbs and neighborhoods. */
    SubLocality,
    /** Cities, towns and villages. */
    Locality,
    /** Islands. */
    Island,
    /** Lower-level administrative areas like counties. */
    SubAdminArea,
    /** Higher-level administrative areas like states. */
    AdminArea,
    /** Countries. */
    Country,
    /** Postal codes. */
    PostCode
    };

/**
A type used in addresses obtained by reverse geocoding.
It gives a coarse notion of what an object is.
Codes are ordered roughly from small to large.
*/
enum class TGeoCodeType
    {
    None,               ///< No geocode type.
    Position,           ///< A position not corresponding to something on the map.
    Address,            ///< An address.

    Building,           ///< A building.
    Farm,               ///< A farm.

    Footpath,           ///< A footpath.
    Cycleway,           ///< A cycleway (cycle path).
    SkiRoute,           ///< A ski route (piste or ski trail).
    WalkingRoute,       ///< A walking route.

    FerryRoute,         ///< A ferry route.
    Railway,            ///< A railway track.

    PedestrianRoad,     ///< A pedestrian road.
    Track,              ///< A track.
    ServiceRoad,        ///< A service or access road.
    ResidentialRoad,    ///< A residential road.
    UnclassifiedRoad,   ///< An unclassified road.
    TertiaryRoad,       ///< A tertiary road.
    SecondaryRoad,      ///< A secondary road.
    PrimaryRoad,        ///< A primary road.
    TrunkRoad,          ///< A trunk road.
    Motorway,           ///< A motorway (freeway).

    HikingRoute,        ///< A hiking route.
    CyclingRoute,       ///< A cycling route.

    LandFeature,        ///< A land feature: a peak, hill, cliff, quarry, etc.
    WaterFeature,       ///< A water feature: a lake, river, stream, spring, etc.

    Locality,           ///< A named locality that is not a populated place, or smaller than other populated place categories.
    Hamlet,             ///< A hamlet (very small populated place).

    PostCode,           ///< The location or area of a postal code.

    AdminArea10,        ///< A level 10 administrative area as defined by OpenStreetMap admin_level: e.g., a parish or suburb.
    AdminArea9,         ///< A level 9 administrative area as defined by OpenStreetMap admin_level: e.g., a village.
    Neighborhood,       ///< A neighborhood: a small distinct part of an urban area.
    Village,            ///< A village: an urban area with a population usually of less than a thousand.
    Suburb,             ///< A suburb: a large part of an urban area.

    Island,             ///< An island.

    AdminArea8,         ///< A level 8 administrative area as defined by OpenStreetMap admin_level: e.g., a French commune.
    Town,               ///< A town: an urban area with a population less than 100,000, but larger than a village.

    AdminArea7,         ///< A level 7 administrative area as defined by OpenStreetMap admin_level: e.g., a U.S. civil township.
    City,               ///< A city: an urban area with a population of 100,000 or more.

    AdminArea6,         ///< A level 6 administrative area as defined by OpenStreetMap admin_level: e.g., a U.K. or U.S. county.
    AdminArea5,         ///< A level 5 administrative area as defined by OpenStreetMap admin_level: e.g., a region of England.
    AdminArea4,         ///< A level 4 administrative area as defined by OpenStreetMap admin_level: e.g., a U.K. constituent country or a U.S. state.
    AdminArea3,         ///< A level 3 administrative area as defined by OpenStreetMap admin_level: e.g., a federal district of Russia or a Swedish landsdel.
    AdminArea2,         ///< A level 2 administrative area as defined by OpenStreetMap admin_level: a country.
    Country,            ///< A country, including areas not defined as such by OpenStreetMap.
    AdminArea1,         ///< A level 1 administrative area as defined by OpenStreetMap admin_level: e.g., the European Union.
    Continent           ///< A continent.
    };

/** A geocode item describes a single map object. */
class CGeoCodeItem
    {
    public:
    /** The geocode type of the item. */
    TGeoCodeType iGeoCodeType = TGeoCodeType::None;

    /**
    The name of the object in the locale used when requesting a geocode.
    For buildings, this may be a building number.
    */
    CString iName;
    /** The postal code if any. */
    CString iPostCode;
    };

}

#endif
