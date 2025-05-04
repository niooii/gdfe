#pragma once

#include <gdfe/prelude.h>

// TODO! replace some stuff with platform specific implementations 
// maybe later tho

// TAGGING NAD STATISTICS ARE NOT IMPLEMENTED RN
typedef enum GDF_MEMTAG {
    // For temporary use. Should be assigned one of the below or have a new tag created.
    GDF_MEMTAG_UNKNOWN,
    GDF_MEMTAG_ARRAY,
    GDF_MEMTAG_COLLECTION,
    GDF_MEMTAG_DICT,
    GDF_MEMTAG_RING_QUEUE,
    GDF_MEMTAG_BST,
    GDF_MEMTAG_STRING,
    GDF_MEMTAG_APPLICATION,
    GDF_MEMTAG_JOB,
    GDF_MEMTAG_TEXTURE,
    GDF_MEMTAG_MATERIAL_INSTANCE,
    GDF_MEMTAG_RENDERER,
    GDF_MEMTAG_GAME,
    GDF_MEMTAG_TRANSFORM,
    GDF_MEMTAG_ENTITY,
    GDF_MEMTAG_ENTITY_NODE,
    GDF_MEMTAG_SCENE,
    GDF_MEMTAG_TEMP_RESOURCE,
    GDF_MEMTAG_IO,
    GDF_MEMTAG_FREE,

    GDF_MEMTAG_MAX_TAGS
} GDF_MEMTAG;

EXTERN_C_BEGIN

// memory allocated with this is automatically zero'd
void* GDF_Malloc(u64 size, GDF_MEMTAG tag);

void* GDF_Realloc(void* block, u64 size);

void GDF_Free(void* block);

void GDF_Memzero(void* block, u64 size);

// TODO!
void GDF_Memcpy(void* dest, const void* src, u64 size);

// TODO!
void GDF_Memset(void* block, i32 val, u64 size);

// void GDF_GetMemUsageStr(char* out_str);

EXTERN_C_END