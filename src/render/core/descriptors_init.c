#include <gdfe/prelude.h>

#include <gdfe/render/vk/utils.h>
#include <i_render/core_renderer.h>
#include "gdfe/render/shader.h"
#include "gdfe/render/vk/types.h"

GDF_BOOL cr_init_global_descriptors(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{
    return GDF_TRUE;
}

void cr_destroy_global_descriptors(GDF_VkRenderContext* vk_ctx, GDF_CoreRendererContext* ctx)
{

}