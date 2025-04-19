#pragma once

#include <gdfe/core.h>

/*
 * Memory layout
 * u64 capacity = number elements that can be held
 * u64 length = number of elements currently contained
 * u64 stride = size of each element in bytes
 * void* elements
 */

enum {
    LIST_CAPACITY,
    LIST_LENGTH,
    LIST_STRIDE,
    LIST_FIELD_LENGTH
};

// For easier reading of struct definitions that contain lists.
#define GDF_LIST(type) type*

#ifdef __cplusplus
extern "C" {
#endif

void* gdfe_list_create(u64 length, u64 stride);
void gdfe_list_destroy(void* list);

u64 gdfe_list_field_get(void* list, u64 field);
void gdfe_list_field_set(void* list, u64 field, u64 value);

void* gdfe_list_resize(void* list);

void* gdfe_list_push(void* list, const void* value_ptr);
void* gdfe_list_append(void* list, const void* value_ptr, u32 num_values);
void gdfe_list_pop(void* list, void* dest);

void* gdfe_list_remove_at(void* list, u64 index, void* dest);
void* gdfe_list_insert_at(void* list, u64 index, void* value_ptr);

#ifdef __cplusplus
    }
#endif

#define LIST_DEFAULT_CAPACITY 1
#define LIST_RESIZE_FACTOR 2

// #define GDF_LIST_Contains

#ifndef __cplusplus

/// @note Storing multiple pointers of the same list is unsafe. The pointer may be reassigned by the \code GDF_ListPush*\endcode macros.
#define GDF_ListCreate(type) \
    gdfe_list_create(LIST_DEFAULT_CAPACITY, sizeof(type))

#define GDF_ListReserve(type, capacity) \
    gdfe_list_create(capacity, sizeof(type))

#define GDF_ListDestroy(list) gdfe_list_destroy(list);

/// Makes a temporary copy of the value
#define GDF_ListPush(list, value)          \
    {                                       \
        TYPEOF(value) temp = (value);         \
        list = (void*)gdfe_list_push(list, &temp);     \
    }

/// @param list The list pointer to push to.
/// @param value_ptr The pointer of the value to be pushed. If this is NULL, then
/// no value will be pushed, but a resize will still be triggered if the list is full.
#define GDF_ListPushPtr(list, value_ptr)          \
    {                                       \
        list = (void*)gdfe_list_push(list, value_ptr);     \
    }

// If values is NULL, then the list will be resized to hold the
// new amount, however no data will be copied
#define GDF_ListAppend(list, values, num_values)          \
    {                                       \
        list = (void*)gdfe_list_append(list, values, num_values);     \
    }

// Sets the out_val ptr equal to the last element in the array, then decrements length.
#define GDF_ListPop(list, out_val_p) \
    gdfe_list_pop(list, out_val_p)

#define GDF_ListInsert(list, index, value)             \
    {                                                   \
        TYPEOF(value) temp = value;                     \
        list = (void*)gdfe_list_insert_at(list, index, &temp);     \
    }

// Removes an element at an index from the list. 
// out_val_p must be a valid pointer, or may be NULL if the removed value is to be ignored.
#define GDF_ListRemove(list, index, out_val_p) \
    gdfe_list_remove_at(list, index, out_val_p)

#define GDF_ListClear(list) \
    gdfe_list_field_set(list, LIST_LENGTH, 0)

#define GDF_ListCapacity(list) \
    gdfe_list_field_get(list, LIST_CAPACITY)

#define GDF_ListLen(list) \
    gdfe_list_field_get(list, LIST_LENGTH)

#define GDF_ListStride(list) \
    gdfe_list_field_get(list, LIST_STRIDE)

#define GDF_ListSetLen(list, value) \
    gdfe_list_field_set(list, LIST_LENGTH, value)

#endif