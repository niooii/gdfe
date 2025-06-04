#include "i_render/renderer.h"
#include <gdfe/collections/list.h>
#include <gdfe/render/vk/types.h>
#include "i_render/vk_os.h"

#include <gdfe/render/renderer.h>
#include <inttypes.h>
#include <gdfe/render/vk/enum_string_helper.h>

#define GDFP_DISABLE

#include <i_render/mesh.h>


#include "gdfe/profiler.h"
#include "gdfe/render/vk/utils.h"
#include "i_render/vk_utils.h"

void GDF_RendererResize(u16 width, u16 height)
{
    GDFE_RENDER_STATE.vk_ctx.pending_resize_event = GDF_TRUE;
    GDFE_RENDER_STATE.framebuffer_width           = width;
    GDFE_RENDER_STATE.framebuffer_height          = height;
}

GDF_BOOL GDF_RendererDrawFrame(f32 delta_time)
{
    GDF_VkRenderContext* vk_ctx = &GDFE_RENDER_STATE.vk_ctx;
    GDF_VkDevice*        device = &vk_ctx->device;
    // because there are separate resources for each frame in flight.
    vk_ctx->resource_idx         = vk_ctx->current_frame % vk_ctx->fof;
    VkFrameResources* per_frame = &vk_ctx->per_frame[vk_ctx->resource_idx];

    // TODO! have some beter way of handling this perhaps
    if (!GDFE_RENDER_STATE.core_ctx.active_camera)
        return GDF_TRUE;

    if (!vk_ctx->ready_for_use && !vk_ctx->pending_resize_event)
    {
        if (!GDF_VkUtilsIsSuccess(vkDeviceWaitIdle(device->handle)))
        {
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
        if (GDFE_RENDER_STATE.framebuffer_height == 0 || GDFE_RENDER_STATE.framebuffer_width == 0)
            return GDF_FALSE;

        vk_ctx->recreating_swapchain = GDF_TRUE;

        gdfe_get_surface_capabilities(vk_ctx->device.physical_info->handle, vk_ctx->surface,
            &vk_ctx->device.physical_info->sc_support_info);
        gdfe_swapchain_destroy(vk_ctx);
        if (!gdfe_swapchain_init(vk_ctx, GDFE_RENDER_STATE.framebuffer_width, GDFE_RENDER_STATE.framebuffer_height))
        {
            LOG_FATAL("failed to recreate swapchain");
            return GDF_FALSE;
        }

        if (!core_renderer_resize(vk_ctx, &GDFE_RENDER_STATE.core_ctx))
            return GDF_FALSE;

        if (GDFE_RENDER_STATE.callbacks->on_render_resize)
        {
            if (!GDFE_RENDER_STATE.callbacks->on_render_resize(
                    vk_ctx, GDFE_RENDER_STATE.app_state, &GDFE_RENDER_STATE.callbacks->on_render_resize_state))
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
    vkWaitForFences(device->handle, 1, &per_frame->in_flight_fence, VK_TRUE, UINT64_MAX);

    // Acquire the next image from the swapchain
    VK_RETURN_FALSE_ASSERT(vkAcquireNextImageKHR(vk_ctx->device.handle, vk_ctx->swapchain.handle,
        UINT64_MAX, per_frame->image_available_semaphore, VK_NULL_HANDLE,
        &vk_ctx->swapchain.current_img_idx));
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
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(cmd_buffer, &begin_info);

    // make as similar to opengls as possible
    VkViewport viewport;
    viewport.x        = 0.0f;
    viewport.y        = (f32)GDFE_RENDER_STATE.framebuffer_height;
    viewport.width    = (f32)GDFE_RENDER_STATE.framebuffer_width;
    viewport.height   = -(f32)GDFE_RENDER_STATE.framebuffer_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = { 0, 0 },
        .extent = vk_ctx->swapchain.extent,
    };
    vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

    if (!core_renderer_draw(vk_ctx, &GDFE_RENDER_STATE.core_ctx))
    {
        // TODO! handle some weird sync stuff here
        LOG_ERR("Core renderer call failed.");
        return GDF_FALSE;
    }

    if (vkEndCommandBuffer(cmd_buffer) != VK_SUCCESS)
        return GDF_FALSE;

    // Submit the command buffer
    VkSubmitInfo submit_info = {
        .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &per_frame->image_available_semaphore,
        .pWaitDstStageMask =
            (VkPipelineStageFlags[]){ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT },
        .commandBufferCount   = 1,
        .pCommandBuffers      = &cmd_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &per_frame->render_finished_semaphore,
    };
    vkQueueSubmit(device->graphics_queue, 1, &submit_info, per_frame->in_flight_fence);

    VkPresentInfoKHR present_info = {
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pImageIndices      = &img_idx,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &per_frame->render_finished_semaphore,
        .swapchainCount     = 1,
        .pSwapchains        = &vk_ctx->swapchain.handle,
        .pResults           = NULL,
    };

    vkQueuePresentKHR(vk_ctx->device.present_queue, &present_info);

    vk_ctx->current_frame++;

    if (GDFE_RENDER_STATE.callbacks->on_render_end)
    {
        if (!GDFE_RENDER_STATE.callbacks->on_render_end(
                vk_ctx, GDFE_RENDER_STATE.app_state, GDFE_RENDER_STATE.callbacks->on_render_end_state))
        {
            return GDF_FALSE;
        }
    }

    return GDF_TRUE;
}

// Has no effect if the core renderer is disabled.
void GDF_RendererSetActiveCamera(GDF_Camera camera)
{
    GDFE_RENDER_STATE.core_ctx.active_camera = camera;
}

/* The object API */

GDF_Object GDF_ObjCreate()
{
    GDF_Object_T* handle = GDF_Malloc(sizeof(GDF_Object_T), GDF_MEMTAG_APPLICATION);

    handle->mesh = GDF_NULL_HANDLE;
    handle->transform = GDF_TransformDefault();

    return handle;
}

void GDF_ObjSetMesh(GDF_Object handle, GDF_Mesh mesh)
{
    if (handle->mesh != GDF_NULL_HANDLE)
    {
        // do some stuff and remove instance data from that mesh
    }

    u64 instance_id = GDF_ListLen(mesh->instance_data);
    handle->instance_index = instance_id;

    GDF_ObjInstanceData instance_data = {
        .model = GDF_TransformModelMatrix(&handle->transform)
    };

    GDF_ListPushPtr(mesh->instance_data, &instance_data);
    handle->mesh = mesh;
}

GDF_Object GDF_ObjFromMesh(GDF_Mesh mesh)
{
    GDF_Object handle = GDF_ObjCreate();

    GDF_ObjSetMesh(handle, mesh);

    return handle;
}

GDF_Transform* GDF_ObjGetTransform(GDF_Object handle)
{
    return &handle->transform;
}

void GDF_ObjSyncInstanceData(GDF_Object handle)
{
    handle->mesh->instance_data[handle->instance_index].model =
        GDF_TransformModelMatrix(&handle->transform);
}

void GDF_RendererSetRenderMode(GDF_RENDER_MODE mode)
{
    GDFE_RENDER_STATE.render_mode = mode;
}

void GDF_RendererCycleRenderMode()
{
    GDFE_RENDER_STATE.render_mode = ++GDFE_RENDER_STATE.render_mode % gdfe_render_mode_max;
}
