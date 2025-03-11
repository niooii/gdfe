#include "irender/core_renderer.h"
#include "render/vk_utils.h"

bool __init_vp_ubos(GDF_VkRenderContext* vk_ctx, const GDF_CoreRendererContext* ctx);

bool create_global_buffers(GDF_VkRenderContext* vk_ctx, const GDF_CoreRendererContext* ctx)
{
    // view-projection uniforms
    if (!__init_vp_ubos(vk_ctx, ctx))
    {
        return false;
    }

    return true;
}

bool __init_vp_ubos(GDF_VkRenderContext* vk_ctx, const GDF_CoreRendererContext* ctx)
{
    u32 image_count = vk_ctx->max_concurrent_frames;
    VkDescriptorPoolSize pool_size = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = image_count
    };

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size,
        .maxSets = image_count
    };

    VK_RETURN_FALSE_ASSERT(
        vkCreateDescriptorPool(
            vk_ctx->device.handle,
            &pool_info,
            NULL,
            &ctx->vp_ubo_pool
        )
    );

    VkDescriptorSetLayoutBinding layout_bindings[] = {
    {
            .binding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        }
    };

    VkDescriptorSetLayoutCreateInfo layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = sizeof(layout_bindings) / sizeof(VkDescriptorSetLayoutBinding),
        .pBindings = layout_bindings,
    };

    VK_RETURN_FALSE_ASSERT(
        vkCreateDescriptorSetLayout(
            vk_ctx->device.handle,
            &layout_create_info,
            vk_ctx->device.allocator,
            &ctx->vp_ubo_layout
        )
    );

    const VkDeviceSize buffer_size = sizeof(ViewProjUB);

    for (u32 i = 0; i < image_count; i++)
    {
        CoreRendererPerFrame* per_frame = &ctx->per_frame[i];
        if (!GDF_VkBufferCreateUniform(vk_ctx, buffer_size, &per_frame->vp_ubo))
        {
            LOG_ERR("Failed to create a uniform buffer.");
            return false;
        }

        VkDescriptorSetAllocateInfo descriptor_sets_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = ctx->vp_ubo_pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &ctx->vp_ubo_layout
        };

        vkAllocateDescriptorSets(
            vk_ctx->device.handle,
            &descriptor_sets_alloc_info,
            &per_frame[i].vp_ubo_set
        );

        VkDescriptorBufferInfo buffer_info = {
            .buffer = per_frame->vp_ubo.buffer.handle,
            .offset = 0,
            .range = sizeof(ViewProjUB)
        };

        VkWriteDescriptorSet descriptor_writes[1] = {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = per_frame[i].vp_ubo_set,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &buffer_info
            }
        };

        vkUpdateDescriptorSets(
            vk_ctx->device.handle,
            1,
            descriptor_writes,
            0,
            NULL
        );
    };

    return true;
}