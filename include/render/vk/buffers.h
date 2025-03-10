#pragma once

#include <core.h>
#include <render/vk_types.h>

bool buffers_create(
    VkRenderContext* context,
    u64 alloc_size,
    u32 usage_flags,
    u32 mem_property_flags,
    buffer* out_buf
);

// TODO! make completely gpu local and transfer data with staging buffers
bool buffers_create_storage(
    VkRenderContext* context,
    void* data,
    u64 data_size,
    buffer* out_buf
);

// Right now support only fixed size creation YUCKY
bool buffers_create_vertex(
    VkRenderContext* context,
    void* vertices,
    u32 vertex_count,
    u32 vertex_size,
    buffer* out_buf
);

bool buffers_create_index(
    VkRenderContext* context,
    u16* indices,
    u32 index_count,
    buffer* out_buf
);

bool buffers_create_uniform(
    VkRenderContext* context,
    u32 size,
    vk_uniform_buffer* out_uniform_buf
);

bool buffers_update(
    VkRenderContext* context,
    buffer* buffer,
    void* data,
    u64 data_size    
);

void buffers_destroy(
    VkRenderContext* context,
    buffer* buf
);

void buffers_destroy_uniform(
    VkRenderContext* context,
    vk_uniform_buffer* uniform_buf
);

bool buffers_create_single_use_command(
    VkRenderContext* context,
    VkCommandBuffer* out_command_buf
);

void buffers_destroy_single_use_command(
    VkRenderContext* context,
    VkCommandBuffer* command_buf
);