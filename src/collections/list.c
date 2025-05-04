#ifdef GDF_COLLECTIONS

    #include <gdfe/collections/list.h>

void* gdfe_list_create(u64 length, u64 stride)
{
    u64  header_size        = LIST_FIELD_LENGTH * sizeof(u64);
    u64  list_size          = length * stride;
    u64* new_list           = GDF_Malloc(header_size + list_size, GDF_MEMTAG_COLLECTION);
    new_list[LIST_CAPACITY] = length;
    new_list[LIST_LENGTH]   = 0;
    new_list[LIST_STRIDE]   = stride;
    return (void*)(new_list + LIST_FIELD_LENGTH);
}

void gdfe_list_destroy(void* list)
{
    u64* header = (u64*)list - LIST_FIELD_LENGTH;
    GDF_Free(header);
}

u64 gdfe_list_field_get(void* list, u64 field)
{
    u64* header = (u64*)list - LIST_FIELD_LENGTH;
    return header[field];
}

void gdfe_list_field_set(void* list, u64 field, u64 value)
{
    u64* header   = (u64*)list - LIST_FIELD_LENGTH;
    header[field] = value;
}

void* gdfe_list_resize(void* list)
{
    u64 length = GDF_ListLen(list);
    u64 stride = GDF_ListStride(list);
    void* new  = gdfe_list_create((LIST_RESIZE_FACTOR * GDF_ListCapacity(list)), stride);
    GDF_Memcpy(new, list, length * stride);

    GDF_ListSetLen(new, length);
    gdfe_list_destroy(list);
    return new;
}

void* gdfe_list_push(void* list, const void* value_ptr)
{
    u64 length = GDF_ListLen(list);
    u64 stride = GDF_ListStride(list);
    if (length >= GDF_ListCapacity(list))
    {
        list = gdfe_list_resize(list);
    }

    if (value_ptr)
    {
        u64 addr = (u64)list;
        addr += (length * stride);
        GDF_Memcpy((void*)addr, value_ptr, stride);
        GDF_ListSetLen(list, length + 1);
    }
    return list;
}

void* gdfe_list_append(void* list, const void* value_ptr, u32 num_values)
{
    u64 length  = GDF_ListLen(list);
    u64 new_len = length + num_values;
    u64 stride  = GDF_ListStride(list);
    while (new_len >= GDF_ListCapacity(list))
    {
        list = gdfe_list_resize(list);
    }

    if (value_ptr)
    {
        u64 addr = (u64)list;
        addr += (length * stride);
        GDF_Memcpy((void*)addr, value_ptr, stride * num_values);
        GDF_ListSetLen(list, new_len);
    }

    return list;
}

void gdfe_list_pop(void* list, void* dest)
{
    u64 length = GDF_ListLen(list);
    u64 stride = GDF_ListStride(list);
    // because arithmetic on void pointers is
    // quite diabolical
    u64 addr = (u64)list;
    addr += ((length - 1) * stride);
    GDF_Memcpy(dest, (void*)addr, stride);
    GDF_ListSetLen(list, length - 1);
}

void* gdfe_list_remove_at(void* list, u64 index, void* dest)
{
    u64 length = GDF_ListLen(list);
    u64 stride = GDF_ListStride(list);
    if (index >= length)
    {
        LOG_ERR("Invalid index. List len: %i, index: %i", length, index);
        return list;
    }

    u64 addr = (u64)list;
    if (dest != NULL)
        GDF_Memcpy(dest, (void*)(addr + (index * stride)), stride);

    // If not on the last element, snip out the entry and copy the rest inward.
    if (index != length - 1)
    {
        GDF_Memcpy((void*)(addr + (index * stride)), (void*)(addr + ((index + 1) * stride)),
            stride * (length - index));
    }

    GDF_ListSetLen(list, length - 1);
    return list;
}

void* __list_insert_at(void* list, u64 index, void* value_ptr)
{
    u64 length = GDF_ListLen(list);
    u64 stride = GDF_ListStride(list);
    if (index >= length)
    {
        LOG_ERR("Invalid index. List len: %i, index: %index", length, index);
        return list;
    }
    if (length >= GDF_ListCapacity(list))
    {
        list = gdfe_list_resize(list);
    }

    u64 addr = (u64)list;

    // If not on the last element, copy the rest outward.
    if (index != length - 1)
    {
        GDF_Memcpy((void*)(addr + ((index + 1) * stride)), (void*)(addr + (index * stride)),
            stride * (length - index));
    }

    // Set the value at the index
    GDF_Memcpy((void*)(addr + (index * stride)), value_ptr, stride);

    GDF_ListSetLen(list, length + 1);
    return list;
}

#endif
