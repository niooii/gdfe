#include <gdfe/render/vk_types.h>
#include "../internal/irender/gpu_types.h"
#include <gdfe/collections/list.h>
#include "irender/vk_os.h"
#include "irender/renderer.h"

#include <inttypes.h>
#include <gdfe/render/renderer.h>
#include <vulkan/vk_enum_string_helper.h>

#include "../../include/gdfe/gdfe.h"
#define GDFP_DISABLE

#include "gdfe/profiler.h"
#include "gdfe/render/vk_utils.h"
#include "irender/vk_utils.h"

void GDF_RendererResize(GDF_Renderer renderer, u16 width, u16 height)
{
    renderer->vk_ctx.pending_resize_event = GDF_TRUE;
    renderer->framebuffer_width = width;
    renderer->framebuffer_height = height;
}

GDF_BOOL GDF_RendererDrawFrame(GDF_Renderer renderer, f32 delta_time)
{
    GDF_VkRenderContext* vk_ctx = &renderer->vk_ctx;
    GDF_VkDevice* device = &vk_ctx->device;
    // because there are separate resources for each frame in flight.
    vk_ctx->resource_idx = vk_ctx->current_frame % vk_ctx->max_concurrent_frames;
    PerFrameResources* per_frame = &vk_ctx->per_frame[vk_ctx->resource_idx];

    if (!vk_ctx->ready_for_use && !vk_ctx->pending_resize_event) {
        if (!GDF_VkUtilsIsSuccess(vkDeviceWaitIdle(device->handle))) {
            LOG_ERR("failed vkDeviceWaitIdle (1) failed");
            return GDF_FALSE;
        }
        LOG_ERR("Something happened with renderer");
        return GDF_FALSE;
    }

    // Check if the framebuffer has been resized. If so, a new swapchain must be created.
    // LOG_INFO("pending resize event: %d", vk_ctx->pending_resize_event);
    if (vk_ctx->pending_resize_event)
    {
        if (!GDF_VkUtilsIsSuccess(vkDeviceWaitIdle(device->handle)))
        {
            LOG_ERR("failed vkDeviceWaitIdle (2) failed");
            return GDF_FALSE;
        }

        vk_ctx->ready_for_use = GDF_FALSE;
        // TODO! HANDLE MINIMIZES BETTER...
        if (renderer->framebuffer_height == 0 || renderer->framebuffer_width == 0)
            return GDF_FALSE;

        vk_ctx->recreating_swapchain = GDF_TRUE;

        gdfe_get_surface_capabilities(
            vk_ctx->device.physical_info->handle,
            vk_ctx->surface,
            &vk_ctx->device.physical_info->sc_support_info
        );
        gdfe_swapchain_destroy(vk_ctx);
        if (!gdfe_swapchain_init(vk_ctx, renderer->framebuffer_width, renderer->framebuffer_height))
        {
            LOG_FATAL("failed to recreate swapchain");
            return GDF_FALSE;
        }

        if (!core_renderer_resize(vk_ctx, &renderer->core_renderer))
            return GDF_FALSE;

        if (renderer->callbacks->on_render_resize)
        {
            if (!renderer->callbacks->on_render_resize(
                vk_ctx,
                renderer->app_state,
                &renderer->callbacks->on_render_resize_state
            ))
            {
                LOG_ERR("Render resize callback failed.");
                return GDF_FALSE;
            }
        }

        LOG_INFO("Resized successfully.");
        vk_ctx->pending_resize_event = GDF_FALSE;
        vk_ctx->recreating_swapchain = GDF_FALSE;

        // LOG_INFO("pending resize event: %d", vk_ctx->pending_resize_event);
        // LOG_INFO("recreating swapchain: %d", vk_ctx->recreating_swapchain);

        vk_ctx->ready_for_use = GDF_TRUE;
        return GDF_FALSE;
    }

    // Wait if the previous use of this frame resource set is still in progress on the GPU
    vkWaitForFences(
        device->handle,
        1,
        &per_frame->in_flight_fence,
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

    GDFP_START();

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(cmd_buffer, &begin_info);

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

    if (!core_renderer_draw(renderer, vk_ctx, &renderer->core_renderer))
    {
        // TODO! handle some weird sync stuff here
        LOG_ERR("Core renderer call failed.");
        return GDF_FALSE;
    }

    if (vkEndCommandBuffer(cmd_buffer) != VK_SUCCESS)
        return GDF_FALSE;

    GDFP_LOG_MSG_RESET("Finished recording commands.");

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

    GDFP_LOG_MSG_RESET("Finished graphics queue submission.");

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

    GDFP_LOG_MSG_RESET("Finished present queue submission.");

    vk_ctx->current_frame++;

    if (renderer->callbacks->on_render_end)
    {
        if (!renderer->callbacks->on_render_end(
            vk_ctx,
            renderer->app_state,
            renderer->callbacks->on_render_end_state))
        {
            return GDF_FALSE;
        }
    }

    GDFP_LOG_MSG_RESET("Finished render function.");

    GDFP_END();

    return GDF_TRUE;
}

// Has no effect if the core renderer is disabled.
void GDF_RendererSetActiveCamera(GDF_Renderer renderer, GDF_Camera camera)
{
    renderer->core_renderer.active_camera = camera;
}


GDF_RenderHandle GDF_DebugDrawLine(GDF_Renderer renderer, GDF_Camera camera)
{
    TODO("debug draw line");
}

GDF_RenderHandle GDF_DebugDrawAABB(GDF_Renderer renderer, GDF_Camera camera)
{
    TODO("debug draw aabb");
}

void GDF_RendererSetRenderMode(GDF_Renderer renderer, GDF_RENDER_MODE mode)
{
    renderer->render_mode = mode;
}

void GDF_RendererCycleRenderMode(GDF_Renderer renderer)
{
    renderer->render_mode = ++renderer->render_mode % __GDF_RENDER_MODE_MAX;
}