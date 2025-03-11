#include <render/vk_types.h>
#include "../internal/irender/gpu_types.h"
#include <collections/list.h>
#include "irender/vk_os.h"
#include "irender/renderer.h"
#include <render/renderer.h>
#include <vulkan/vk_enum_string_helper.h>

#include "gdfe.h"
#include "../../include/render/vk_utils.h"
#include "../../include/render/vk_utils.h"
#include "irender/vk_utils.h"

void GDF_RendererResize(GDF_Renderer renderer, u16 width, u16 height)
{
    renderer->vk_ctx.pending_resize_event = true;
    renderer->framebuffer_width = width;
    renderer->framebuffer_height = height;
}

bool GDF_RendererDrawFrame(GDF_Renderer renderer, f32 delta_time)
{
    GDF_VkRenderContext* vk_ctx = &renderer->vk_ctx;
    GDF_VkDevice* device = &vk_ctx->device;
    // because there are separate resources for each frame in flight.
    vk_ctx->resource_idx = vk_ctx->current_frame % vk_ctx->max_concurrent_frames;
    u32 resource_idx = vk_ctx->resource_idx;
    // for debugging we test
    u32 prev_resource_idx = (MIN(vk_ctx->current_frame - 1, 0)) % vk_ctx->max_concurrent_frames;
    PerFrameResources* per_frame = &vk_ctx->per_frame[resource_idx];

    if (!vk_ctx->ready_for_use && !vk_ctx->pending_resize_event) {
        if (!GDF_VkUtilsIsSuccess(vkDeviceWaitIdle(device->handle))) {
            LOG_ERR("begin frame vkDeviceWaitIdle (1) failed");
            return false;
        }
        LOG_ERR("Something happened with renderer");
        return false;
    }

    // Check if the framebuffer has been resized. If so, a new swapchain must be created.
    if (vk_ctx->pending_resize_event)
    {
        if (!GDF_VkUtilsIsSuccess(vkDeviceWaitIdle(device->handle)))
        {
            LOG_ERR("begin frame vkDeviceWaitIdle (2) failed");
            return false;
        }

        vk_ctx->ready_for_use = false;
        // TODO! HANDLE MINIMIZES BETTER...
        if (renderer->framebuffer_height == 0 || renderer->framebuffer_width == 0)
        {
            return false;
        }

        vk_ctx->recreating_swapchain = true;

        gdfe_get_surface_capabilities(
            vk_ctx->device.physical_info->handle,
            vk_ctx->surface,
            &vk_ctx->device.physical_info->sc_support_info
        );
        gdfe_swapchain_destroy(vk_ctx);
        if (!gdfe_swapchain_init(vk_ctx, renderer->framebuffer_width, renderer->framebuffer_height))
        {
            LOG_FATAL("failed to recreate swapchain");
            return false;
        }

        LOG_INFO("Resized successfully.");
        vk_ctx->pending_resize_event = false;
        vk_ctx->recreating_swapchain = false;

        vk_ctx->ready_for_use = true;
        return false;
    }

    // Wait if the previous use of this frame resource set is still in progress on the GPU
    vkWaitForFences(
        device->handle,
        1,
        &vk_ctx->per_frame[resource_idx].in_flight_fence,
        VK_TRUE,
        UINT64_MAX
    );

    // Acquire the next image from the swapchain
    VK_RETURN_FALSE_ASSERT(
        vkAcquireNextImageKHR(
            vk_ctx->device.handle,
            vk_ctx->swapchain.handle,
            UINT64_MAX,
            per_frame->image_available_semaphore,
            VK_NULL_HANDLE,
            &vk_ctx->swapchain.current_img_idx
        )
    );
    // TODO! handle VK_ERROR_OUT_OF_DATE_KHR and resizing stuff etc etc

    u32 img_idx = vk_ctx->swapchain.current_img_idx;

    // Check if a previous frame is using this image (if there is its fence to wait on)
    if (vk_ctx->images_in_flight[img_idx] != VK_NULL_HANDLE)
    {
        vkWaitForFences(device->handle, 1, &vk_ctx->images_in_flight[img_idx], VK_TRUE, UINT64_MAX);
        // basically mark image as free
        vk_ctx->images_in_flight[img_idx] = VK_NULL_HANDLE;
    }
    vkResetFences(device->handle, 1, &per_frame->in_flight_fence);
    vk_ctx->images_in_flight[img_idx] = per_frame->in_flight_fence;

    VkCommandBuffer cmd_buffer = per_frame->cmd_buffer;
    vkResetCommandBuffer(cmd_buffer, 0);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(cmd_buffer, &begin_info);



    VkClearValue clear_values[2] = {
        {.color = {0, 0, 0, 1}},
        {.depthStencil = {1, 0}}
    };
    // render_pass_info.clearValueCount = 2;
    // render_pass_info.pClearValues = clear_values;

    // vkCmdBeginRenderPass(cmd_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    // make as similar to opengls as possible
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)renderer->framebuffer_height;
    viewport.width = (f32)renderer->framebuffer_width;
    viewport.height = -(f32)renderer->framebuffer_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = vk_ctx->swapchain.extent
    };
    vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

    // TODO! draw UI

    if (renderer->disable_core)
    {
        if (renderer->callbacks->on_render)
        {
            if (!renderer->callbacks->on_render(
                vk_ctx,
                renderer->app_state,
                renderer->callbacks->on_render_state))
            {
                LOG_ERR("Render callback call failed.");
                return false;
            }
        }
    }
    else
    {
        if (!core_renderer_draw(vk_ctx, renderer->callbacks, &renderer->core_renderer))
        {
            // TODO! handle some weird sync stuff here
            LOG_ERR("Core renderer call failed.");
            return false;
        }
    }
    // draw debug grid
    // vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->gdfe_grid_pipeline.handle);
    //
    // vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &context->up_facing_plane_vbo.handle, offsets);
    // vkCmdBindIndexBuffer(cmd_buffer, vk_ctx->up_facing_plane_index_buffer.handle, 0, VK_INDEX_TYPE_UINT16);
    //
    // vkCmdBindDescriptorSets(
    //     cmd_buffer,
    //     VK_PIPELINE_BIND_POINT_GRAPHICS,
    //     vk_ctx->gdfe_grid_pipeline.layout,
    //     0,
    //     1,
    //     &context->global_vp_ubo_sets[resource_idx],
    //     0,
    //     NULL
    // );
    //
    // vkCmdPushConstants(
    //     cmd_buffer,
    //     context->gdfe_grid_pipeline.layout,
    //     VK_SHADER_STAGE_VERTEX_BIT,
    //     0,
    //     sizeof(vec3),
    //     &active_camera->pos
    // );
    //
    // vkCmdDrawIndexed(cmd_buffer, 6, 1, 0, 0, 0);

    // TODO! post processing here

    // vkCmdEndRenderPass(cmd_buffer);

    if (vkEndCommandBuffer(cmd_buffer) != VK_SUCCESS) {

        return false;
    }

    // Submit the command buffer
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &per_frame->image_available_semaphore,
        .pWaitDstStageMask = (VkPipelineStageFlags[]) {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &per_frame->render_finished_semaphore,
    };
    vkQueueSubmit(
        device->graphics_queue,
        1,
        &submit_info,
        per_frame->in_flight_fence
    );

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pImageIndices = &img_idx,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &per_frame->render_finished_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &vk_ctx->swapchain.handle,
        .pResults = NULL,
    };

    vkQueuePresentKHR(
        vk_ctx->device.present_queue,
        &present_info
    );

    vk_ctx->current_frame++;

    if (renderer->callbacks->on_render_end)
    {
        if (!renderer->callbacks->on_render_end(
            vk_ctx,
            renderer->app_state,
            renderer->callbacks->on_render_end_state))
        {
            return false;
        }
    }

    return true;
}

void GDF_RendererSetRenderMode(GDF_Renderer renderer, GDF_RENDER_MODE mode)
{
    renderer->render_mode = mode;
}

void GDF_RendererCycleRenderMode(GDF_Renderer renderer)
{
    renderer->render_mode = ++renderer->render_mode % __GDF_RENDER_MODE_MAX;
}