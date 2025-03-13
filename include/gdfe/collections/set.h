#pragma once

#include <gdfe/core.h>

#include <superfasthash.h>
// TODO! move away from memcmp for comparison, use custom comparison function

static FORCEINLINE u32 _superfasthash_wrapper(const u8* data, u32 len) 
{
    return (u32)SuperFastHash((const char*)data, (int) len);
}

typedef struct GDF_Set_T* GDF_Set;

#define GDF_Set(type) GDF_Set

typedef struct SetIterator {
    void* curr;
    GDF_Set __owner;
    u32 __idx;
} SetIterator;

#ifndef __cplusplus

GDF_Set GDF_SetCreateFull(u32 stride, u32 (*hash_func)(const u8* data, u32 len), GDF_BOOL is_string, u32 initial_capacity);

#define GDF_SetCreate(type, is_string) \
    GDF_SetCreateFull(sizeof(type), NULL, is_string, 32)

#define GDF_SetReserve(type, is_string, initial_capacity) \
    GDF_SetCreateFull(sizeof(type), NULL, is_string, initial_capacity)

#define GDF_SetWithHasher(type, hash_func, is_string) \
    GDF_SetCreateFull(sizeof(type), hash_func, is_string, 32)

#define GDF_SetReserveWithHasher(type, hash_func, is_string, initial_capacity) \
    GDF_SetCreateFull(sizeof(type), hash_func, is_string, 32, initial_capacity)

#endif

GDF_BOOL GDF_SetDestroy(GDF_Set set);

// The value and value are memcpy'd on insertion.
// Returns a pointer to the inserted value (owned by set) on success or if it already existed, and NULL on an error.
// Returns NULL if the requested value is NULL. 
// already_existed may be NULL.
void* GDF_SetInsert(GDF_Set set, void* value, GDF_BOOL* already_existed);
GDF_BOOL GDF_SetContains(GDF_Set set, void* value);
// Removes the requested value from the set. 
// out_val_p must be a valid pointer, or may be NULL if the removed value is to be ignored.
// True is returned on success, GDF_FALSE is returned if the value could not be found
GDF_BOOL GDF_SetRemove(GDF_Set set, void* value, void* out_val_p);
// The amount of elements present in the set.
u32 GDF_SetLen(GDF_Set set);
// Returns a SetIterator with .curr set to the element in the set (unordered).
// Advance with GDF_SetIterNext. Functions like a linked list.
// returns NULL if the map is empty.
SetIterator GDF_SetIter(GDF_Set set);
// Advances the iterator by finding the next element in the set.
// Sets iter.val to NULL on end.
// Pushing any new values to the set while iterating is undefined behavior.
// However, removing the current value is well defined - although you may want to use SetIterConsume
// for that purpose.
void GDF_SetIterAdvance(SetIterator* iter);
// Advances the iterator by finding the next element in the set.
// This function will remove the previous element, thus making it a consuming iterator.
// Sets iter.val to NULL on end.
// prev_val_p may be NULL. If it is not NULL, after advancing the iterator, the previous value
// will be memcpy'd into prev_val_p.  
// Pushing any new values to the set while iterating is undefined behavior.
void GDF_SetIterConsume(SetIterator* iter, void* prev_val_p);