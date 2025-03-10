#pragma once

#include <camera.h>

#include <math/math.h>
#include <os/window.h>
#include "vk_types.h"

#define MAX_FRAMES_IN_FLIGHT 3

typedef enum GDF_RENDER_MODE {
    GDF_RENDER_MODE_FULL,
    GDF_RENDER_MODE_WIREFRAME,
    __GDF_RENDER_MODE_MAX
} GDF_RENDER_MODE;

typedef struct GDF_Renderer_T* GDF_Renderer;

void GDF_RendererResize(GDF_Renderer renderer, u16 width, u16 height);

bool GDF_RendererDrawFrame(GDF_Renderer renderer, f32 delta_time);

void GDF_RendererSetCamera(GDF_Renderer renderer, GDF_Camera* camera);

void GDF_RendererSetRenderMode(GDF_Renderer renderer, GDF_RENDER_MODE mode);
void GDF_RendererCycleRenderMode(GDF_Renderer renderer);

// bool renderer_register_chunk(Chunk* chunk);
// // queue a chunk's remeshing if needed. can be repeatedly called for the same chunk, it will still only be meshed once per frame
// bool renderer_queue_chunk_remesh(ivec3 at);
// bool renderer_remove_chunk(ivec3 at);
