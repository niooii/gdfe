#pragma once

#include <camera.h>

#include <math/math_types.h>
#include <os/window.h>
#include "vk_types.h"

#define MAX_FRAMES_IN_FLIGHT 3

typedef enum GDF_RENDER_MODE {
    GDF_RENDER_MODE_FULL,
    GDF_RENDER_MODE_WIREFRAME,
} GDF_RENDER_MODE;

struct GDF_Renderer_T;
typedef struct GDF_Renderer_T* GDF_Renderer;

GDF_Renderer GDF_RendererInit(GDF_Window window);
void GDF_RendererDestroy(GDF_Renderer renderer);

void GDF_RendererResize(GDF_Renderer renderer, u16 width, u16 height);

bool GDF_RendererDrawFrame(GDF_Renderer renderer, f32 delta_time);

void GDF_RendererSetCamera(GDF_Renderer renderer, GDF_Camera* camera);

// bool renderer_register_chunk(Chunk* chunk);
// // queue a chunk's remeshing if needed. can be repeatedly called for the same chunk, it will still only be meshed once per frame
// bool renderer_queue_chunk_remesh(ivec3 at);
// bool renderer_remove_chunk(ivec3 at);
