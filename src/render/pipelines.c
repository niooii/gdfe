#include <gdfe/render/vk/pipelines.h>

#include "i_render/renderer.h"

GDF_BOOL GDF_VkCreatePipelineBase(GDF_VkPipelineBase* out, GDF_VkPipelineCreateInfo create_info)
{
    GDF_Memzero(out, sizeof(GDF_VkPipelineBase));

    GDF_Memcpy(&out->create_params, &create_info, sizeof(GDF_VkPipelineCreateInfo));

    switch (out->create_params.type)
    {
    case GDF_PIPELINE_TYPE_GRAPHICS:
        {
            out->layout = create_info.create_info.graphics.layout;

            const VkPipelineShaderStageCreateInfo* shader_stages =
                create_info.create_info.graphics.pStages;
            uint32_t stage_count = create_info.create_info.graphics.stageCount;

            // find and store vertex and fragment shader modules
            for (uint32_t i = 0; i < stage_count; i++)
            {
                if (shader_stages[i].stage == VK_SHADER_STAGE_VERTEX_BIT)
                    out->vert = shader_stages[i].module;
                else if (shader_stages[i].stage == VK_SHADER_STAGE_FRAGMENT_BIT)
                    out->frag = shader_stages[i].module;
            }

            if (stage_count > 0 && shader_stages)
            {
                VkPipelineShaderStageCreateInfo* stages_copy = GDF_Malloc(
                    sizeof(VkPipelineShaderStageCreateInfo) * stage_count, GDF_MEMTAG_UNKNOWN);

                GDF_Memcpy(stages_copy, shader_stages,
                    sizeof(VkPipelineShaderStageCreateInfo) * stage_count);

                out->create_params.create_info.graphics.pStages = stages_copy;
            }

            VkResult result = vkCreateGraphicsPipelines(GDFE_INTERNAL_VK_CTX->device.handle,
                VK_NULL_HANDLE, 1, &create_info.create_info.graphics,
                GDFE_INTERNAL_VK_CTX->device.allocator, &out->handle);

            VK_RETURN_FALSE_ASSERT(result);
        break;
        }
    case GDF_PIPELINE_TYPE_COMPUTE:
        {
            out->layout = create_info.create_info.compute.layout;

            if (create_info.create_info.compute.stage.stage == VK_SHADER_STAGE_COMPUTE_BIT)
            {
                out->comp = create_info.create_info.compute.stage.module;
            }

            VkResult result = vkCreateComputePipelines(GDFE_INTERNAL_VK_CTX->device.handle,
                VK_NULL_HANDLE, 1, &create_info.create_info.compute,
                GDFE_INTERNAL_VK_CTX->device.allocator, &out->handle);

            VK_RETURN_FALSE_ASSERT(result);
        break;
        }
    default:
        return GDF_FALSE;
    }

    return GDF_TRUE;
}

GDF_BOOL GDF_VkDestroyPipelineBase(GDF_VkPipelineBase* base)
{
    switch (base->create_params.type)
    {
        case GDF_PIPELINE_TYPE_GRAPHICS:
        {
            GDF_Free(base->create_params.create_info.graphics.pStages);
        }
        case GDF_PIPELINE_TYPE_COMPUTE: {}
    default: return GDF_FALSE;
    }

    if (base->handle != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(GDFE_INTERNAL_VK_CTX->device.handle, base->handle,
            GDFE_INTERNAL_VK_CTX->device.allocator);
        base->handle = VK_NULL_HANDLE;
    }

    base->vert   = VK_NULL_HANDLE;
    base->frag   = VK_NULL_HANDLE;
    base->layout = VK_NULL_HANDLE;

    GDF_Memzero(&base->create_params, sizeof(GDF_VkPipelineCreateInfo));

    return GDF_TRUE;
}
