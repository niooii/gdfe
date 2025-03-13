#include <gdfe/collections/hashmap.h>
#include <superfasthash.h>

// TODO! move away from memcmp for comparison, use custom comparison function

typedef struct GDF_HashMap_T {
    u32 k_stride;
    u32 v_stride;
    // TODO! string keys should have a different hash function i think
    GDF_BOOL string_keys;
    u32 num_entries;
    u32 capacity;
    HashmapEntry* bucket;
    u32 (*hash_func)(const u8* data, u32 len);
} GDF_HashMap_T;

static FORCEINLINE u32 __get_idx(
    void* key, 
    GDF_HashMap map
)
{
    u32 idx = map->hash_func((const u8*)key, map->k_stride) % map->capacity;
    return idx;
}

static FORCEINLINE u32 __get_idx_custom(
    void* key, 
    u32 key_size,
    u32 capacity,
    u32 (*hash_func)(const u8* data, u32 len)
)
{
    u32 idx = hash_func((const u8*)key, key_size) % capacity;
    return idx;
}

// Inserts key and value at the given index or linearly probes for an
// empty slot.
// Returns the slot modified.
// TODO! this should return NULL if anything went wrong.
static FORCEINLINE HashmapEntry* __insert(
    u32 start_idx, 
    void* key,
    u32 key_size,
    void* val,
    u32 val_size,
    u32 capacity,
    HashmapEntry* bucket,
    GDF_BOOL* existed
)
{
    u32 idx;

    for (u32 i = 0; i < capacity; i++)
    {
        idx = (start_idx + i) % capacity;

        if (bucket[idx].key != NULL && memcmp(bucket[idx].key, key, key_size) == 0)
        {
            // update entry
            GDF_MemCopy(bucket[idx].val, val, val_size);
            if (existed)
                *existed = GDF_TRUE;
            return bucket + idx;  
        }

        if (bucket[idx].key == NULL)
        {
            bucket[idx].key = GDF_Malloc(key_size, GDF_MEMTAG_APPLICATION);
            GDF_MemCopy(bucket[idx].key, key, key_size);
            bucket[idx].val = GDF_Malloc(val_size, GDF_MEMTAG_APPLICATION);
            GDF_MemCopy(bucket[idx].val, val, val_size);
            if (existed)
                *existed = GDF_FALSE;
            return bucket + idx;  
        }
    }

    // if capacity is exceeded for some reason
    return NULL;
}

static FORCEINLINE void __free_mapentry(HashmapEntry* map_entry)
{
    GDF_Free(map_entry->key);
    map_entry->key = NULL;
    GDF_Free(map_entry->val);
    map_entry->val = NULL;
}

GDF_HashMap GDF_HashmapCreateFull(u32 k_stride, u32 v_stride, u32 (*hash_func)(const u8* data, u32 len), GDF_BOOL string_keys, u32 initial_capacity)
{
    GDF_HashMap map = GDF_Malloc(sizeof(GDF_HashMap_T), GDF_MEMTAG_APPLICATION);
    map->k_stride = k_stride;
    map->v_stride = v_stride;
    map->string_keys = string_keys;
    map->num_entries = 0;
    map->capacity = initial_capacity;

    if (hash_func == NULL)
    {
        hash_func = superfasthash_wrapper;
    }

    map->hash_func = hash_func;

    map->bucket = GDF_Malloc(sizeof(HashmapEntry) * map->capacity, GDF_MEMTAG_APPLICATION);

    return map;
}

GDF_BOOL GDF_HashmapDestroy(GDF_HashMap hashmap)
{
    HashmapEntry* bucket = hashmap->bucket;
    for (u32 i = 0; i < hashmap->capacity; i++) 
    {
        if (bucket[i].key == NULL)
            continue;
            
        __free_mapentry(&bucket[i]);
    }

    GDF_Free(bucket);
    GDF_Free(hashmap);
    
    return GDF_TRUE;
}

