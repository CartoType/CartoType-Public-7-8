/*
cartotype_epsg.h
Copyright (C) 2013-2018 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_EPSG_H__
#define CARTOTYPE_EPSG_H__

#include <cartotype_types.h>

namespace CartoType
{

// EPSG codes for projections.

/** The EPSG code for the Web Mercator projection. Used by TTileParam. */
constexpr int32_t KEpsgWebMercator = 3857;
/** The EPSG code for the Plate Carree projection. Used by TTileParam. */
constexpr int32_t KEpsgPlateCarree = 4326;

}

#endif
