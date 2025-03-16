#pragma once

#include <gdfe/core.h>
#include <gdfe/os/io.h>
#include <string.h>

#include "map.h"

// what im thinking:
/*
save in .gdf file with format:
CAN_FLY=3
NOCLIP=2
DRAW_WIREFRAME=6
a=b where
a is type GDF_MKEY converted into a string with GDF_MKEY_ToString(...);
b is of type GDF_MapEntry

*/

#ifdef __cplusplus
extern "C" {
#endif

/* TODO! this is the hard part :sob: */
// serializes into format .gdf
GDF_BOOL GDF_SerializeMap(GDF_Map* map, char* out_buf);
// deserializes from format .gdf
GDF_BOOL GDF_DeserializeToMap(char* data, GDF_Map* out_map);

GDF_BOOL GDF_WriteMapToFile(GDF_Map* map, const char* rel_path);
GDF_BOOL GDF_ReadMapFromFile(const char* rel_path, GDF_Map* out_map);

#ifdef __cplusplus
}
#endif