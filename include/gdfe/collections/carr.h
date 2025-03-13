#pragma once

#include <gdfe/core.h>

typedef struct GDF_CArray_T* GDF_CArray;

#define GDF_CArray(type) GDF_CArray

GDF_CArray __create_carray(u32 stride, u32 capacity);
// Creates a cyclic array structure. 
// Meant to be a circular buffer with minimal allocations and in-place element modification.
#define GDF_CArrayCreate(type, capacity) \
    __create_carray(sizeof(type), capacity);

// Returns a pointer to the next element of the array for writing.
// Immediately after this is called, CArrayReadNext will be able to 
// read this element, whether the value was modified or not.
void* GDF_CArrayWriteNext(GDF_CArray arr);
// Returns a pointer to the next element of the array for reading.
// If the next element has not been returned from CArrayWriteNext yet,
// this function will return NULL.
const void* const GDF_CArrayReadNext(GDF_CArray arr);
// Returns the pointer to the stored data. User decides what to do with it.
void* GDF_CArrayGetData(GDF_CArray arr);