#pragma once

#include <render/renderer.h>
#include <os/window.h>
#include <math/math.h>

#include "buffers.h"
#include "utils.h"

// wait for the gpu to be in an idle state, if anything needs it
// should be used for once in a while tasks
FORCEINLINE void vk_wait_idle(GDF_RendererState* state)
{   
    vkDeviceWaitIdle(state->vk_ctx.device.handle);
}

bool vk_renderer_init(GDF_Window window, GDF_RendererState* renderer, const char* application_name);
void vk_renderer_destroy(GDF_RendererState* renderer);

void vk_renderer_resize(GDF_RendererState* renderer, u16 width, u16 height);

bool vk_renderer_begin_frame(GDF_RendererState* renderer, f32 delta_time);
bool vk_renderer_end_frame(GDF_RendererState* renderer, f32 delta_time);
