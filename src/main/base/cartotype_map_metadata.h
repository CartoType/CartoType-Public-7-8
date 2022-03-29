/*
cartotype_map_metadata.h
Copyright (C) 2021 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_MAP_METADATA_H__
#define CARTOTYPE_MAP_METADATA_H__

#include <cartotype_base.h>

namespace CartoType
{

/** Identifiers of tables in CTM1 map files. */
class TMapTableType
    {
    public:
    /** The ID of the global information table. */
    static constexpr uint16_t KGlobalTable = 0;
    /** The ID of the table containing the layers containing the map objects; see also KLowResolutionLayerTable. */
    static constexpr uint16_t KLayerTable = 1;
    /** The ID of the obsolete text index table. */
    static constexpr uint16_t KTable2Obsolete = 2;
    /** The ID of the obsolete projection table used up to CTM1 version 3.0. */
    static constexpr uint16_t KTable3Obsolete = 3;
    /** The ID of the the table containing the map projection. */
    static constexpr uint16_t KProjectionTable = 4;
    /** The ID of the table of compressed strings referenced by the map objects. */
    static constexpr uint16_t KStringTable = 5;
    /** The ID of the table containing layer data containing map objects at lower resolutions, simplified for display at smaller scales; see also KLayerTable. */
    static constexpr uint16_t KLowResolutionLayerTable = 6;
    /** The ID of the table containing color palettes for raster image objects. */
    static constexpr uint16_t KPaletteTable = 7;
    /** The ID of the table containing the text index used when searching for string attributes. */
    static constexpr uint16_t KTextIndexTable = 8;
    /** The ID of the table containing the obsolete serialised A-star routing network used up to format version 4.1. */
    static constexpr uint16_t KTable9Obsolete = 9;
    /** The ID of the A-star routing data table. */
    static constexpr uint16_t KRouteTableAStar = 10;
    /** The ID of the contraction hierarchy routing data table. */
    static constexpr uint16_t KRouteTableCH = 11;
    /** The ID of the table containing extra information used for A-star routing. */
    static constexpr uint16_t KRouteTableExtra = 12;
    /** The ID of the turn-expanded routing data table. */
    static constexpr uint16_t KRouteTableTurnExpanded = 13;
    /** The ID of the table containing contraction hierarchy routing data that can optionally be used stand-alone, in combination with other map files. */
    static constexpr uint16_t KRouteTableCHStandAlone = 14;
    /** The ID of the turn-expanded contraction hierarchy routing data table. */
    static constexpr uint16_t KRouteTableTECH = 15;
    /** The ID of the table containing contraction hierarchy routing data that is a tile that can be used with other tiles from the same data set. */
    static constexpr uint16_t KRouteTableCHTiled = 16;
    /** The ID of the table containing turn-expanded contraction hierarchy routing data that is a tile that can be used with other tiles from the same data set. */
    static constexpr uint16_t KRouteTableTECHTiled = 17;
    /** The ID of the table containing compact turn-expanded routing data, which uses less run-time RAM. */
    static constexpr uint16_t KRouteTableTurnExpandedCompact = 18;
    /** An ID used when no table exists or the table type is unknown. */
    static constexpr uint16_t KNoTable = UINT16_MAX;
    };

/**
The format used for points in CTM1 data.
These numbers must fit into 8 bits for the moment because of the way they are stored in the CTM1 file.
The values are discontinuous because several obsolete values have been discarded.
*/
enum class TPointFormat
    {
    /** An arbitrary or unknown point format. */
    Unknown = 0,

    /** Units are projected map meters. */
    Meter = 2,

    /**
    Units are degrees as 11.21 fixed-point numbers.
    That is, there are 2^21 (2097152) units to a degree.
    This format is used only by the makemap tool, for representing
    unprojected map data.
    */
    ScaledDegree = 7,

    /**
    Units are 32nds of projected map meters.
    This is the default format for CTM1 data.
    */
    Meter32nds = 8
    };

/** A data version. */
class TDataVersion
    {
    public:
    /** The major part of the version. */
    uint16_t iMajor = 0;
    /** The minor part of the version. */
    uint16_t iMinor = 0;
    };

/** Metadata describing a CTM1 map file. */
class CMapMetaData
    {
    public:
    /** The CTM1 format version. */
    TDataVersion iFileVersion;
    /** The version of CartoType used to build the makemap tool which created the CTM1 file. */
    TDataVersion iCartoTypeVersion;
    /** The build (version control revision number) used to build the makemap tool which created the CTM1 file. */
    int32_t iCartoTypeBuild = 0;
    /** The name of the map data set. */
    std::string iDataSetName;
    /** The copyright notice applying to the map data. */
    std::string iCopyright;
    /** The name of the map projection. */
    std::string iProjectionName;
    /** The Proj4 parameters for the map projection. */
    std::string iProjectionParameters;
    /** The point format for map coordinates: either Meter or Meter32nds. */
    TPointFormat iPointFormat;
    /** The axis-aligned bounds of the map, in map coordinates. */
    TRect iExtentInMapCoords;
    /** The axis-aligned bounds of the map, in degrees of longitude and latitude. */
    TRectFP iExtentInDegrees;
    /** The route table type: one of the constants defined in TMapTableType. */
    uint16_t iRouteTableType = TMapTableType::KNoTable;
    /** True if the route data contains gradients. */
    bool iRouteDataHasGradients = false;
    /** True if file positions in the data file take up 5 bytes rather than 4. */
    bool iLargeFile = false;
    /** True if the driving side (rule of the road) is known. */
    bool iDrivingSideKnown = false;
    /** True if the driving side is known and the rule is to drive on the left. */
    bool iDriveOnLeft = false;
    };

}

#endif