void* GDF_HashmapInsert(GDF_HashMap hashmap, void* key, void* value, GDF_BOOL* already_existed)
{
    if (key == NULL)
        return NULL;
    // TODO! when num entries is almost (0.75 or 0.5)x at capacity, alloc new array
    // and rehash all entries.

    HashmapEntry* bucket = hashmap->bucket;

    // Check if map needs to be expanded
    if (hashmap->num_entries + 1 >= (hashmap->capacity / 2))
    {
        u32 new_capacity = hashmap->capacity * 2;
        HashmapEntry* new_bucket = GDF_Malloc(sizeof(HashmapEntry) * new_capacity, GDF_MEMTAG_COLLECTION);

        // Rehash all entries
        for (u32 i = 0; i < hashmap->capacity; i++) 
        {
            if (bucket[i].key == NULL) 
                continue;

            u32 start_idx = __get_idx_custom(
                bucket[i].key, 
                hashmap->k_stride,
                new_capacity,
                hashmap->hash_func
            );
            
            HashmapEntry* entry = __insert(
                start_idx,
                bucket[i].key,
                hashmap->k_stride,
                bucket[i].val,
                hashmap->v_stride,
                new_capacity,
                new_bucket,
                NULL
            );
            __free_mapentry(bucket + i);

            entry->owner = hashmap;
        }

        GDF_Free(hashmap->bucket);
        hashmap->bucket = new_bucket;
        hashmap->capacity = new_capacity;
    }

    // Find next free index to insert in
    u32 start_idx = __get_idx(key, hashmap);
    GDF_BOOL __existed;
    HashmapEntry* entry = __insert(
        start_idx,
        key,
        hashmap->k_stride,
        value,
        hashmap->v_stride,
        hashmap->capacity,
        hashmap->bucket,
        &__existed
    );

    if (entry == NULL)
    {
        return NULL;
    }

    if (!__existed) {
        hashmap->num_entries++;
        entry->owner = hashmap;
    }
    if (already_existed)
        *already_existed = __existed;

    return entry->val;
}

void* GDF_HashmapGet(GDF_HashMap hashmap, void* key)
{
    if (key == NULL)
        return NULL;
    
    HashmapEntry* bucket = hashmap->bucket;
    u32 start_idx = __get_idx(key, hashmap);
    // Linear probing, wrap around until something is found or nothing is found.
    for (u32 i = 0, idx; i < hashmap->capacity && bucket[(
        (idx = (start_idx + i) % hashmap->capacity) 
    )].key != NULL; i++)
    {
        if (memcmp(bucket[idx].key, key, hashmap->k_stride) == 0)
        {
            return bucket[idx].val;
        } 
    }

    return NULL;
}

GDF_BOOL GDF_HashmapRemove(GDF_HashMap hashmap, void* key, void* out_val_p)
{
    if (key == NULL)
    {
        out_val_p = NULL;
        return GDF_FALSE;
    }

    HashmapEntry* bucket = hashmap->bucket;
    u32 start_idx = __get_idx(key, hashmap);

    for (u32 i = 0, idx; i < hashmap->capacity && bucket[(
        (idx = (start_idx + i) % hashmap->capacity) 
    )].key != NULL; i++)
    {
        if (memcmp(bucket[idx].key, key, hashmap->k_stride) == 0)
        {
            if (out_val_p != NULL)
            {
                GDF_MemCopy(out_val_p, bucket[idx].val, hashmap->v_stride);
            }
            __free_mapentry(&bucket[idx]);
            hashmap->num_entries--;
            return GDF_TRUE;
        } 
    }
    
    return GDF_FALSE;
}

u32 GDF_HashmapLen(GDF_HashMap hashmap)
{
    return hashmap->num_entries;
}

HashmapEntry* GDF_HashmapIter(GDF_HashMap hashmap)
{
    for (u32 i = 0; i < hashmap->capacity; i++)
    {
        if (hashmap->bucket[i].key != NULL)
            return hashmap->bucket + i;
    }
    
    return NULL;
}

void GDF_HashmapIterAdvance(HashmapEntry** iter)
{
    GDF_HashMap map = (*iter)->owner;
    u32 idx = ((*iter) - map->bucket) + 1;
    for (; idx < map->capacity; idx++)
    {
        if (map->bucket[idx].key != NULL)
        {
            *iter = map->bucket + idx;
            return;
        }
    }

    *iter = NULL;
    return;
}