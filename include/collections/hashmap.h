#pragma once

#include <core.h>
#include <superfasthash.h>

// TODO! move away from memcmp for comparison, use custom comparison function

static FORCEINLINE u32 superfasthash_wrapper(const u8* data, u32 len) 
{
    return (u32)SuperFastHash((const char*)data, (int) len);
}

typedef struct GDF_HashMap_T* GDF_HashMap;

#define GDF_HashMap(key, type) GDF_HashMap

typedef struct HashmapEntry {
    void* key;
    void* val;
    GDF_HashMap owner;
} HashmapEntry;

GDF_HashMap __hashmap_create(u32 k_stride, u32 v_stride, u32 (*hash_func)(const u8* data, u32 len), bool string_keys, u32 initial_capacity);

#define GDF_HashmapCreate(k_type, v_type, string_keys) \
    __hashmap_create(sizeof(k_type), sizeof(v_type), NULL, string_keys, 32)

#define GDF_HashmapReserve(k_type, v_type, string_keys, initial_capacity) \
    __hashmap_create(sizeof(k_type), sizeof(v_type), NULL, string_keys, initial_capacity)

#define GDF_HashmapWithHasher(k_type, v_type, hash_func, string_keys) \
    __hashmap_create(sizeof(k_type), sizeof(v_type), hash_func, string_keys, 32)

#define GDF_HashmapReserveWithHasher(k_type, v_type, hash_func, string_keys, initial_capacity) \
    __hashmap_create(sizeof(k_type), sizeof(v_type), hash_func, string_keys, 32, initial_capacity)

bool GDF_HashmapDestroy(GDF_HashMap hashmap);

// The key and value are memcpy'd on insertion.
// Returns a pointer to the inserted value (owned by hashmap) on success, will update the value if the key exists.
// Returns NULL if the requested key is NULL. 
// already_existed may be NULL.
void* GDF_HashmapInsert(GDF_HashMap hashmap, void* key, void* value, bool* already_existed);
// Gets the value stored for a key.
// Returns NULL if the key does not exist, or the requested key is NULL.
void* GDF_HashmapGet(GDF_HashMap hashmap, void* key);
// Removes the requested key from the hashmap. 
// out_val_p must be a valid pointer, or may be NULL if the removed value is to be ignored.
// True is returned on success, false is returned if the key could not be found
bool GDF_HashmapRemove(GDF_HashMap hashmap, void* key, void* out_val_p);
// The amount of elements present in the hashmap.
u32 GDF_HashmapLen(GDF_HashMap hashmap);
// Returns a pointer to the first element in the hashmap (unordered).
// Advance with GDF_HashmapIterNext. Functions like a linked list.
// returns NULL if the map is empty.
// Modifying a key will result in undefined behavior.
HashmapEntry* GDF_HashmapIter(GDF_HashMap hashmap);
// Advances the "iterator" by finding the next element in the hashmap.
// Sets curr to NULL on end.
// If a hashmap is modified while an iterator is still being used, behavior is undefined.
void GDF_HashmapIterAdvance(HashmapEntry** iter);