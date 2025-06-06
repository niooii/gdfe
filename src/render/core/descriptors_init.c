#include <gdfe/prelude.h>

#include <gdfe/render/vk/utils.h>
#include <i_render/core_renderer.h>
#include "gdfe/render/shader.h"
#include "gdfe/render/vk/types.h"

#define MAX_GLOBAL_DESCRIPTOR_BUFFER_COUNT 512

GDF_BOOL cr_init_global_descriptors(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    VkDescriptorPoolCreateInfo poolInfo = {
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &(VkDescriptorPoolSize){
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = MAX_GLOBAL_DESCRIPTOR_BUFFER_COUNT,
        }
    };
    return GDF_TRUE;
}

void cr_destroy_global_descriptors(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{

}