#pragma once

#include <gdfe/core.h>
#include <gdfe/render/vk_types.h>

#define FOR_EACH_FRAME_IN_FLIGHT(max_frames, code) \
    for(int i = 0; i < max_frames; i++) { \
        code \
    }


// As defined by spec. for quick error checking
FORCEINLINE GDF_BOOL GDF_VkUtilsIsSuccess(VkResult result)
{
    switch (result) {
        // Success Codes
        default:
        case VK_SUCCESS:
        case VK_NOT_READY:
        case VK_TIMEOUT:
        case VK_EVENT_SET:
        case VK_EVENT_RESET:
        case VK_INCOMPLETE:
        case VK_SUBOPTIMAL_KHR:
        case VK_THREAD_IDLE_KHR:
        case VK_THREAD_DONE_KHR:
        case VK_OPERATION_DEFERRED_KHR:
        case VK_OPERATION_NOT_DEFERRED_KHR:
        case VK_PIPELINE_COMPILE_REQUIRED_EXT:
            return GDF_TRUE;

        // Error codes
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        case VK_ERROR_INITIALIZATION_FAILED:
        case VK_ERROR_DEVICE_LOST:
        case VK_ERROR_MEMORY_MAP_FAILED:
        case VK_ERROR_LAYER_NOT_PRESENT:
        case VK_ERROR_EXTENSION_NOT_PRESENT:
        case VK_ERROR_FEATURE_NOT_PRESENT:
        case VK_ERROR_INCOMPATIBLE_DRIVER:
        case VK_ERROR_TOO_MANY_OBJECTS:
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
        case VK_ERROR_FRAGMENTED_POOL:
        case VK_ERROR_SURFACE_LOST_KHR:
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
        case VK_ERROR_INVALID_SHADER_NV:
        case VK_ERROR_OUT_OF_POOL_MEMORY:
        case VK_ERROR_FRAGMENTATION:
        case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        case VK_ERROR_UNKNOWN:
            return GDF_FALSE;
    }
}

#ifdef __cplusplus
extern "C" {
#endif

// Will return VK_NULL_HANDLE if failed.
VkShaderModule GDF_VkUtilsLoadShader(const char* src_rel_path);

i32 GDF_VkUtilsFindMemTypeIdx(u32 type_filter, u32 property_flags);

// The .image field in the VkImageViewCreateInfo struct does not have to point
// to a valid image. It will be modified to do so within this function.
GDF_BOOL GDF_VkImageCreate(VkImageCreateInfo* image_info, VkImageViewCreateInfo* view_info, GDF_VkImage* out_image);

void GDF_VkImageDestroy(GDF_VkRenderContext* context, GDF_VkImage* image);

// For applications with custom rendering, any graphics pipeline created
// for rendering geometry during the deferred pass
// must be passed to this function before being used.
// Only the fields renderPass, subpass are modified
void GDF_VkPipelineInfoFillGeometryPass(VkGraphicsPipelineCreateInfo* info);

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
    u16* indices,
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

#ifdef __cplusplus
}
#endif