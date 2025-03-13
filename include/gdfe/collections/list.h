#pragma once

#include <gdfe/../gdfe/core.h>

/*
Memory layout
u64 capacity = number elements that can be held
u64 length = number of elements currently contained
u64 stride = size of each element in bytes
void* elements
*/

enum {
    LIST_CAPACITY,
    LIST_LENGTH,
    LIST_STRIDE,
    LIST_FIELD_LENGTH
};

// For easier reading of struct definitions that contain lists.
#define GDF_LIST(type) type*

void* __list_create(u64 length, u64 stride);
void __list_destroy(void* list);

u64 __list_field_get(void* list, u64 field);
void __list_field_set(void* list, u64 field, u64 value);

void* __list_resize(void* list);

void* __list_push(void* list, const void* value_ptr);
void __list_pop(void* list, void* dest);

void* __list_remove_at(void* list, u64 index, void* dest);
void* __list_insert_at(void* list, u64 index, void* value_ptr);

#define LIST_DEFAULT_CAPACITY 1
#define LIST_RESIZE_FACTOR 2

// #define GDF_LIST_Contains

#ifndef __cplusplus

#define GDF_LIST_Create(type) \
    __list_create(LIST_DEFAULT_CAPACITY, sizeof(type))

#define GDF_LIST_Reserve(type, capacity) \
    __list_create(capacity, sizeof(type))

#define GDF_LIST_Destroy(list) __list_destroy(list);

#define GDF_LIST_Push(list, value)          \
    {                                       \
        TYPEOF(value) temp = (value);         \
        list = (void*)__list_push(list, &temp);     \
    }

// Sets the out_val ptr equal to the last element in the array, then decrements length.
#define GDF_LIST_Pop(list, out_val_p) \
    __list_pop(list, out_val_p)

#define GDF_LIST_Insert(list, index, value)             \
    {                                                   \
        TYPEOF(value) temp = value;                     \
        list = (void*)__list_insert_at(list, index, &temp);     \
    }

// Removes an element at an index from the list. 
// out_val_p must be a valid pointer, or may be NULL if the removed value is to be ignored.
#define GDF_LIST_Remove(list, index, out_val_p) \
    __list_remove_at(list, index, out_val_p)

#define GDF_LIST_Clear(list) \
    __list_field_set(list, LIST_LENGTH, 0)

#define GDF_LIST_GetCapacity(list) \
    __list_field_get(list, LIST_CAPACITY)

#define GDF_LIST_GetLength(list) \
    __list_field_get(list, LIST_LENGTH)

#define GDF_LIST_GetStride(list) \
    __list_field_get(list, LIST_STRIDE)

#define GDF_LIST_SetLength(list, value) \
    __list_field_set(list, LIST_LENGTH, value)

#endif