#include <render/vk_types.h>
#include "gpu_types.h"
#include <collections/list.h>
#include "irender/vk_os.h"
#include "irender/renderer.h"
#include <render/renderer.h>
#include <vulkan/vk_enum_string_helper.h>

#include "gdfe.h"
#include "render/vk/buffers.h"
#include "render/vk/utils.h"

void GDF_RendererResize(GDF_Renderer renderer, u16 width, u16 height)
{
    renderer->vk_ctx.pending_resize_event = true;
}

bool GDF_RendererDrawFrame(GDF_Renderer renderer, f32 delta_time)
{
    VkRenderContext* vk_ctx = &renderer->vk_ctx;
    vk_device* device = &vk_ctx->device;
    // because there are separate resources for each frame in flight.
    vk_ctx->resource_idx = vk_ctx->current_frame % vk_ctx->max_concurrent_frames;
    u32 resource_idx = vk_ctx->resource_idx;

    if (!vk_ctx->ready_for_use) {
        if (!GDF_VkUtilsIsSuccess(vkDeviceWaitIdle(device->handle))) {
            LOG_ERR("begin frame vkDeviceWaitIdle (1) failed");
            return false;
        }
        LOG_DEBUG("Something is happening w the renderer");
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
        if (renderer->framebuffer_height == 0 && renderer->framebuffer_width == 0)
        {
            return false;
        }

        vk_ctx->recreating_swapchain = true;

        // TODO! dont forget about this buddy
        TODO("yea ur not getting away with this one");
        // if (!__recreate_sized_resources(renderer)) {
        //     return false;
        // }

        LOG_INFO("Resized successfully.");
        vk_ctx->pending_resize_event = false;
        vk_ctx->recreating_swapchain = false;
        vk_ctx->ready_for_use = true;
        return false;
    }

    // Wait for the previous frame to finish
    vkWaitForFences(
        device->handle,
        1,
        &vk_ctx->in_flight_fences[resource_idx],
        VK_TRUE,
        UINT64_MAX
    );

    // Acquire the next image from the swapchain
    VK_RETURN_FALSE_ASSERT(
        vkAcquireNextImageKHR(
            vk_ctx->device.handle,
            vk_ctx->swapchain.handle,
            UINT64_MAX,
            vk_ctx->image_available_semaphores[resource_idx],
            VK_NULL_HANDLE,
            &vk_ctx->swapchain.current_img_idx
        )
    );
    // TODO! handle VK_ERROR_OUT_OF_DATE_KHR and resizing stuff etc etc

    // shorthand i guess i aint typing all that
    u32 current_img_idx = vk_ctx->swapchain.current_img_idx;

    // GDF_Camera* active_camera = renderer->camera;
    // if (active_camera)
    // {
    //
    // }
    //
    // // TODO! remove and extract updating ubo into another function
    // ViewProjUB ubo = {
    //     .view_projection = active_camera->view_perspective
    // };
    // memcpy(vk_ctx->uniform_buffers[current_img_idx].mapped_data, &ubo, sizeof(ubo));

    // Check if a previous frame is using this image (if there is its fence to wait on)
    if (vk_ctx->images_in_flight[current_img_idx] != VK_NULL_HANDLE)
    {
        vkWaitForFences(device->handle, 1, &vk_ctx->images_in_flight[current_img_idx], VK_TRUE, UINT64_MAX);
        // basically mark image as free
        vk_ctx->images_in_flight[current_img_idx] = VK_NULL_HANDLE;
    }
    vkResetFences(device->handle, 1, &vk_ctx->in_flight_fences[resource_idx]);
    vk_ctx->images_in_flight[current_img_idx] = vk_ctx->in_flight_fences[resource_idx];

    VkCommandBuffer cmd_buffer = vk_ctx->command_buffers[resource_idx];
    vkResetCommandBuffer(cmd_buffer, 0);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(cmd_buffer, &begin_info);

    // VkRenderPassBeginInfo render_pass_info = {
    //     .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    //     .renderPass = vk_ctx->renderpasses[GDF_VK_RENDERPASS_INDEX_MAIN],
    //     .framebuffer = vk_ctx->swapchain.framebuffers[current_img_idx],
    //     .renderArea.offset = {0, 0},
    //     .renderArea.extent = vk_ctx->swapchain.extent
    // };

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

    if (renderer->callbacks->on_render)
    {
        if (!renderer->callbacks->on_render(
            vk_ctx,
            renderer->app_state,
            renderer->callbacks->on_render_state))
        {
            return false;
        }
    }
    // if (!vk_game_renderer_draw(vk_ctx, state, resource_idx, delta_time))
    // {
    //     LOG_ERR("Failed to render the game.");
    //     // TODO! handle some weird sync stuff here
    //     return false;
    // }

    // draw debug grid
    // vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->grid_pipeline.handle);
    //
    // vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &context->up_facing_plane_vbo.handle, offsets);
    // vkCmdBindIndexBuffer(cmd_buffer, vk_ctx->up_facing_plane_index_buffer.handle, 0, VK_INDEX_TYPE_UINT16);
    //
    // vkCmdBindDescriptorSets(
    //     cmd_buffer,
    //     VK_PIPELINE_BIND_POINT_GRAPHICS,
    //     vk_ctx->grid_pipeline.layout,
    //     0,
    //     1,
    //     &context->global_vp_ubo_sets[resource_idx],
    //     0,
    //     NULL
    // );
    //
    // vkCmdPushConstants(
    //     cmd_buffer,
    //     context->grid_pipeline.layout,
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
        .pWaitSemaphores = &vk_ctx->image_available_semaphores[resource_idx],
        .pWaitDstStageMask = (VkPipelineStageFlags[]) {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &vk_ctx->render_finished_semaphores[resource_idx]
    };
    vkQueueSubmit(
        device->graphics_queue,
        1,
        &submit_info,
        vk_ctx->in_flight_fences[resource_idx]
    );

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pImageIndices = &current_img_idx,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &vk_ctx->render_finished_semaphores[resource_idx],
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