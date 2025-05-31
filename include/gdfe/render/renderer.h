#pragma once

#include <gdfe/render/camera.h>
#include <gdfe/math/math.h>
#include <gdfe/os/video.h>
#include <gdfe/render/geometry.h>
#include "mesh.h"
#include "vk/types.h"

/* The renderer is also a subsystem. */

#define MAX_FRAMES_IN_FLIGHT 3

typedef enum GDF_RENDER_MODE {
    GDF_RENDER_MODE_FULL,
    GDF_RENDER_MODE_WIREFRAME,
    gdfe_render_mode_max
} GDF_RENDER_MODE;

/// Represents a traditional "object" in a general engine.
/// Attach a mesh onto the handle to render a mesh.
typedef struct GDF_Object_T* GDF_Object;

EXTERN_C_BEGIN

void GDF_RendererResize(u16 width, u16 height);

GDF_BOOL GDF_RendererDrawFrame(f32 delta_time);

void GDF_RendererSetActiveCamera(GDF_Camera camera);

void GDF_DebugDrawLine();
void GDF_DebugDrawAABB();

// TODO! create an array of Transform objects or something idk
GDF_Object GDF_ObjCreate();
void GDF_ObjDestroy(GDF_Object handle);
void GDF_ObjSetMesh(GDF_Object handle, GDF_Mesh mesh);
GDF_Object GDF_ObjFromMesh(GDF_Mesh mesh);
/// Returns a pointer to the transform owned by this object.
/// The caller should modify the returned transform, but should never store the pointer.
GDF_Transform* GDF_ObjGetTransform(GDF_Object handle);
/// Temporary, maybe find a better way to structure the API.
/// This just updates the model matrix in the instance data buffer for now.
void GDF_ObjSyncInstanceData(GDF_Object handle);

void GDF_RendererSetRenderMode(GDF_RENDER_MODE mode);
void GDF_RendererCycleRenderMode();

EXTERN_C_END