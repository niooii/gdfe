#include <gdfe/render/vk/buffers.h>

#include <gdfe/render/vk/utils.h>
#include <i_render/renderer.h>

GDF_BOOL GDF_VkBufferCreate(
    u64 alloc_size, u32 usage_flags, u32 mem_property_flags, GDF_VkBuffer* out_buf)
{
    GDF_VkDevice*      device   = &GDFE_VK_CTX->device;
    VkBufferCreateInfo buf_info = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = alloc_size,
        .usage       = usage_flags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VK_RETURN_FALSE_ASSERT(vkCreateBuffer(device->handle, &buf_info, NULL, &out_buf->handle));

    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(device->handle, out_buf->handle, &mem_req);

    VkMemoryAllocateInfo alloc_info = {
        .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize  = mem_req.size,
        .memoryTypeIndex = GDF_VkUtilsFindMemTypeIdx(mem_req.memoryTypeBits, mem_property_flags),
    };
    VK_RETURN_FALSE_ASSERT(vkAllocateMemory(
        device->handle, &alloc_info, GDFE_VK_CTX->device.allocator, &out_buf->memory));
    VK_RETURN_FALSE_ASSERT(vkBindBufferMemory(device->handle, out_buf->handle, out_buf->memory, 0));

    out_buf->size = mem_req.size;

    return GDF_TRUE;
}

GDF_BOOL GDF_VkBufferCreateStorage(void* data, u64 data_size, GDF_VkBuffer* out_buf)
{
    // TODO! this can be staging buffered since
    // i copy once and forget about it
    if (!GDF_VkBufferCreate(data_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, out_buf))
    {
        return GDF_FALSE;
    }

    VK_RETURN_FALSE_ASSERT(vkMapMemory(GDFE_VK_CTX->device.handle, out_buf->memory, 0,
        data_size, 0, &out_buf->mapped_data));
    if (data)
    {
        GDF_Memcpy(out_buf->mapped_data, data, data_size);
    }

    return GDF_TRUE;
}

GDF_BOOL GDF_VkBufferCreateVertex(
    void* vertices, u32 vertex_count, u32 vertex_size, GDF_VkBuffer* out_buf)
{
    if (!GDF_VkBufferCreate(vertex_size * vertex_count, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, out_buf))
    {
        return GDF_FALSE;
    }

    VK_RETURN_FALSE_ASSERT(vkMapMemory(GDFE_VK_CTX->device.handle, out_buf->memory, 0,
        vertex_count * vertex_size, 0, &out_buf->mapped_data));
    if (vertices)
    {
        GDF_Memcpy(out_buf->mapped_data, vertices, vertex_count * vertex_size);
    }
    return GDF_TRUE;
}

GDF_BOOL GDF_VkBufferCreateIndex(u32* indices, u32 index_count, GDF_VkBuffer* out_buf)
{
    if (!GDF_VkBufferCreate(sizeof(*indices) * index_count, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, out_buf))
        return GDF_FALSE;

    VK_RETURN_FALSE_ASSERT(vkMapMemory(GDFE_VK_CTX->device.handle, out_buf->memory, 0,
        index_count * sizeof(*indices), 0, &out_buf->mapped_data));
    if (indices)
        GDF_Memcpy(out_buf->mapped_data, indices, index_count * sizeof(*indices));
    return GDF_TRUE;
}

GDF_BOOL GDF_VkBufferCreateUniform(u32 size, GDF_VkUniformBuffer* out_uniform_buf)
{
    if (!GDF_VkBufferCreate(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &out_uniform_buf->buffer))
        return GDF_FALSE;

    VK_RETURN_FALSE_ASSERT(vkMapMemory(GDFE_VK_CTX->device.handle,
        out_uniform_buf->buffer.memory, 0, size, 0, &out_uniform_buf->mapped_data));
    return GDF_TRUE;
}

GDF_BOOL GDF_VkBufferUpdate(GDF_VkBuffer* buffer, void* data, u64 data_size)
{
    if (data_size > buffer->size)
        LOG_FATAL("Tried to transfer data of a larger size into a fixed size buffer");
    GDF_Memcpy(buffer->mapped_data, data, data_size);

    return GDF_TRUE;
}

void GDF_VkBufferDestroy(GDF_VkBuffer* buf)
{
    vkUnmapMemory(GDFE_VK_CTX->device.handle, buf->memory);
    vkFreeMemory(
        GDFE_VK_CTX->device.handle, buf->memory, GDFE_VK_CTX->device.allocator);
    vkDestroyBuffer(
        GDFE_VK_CTX->device.handle, buf->handle, GDFE_VK_CTX->device.allocator);
}

void GDF_VkBufferDestroyUniform(GDF_VkUniformBuffer* uniform_buf)
{
    // TODO! uncomment when implementation changes to staging
    // vkUnmapMemory(
    //     GDFE_INTERNAL_VK_CTX->device.handle,
    //     uniform_buf->buffer.memory
    // );
    GDF_VkBufferDestroy(&uniform_buf->buffer);
}

GDF_BOOL GDF_VkBufferCreateSingleUseCmd(VkCommandBuffer* out_command_buf)
{
    VkCommandBufferAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = GDFE_VK_CTX->transient_command_pool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VK_RETURN_FALSE_ASSERT(vkAllocateCommandBuffers(
        GDFE_VK_CTX->device.handle, &alloc_info, out_command_buf));

    return GDF_TRUE;
}

void GDF_VkBufferDestroySingleUseCmd(VkCommandBuffer* command_buf)
{
    vkFreeCommandBuffers(GDFE_VK_CTX->device.handle,
        GDFE_VK_CTX->transient_command_pool, 1, command_buf);
}
