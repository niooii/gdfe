#include <render/vk/buffers.h>
#include "gpu_types.h"

bool buffers_create(
    VkRenderContext* context,
    u64 alloc_size,
    u32 usage_flags,
    u32 mem_property_flags,
    buffer* out_buf
)
{
    vk_device* device = &context->device;
    VkBufferCreateInfo buf_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = alloc_size,
        .usage = usage_flags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    
    VK_RETURN_FALSE_ASSERT(
        vkCreateBuffer(device->handle, &buf_info, NULL, &out_buf->handle)
    );

    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(device->handle, out_buf->handle, &mem_req);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_req.size,
        .memoryTypeIndex = GDF_VkUtilsFindMemTypeIdx(context, mem_req.memoryTypeBits, mem_property_flags)
    };
    VK_RETURN_FALSE_ASSERT(
        vkAllocateMemory(device->handle, &alloc_info, context->device.allocator, &out_buf->memory)
    );
    VK_RETURN_FALSE_ASSERT(
        vkBindBufferMemory(device->handle, out_buf->handle, out_buf->memory, 0)
    );

    out_buf->size = mem_req.size;

    return true;
}

bool buffers_create_storage(
    VkRenderContext* context,
    void* data,
    u64 data_size,
    buffer* out_buf
) 
{
    // TODO! this can be staging buffered since 
    // i copy once and forget about it
    if (
        !buffers_create(
            context,
            data_size,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            out_buf
        )
    )
    {
        return false;
    }

    VK_RETURN_FALSE_ASSERT(
        vkMapMemory(context->device.handle, out_buf->memory, 0, data_size, 0, &out_buf->mapped_data)
    );
    if (data) {
        GDF_MemCopy(out_buf->mapped_data, data, data_size);
    }

    return true;
}

bool buffers_create_vertex(
    VkRenderContext* context,
    void* vertices,
    u32 vertex_count,
    u32 vertex_size,
    buffer* out_buf
)
{
    if (
        !buffers_create(
            context,
            vertex_size * vertex_count,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            out_buf
        )
    )
    {
        return false;
    }

    VK_RETURN_FALSE_ASSERT(
        vkMapMemory(context->device.handle, out_buf->memory, 0, vertex_count * vertex_size, 0, &out_buf->mapped_data)
    );
    if (vertices) {
        GDF_MemCopy(out_buf->mapped_data, vertices, vertex_count * vertex_size);
    }
    return true;
}

bool buffers_create_index(
    VkRenderContext* context,
    u16* indices,
    u32 index_count,
    buffer* out_buf
)
{
    if (
        !buffers_create(
            context,
            sizeof(u16) * index_count,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            out_buf
        )
    )
    {
        return false;
    }

    VK_RETURN_FALSE_ASSERT(
        vkMapMemory(context->device.handle, out_buf->memory, 0, index_count * sizeof(*indices), 0, &out_buf->mapped_data)
    );
    if (indices) {
        GDF_MemCopy(out_buf->mapped_data, indices, index_count * sizeof(*indices));
    }
    return true;
}

bool buffers_create_uniform(
    VkRenderContext* context,
    u32 size,
    vk_uniform_buffer* out_uniform_buf
)
{
    if (
        !buffers_create(
            context,
            size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &out_uniform_buf->buffer
        )
    )
    {
        return false;
    }

    VK_RETURN_FALSE_ASSERT(
        vkMapMemory(context->device.handle, out_uniform_buf->buffer.memory, 0, size, 0, &out_uniform_buf->mapped_data)
    );
    return true;
}

bool buffers_update(
    VkRenderContext* context,
    buffer* buffer,
    void* data,
    u64 data_size    
)
{
    if (data_size > buffer->size) {
        LOG_FATAL("Tried to transfer data of a larger size into a fixed size buffer");
    }
    GDF_MemCopy(buffer->mapped_data, data, data_size);

    return true;
}

void buffers_destroy(
    VkRenderContext* context,
    buffer* buf
)
{
    vkUnmapMemory(context->device.handle, buf->memory);
    vkFreeMemory(
        context->device.handle,
        buf->memory,
        context->device.allocator
    );
    vkDestroyBuffer(
        context->device.handle,
        buf->handle,
        context->device.allocator
    );
}

void buffers_destroy_uniform(
    VkRenderContext* context,
    vk_uniform_buffer* uniform_buf
)
{
    // TODO! uncomment when implementation changes to staging
    // vkUnmapMemory(
    //     context->device.handle,
    //     uniform_buf->buffer.memory
    // );
    buffers_destroy(context, &uniform_buf->buffer);
}

bool buffers_create_single_use_command(
    VkRenderContext* context,
    VkCommandBuffer* out_command_buf
)
{
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = context->transient_command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VK_RETURN_FALSE_ASSERT(
        vkAllocateCommandBuffers(
            context->device.handle,
            &alloc_info,
            out_command_buf
        )
    );

    return true;
}

void buffers_destroy_single_use_command(
    VkRenderContext* context,
    VkCommandBuffer* command_buf
)
{
    vkFreeCommandBuffers(
        context->device.handle,
        context->transient_command_pool,
        1,
        command_buf
    );
}