#include <collections/set.h>
#include <superfasthash.h>
// TODO! move away from memcmp for comparison, use custom comparison function

typedef struct GDF_Set_T {
    u32 stride;
    // TODO! string keys should have a different hash function i think
    bool string_keys;
    u32 num_entries;
    u32 capacity;
    void** bucket;
    u32 (*hash_func)(const u8* data, u32 len);
} GDF_Set_T;

static FORCEINLINE u32 __get_idx(
    void* val, 
    GDF_Set set
)
{
    u32 idx = set->hash_func((const u8*)val, set->stride) % set->capacity;
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

// Inserts value at the given index or linearly probes for an
// empty slot.
// Returns the 'slot' inserted at, or if it was already there.
// should return NULL on error.
static FORCEINLINE void** __insert(
    u32 start_idx, 
    void* val,
    u32 stride,
    u32 capacity,
    void** bucket,
    bool* existed
)
{
    u32 idx;

    for (u32 i = 0; i < capacity; i++)
    {
        idx = (start_idx + i) % capacity;

        if (bucket[idx] != NULL && memcmp(bucket[idx], val, stride) == 0)
        {
            // already exists
            if (existed)
                *existed = true;
            return bucket + idx; 
        }

        if (bucket[idx] == NULL)
        {
            bucket[idx] = GDF_Malloc(stride, GDF_MEMTAG_APPLICATION);
            GDF_MemCopy(bucket[idx], val, stride);
            if (existed)
                *existed = false;
            return bucket + idx;  
        }
    }

    // if capacity is exceeded for some reason
    return NULL;
}

static FORCEINLINE void __free_setentry(void** set_entry)
{
    GDF_Free(*set_entry);
    *set_entry = NULL;
}

GDF_Set __set_create(u32 stride, u32 (*hash_func)(const u8* data, u32 len), bool string_keys, u32 initial_capacity)
{
    GDF_Set set = GDF_Malloc(sizeof(GDF_Set_T), GDF_MEMTAG_APPLICATION);
    set->stride = stride;
    set->string_keys = string_keys;
    set->num_entries = 0;
    set->capacity = initial_capacity;

    if (hash_func == NULL)
    {
        hash_func = _superfasthash_wrapper;
    }

    set->hash_func = hash_func;

    set->bucket = GDF_Malloc(sizeof(void*) * set->capacity, GDF_MEMTAG_APPLICATION);

    return set;
}

bool GDF_SetDestroy(GDF_Set set)
{
    void** bucket = set->bucket;
    for (u32 i = 0; i < set->capacity; i++) 
    {
        if (bucket[i] == NULL)
            continue;
            
        __free_setentry(&bucket[i]);
    }

    GDF_Free(bucket);
    GDF_Free(set);
    
    return true;    
}

void* GDF_SetInsert(GDF_Set set, void* value, bool* already_existed)
{
    if (value == NULL)
        return NULL;
    // TODO! when num entries is almost (0.75 or 0.5)x at capacity, alloc new array
    // and rehash all entries.

    void** bucket = set->bucket;

    // Check if set needs to be expanded
    if (set->num_entries + 1 >= (set->capacity / 2))
    {
        u32 new_capacity = set->capacity * 2;
        void** new_bucket = GDF_Malloc(sizeof(void*) * new_capacity, GDF_MEMTAG_COLLECTION);

        // Rehash all entries
        for (u32 i = 0; i < set->capacity; i++) 
        {
            if (bucket[i] == NULL) 
                continue;

            u32 start_idx = __get_idx_custom(
                bucket[i], 
                set->stride,
                new_capacity,
                set->hash_func
            );
            
            void** entry = __insert(
                start_idx,
                bucket[i],
                set->stride,
                new_capacity,
                new_bucket,
                NULL
            );
            __free_setentry(bucket + i);
        }

        GDF_Free(set->bucket);
        set->bucket = new_bucket;
        set->capacity = new_capacity;
    }

    // Find next free index to insert in
    u32 start_idx = __get_idx(value, set);
    bool __existed;
    void** entry = __insert(
        start_idx,
        value,
        set->stride,
        set->capacity,
        set->bucket,
        &__existed
    );

    if (entry == NULL)
    {
        return NULL;
    }

    if (!__existed)
        set->num_entries++;
    if (already_existed)
        *already_existed = __existed;

    return *entry;
}

bool GDF_SetRemove(GDF_Set set, void* value, void* out_val_p)
{
    if (value == NULL)
    {
        out_val_p = NULL;
        return false;
    }

    void** bucket = set->bucket;
    u32 start_idx = __get_idx(value, set);

    for (u32 i = 0, idx; i < set->capacity && bucket[(
        (idx = (start_idx + i) % set->capacity) 
    )] != NULL; i++)
    {
        if (memcmp(bucket[idx], value, set->stride) == 0)
        {
            if (out_val_p != NULL)
            {
                GDF_MemCopy(out_val_p, bucket[idx], set->stride);
            }
            __free_setentry(&bucket[idx]);
            set->num_entries--;
            return true;
        } 
    }
    
    return false;
}

u32 GDF_SetLen(GDF_Set set)
{
    return set->num_entries;
}

SetIterator GDF_SetIter(GDF_Set set)
{
    SetIterator iter = {
        .__owner = set
    };
    for (u32 i = 0; i < set->capacity; i++)
    {
        if (set->bucket[i] != NULL) {
            iter.curr = set->bucket[i];
            iter.__idx = i;
            return iter;
        }
    }
    
    iter.curr = NULL;

    return iter;
}

void GDF_SetIterAdvance(SetIterator* iter)
{
    GDF_Set set = iter->__owner;
    u32 idx = iter->__idx + 1;
    for (; idx < set->capacity; idx++)
    {
        if (set->bucket[idx] != NULL)
        {
            iter->curr = set->bucket[idx];
            iter->__idx = idx;
            return;
        }
    }

    iter->curr = NULL;
    return;
}

void GDF_SetIterConsume(SetIterator* iter, void* prev_val_p)
{
    GDF_Set set = iter->__owner;
    u32 idx = iter->__idx + 1;
    for (; idx < set->capacity; idx++)
    {
        if (set->bucket[idx] != NULL)
        {
            if (prev_val_p)
                GDF_MemCopy(prev_val_p, iter->curr, set->stride);
            __free_setentry(set->bucket + iter->__idx);
            set->num_entries--;
            iter->curr = set->bucket[idx];
            iter->__idx = idx;
            return;
        }
    }

    if (prev_val_p)
        GDF_MemCopy(prev_val_p, iter->curr, set->stride);

    __free_setentry(set->bucket + iter->__idx);
    set->num_entries--;
        
    iter->curr = NULL;
    return;
}