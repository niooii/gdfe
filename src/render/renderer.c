#include <render/renderer.h>
#include <render/vk/renderer.h>
#include <core.h>
#include <os/window.h>
#include <profiler.h>

#include <stdio.h>
#include <collections/hashmap.h>

typedef struct GDF_Renderer_T {
    GDF_RendererState state;
} GDF_Renderer_T;

GDF_Renderer GDF_RendererInit(GDF_Window window)
{
    GDF_Renderer renderer = GDF_Malloc(sizeof(GDF_Renderer_T), GDF_MEMTAG_RENDERER);

    renderer->state.frame_number = 0;
    GDF_GetWindowSize(window, &renderer->state.framebuffer_width, &renderer->state.framebuffer_height);

    if (!vk_renderer_init(window, &renderer->state, "GDF")) {
        GDF_Free(renderer);
        return NULL;
    }

    return renderer;
}

void GDF_RendererDestroy(GDF_Renderer renderer)
{
    vk_wait_idle(&renderer->state);
    // for (
    //     HashmapEntry* e = GDF_HashmapIter(renderer->chunk_meshes);
    //     e != NULL;
    //     GDF_HashmapIterAdvance(&e)
    // )
    // {
    //     ChunkMesh* mesh = *(ChunkMesh**)e->val;
    //     chunk_mesh_destroy(&renderer.vk_ctx, mesh);
    // }
    vk_renderer_destroy(&renderer->state);
}

void GDF_RendererResize(GDF_Renderer renderer, u16 width, u16 height)
{
    GDF_RendererState* state = &renderer->state;
    state->framebuffer_width = width;
    state->framebuffer_height = height;
    vk_renderer_resize(state, width, height);
}

void GDF_RendererSetCamera(GDF_Renderer renderer, GDF_Camera* camera)
{
    // TODO! may be the cause of async/parallelism bug later
    renderer->state.camera = camera;
}

bool GDF_RendererDrawFrame(GDF_Renderer renderer, f32 delta_time)
{
    if (vk_renderer_begin_frame(&renderer->state, delta_time)) {

        bool result = vk_renderer_end_frame(&renderer->state, delta_time);

        if (!result) {
            LOG_ERR("Failed to end frame.");
            return false;
        }
        renderer->state.frame_number++;
        return true;
    }

    return false;
}

