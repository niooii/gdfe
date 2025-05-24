#pragma once

#include <gdfe/prelude.h>
#include "types.h"

EXTERN_C_BEGIN

GDF_BOOL GDF_VkBufferCreate(
    u64 alloc_size,
    u32 usage_flags,
    u32 mem_property_flags,
    GDF_VkBuffer* out_buf
);

// TODO! make completely gpu local and transfer data with staging buffers
GDF_BOOL GDF_VkBufferCreateStorage(
    void* data,
    u64 data_size,
    GDF_VkBuffer* out_buf
);

// Right now support only fixed size creation YUCKY
GDF_BOOL GDF_VkBufferCreateVertex(
    void* vertices,
    u32 vertex_count,
    u32 vertex_size,
    GDF_VkBuffer* out_buf
);

GDF_BOOL GDF_VkBufferCreateIndex(
    u32* indices,
    u32 index_count,
    GDF_VkBuffer* out_buf
);

GDF_BOOL GDF_VkBufferCreateUniform(
    u32 size,
    GDF_VkUniformBuffer* out_uniform_buf
);

GDF_BOOL GDF_VkBufferUpdate(
    GDF_VkBuffer* buffer,
    void* data,
    u64 data_size
);

void GDF_VkBufferDestroy(
    GDF_VkBuffer* buf
);

void GDF_VkBufferDestroyUniform(
    GDF_VkUniformBuffer* uniform_buf
);

GDF_BOOL GDF_VkBufferCreateSingleUseCmd(
    VkCommandBuffer* out_command_buf
);

void GDF_VkBufferDestroySingleUseCmd(
    VkCommandBuffer* command_buf
);

EXTERN_C_END