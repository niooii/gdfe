#pragma once

#include <gdfe/camera.h>

#include <gdfe/math/math.h>
#include <gdfe/os/video.h>
#include "vk/types.h"

#define MAX_FRAMES_IN_FLIGHT 3

typedef enum GDF_RENDER_MODE {
    GDF_RENDER_MODE_FULL,
    GDF_RENDER_MODE_WIREFRAME,
    GDF_RENDER_MODE_MAX
} GDF_RENDER_MODE;

typedef struct GDF_RenderHandle_T* GDF_RenderHandle;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GDF_Renderer_T* GDF_Renderer;

void GDF_RendererResize(GDF_Renderer renderer, u16 width, u16 height);

GDF_BOOL GDF_RendererDrawFrame(GDF_Renderer renderer, f32 delta_time);

void GDF_RendererSetActiveCamera(GDF_Renderer renderer, GDF_Camera camera);

void GDF_DebugDrawLine(GDF_Renderer renderer);
void GDF_DebugDrawAABB(GDF_Renderer renderer);

void GDF_RendererSetRenderMode(GDF_Renderer renderer, GDF_RENDER_MODE mode);
void GDF_RendererCycleRenderMode(GDF_Renderer renderer);

#ifdef __cplusplus
}
#endif
