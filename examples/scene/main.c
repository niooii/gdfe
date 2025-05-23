#include <gdfe/gdfe.h>

#include "gdfe/input.h"

typedef struct App {
    GDF_Camera camera;
} App;

GDF_BOOL on_loop(const GDF_AppState* app_state, f64 dt, void* state);

int main()
{
    App app;

    GDF_InitInfo init_info = {
        .callbacks = {
            .on_loop = on_loop,
            .on_loop_state = &app,
        },
    };

    GDF_AppState* gdfapp = GDF_Init(init_info);

    GDF_CameraCreateInfo camera_info = {
        .pos          = vec3_new(0, 1, 0),
        .fov          = 77.0f,
        .aspect_ratio = 1.777f,
        .far_clip     = 1000,
        .near_clip    = 0.01f,
    };

    GDF_Camera cam = GDF_CameraCreate(&camera_info);
    GDF_RendererSetActiveCamera(cam);

    GDF_Mesh sphere = GDF_MeshGetPrimitive(GDF_PRIMITIVE_MESH_TYPE_SPHERE);

    // This should be rendered at (0, 0, 0)
    GDF_Object sphere_object = GDF_ObjFromMesh(sphere);

    app.camera = cam;

    GDF_Run();

    return 0;
}

GDF_BOOL on_loop(const GDF_AppState* app_state, f64 dt, void* state)
{
    const App* app = (App*)state;

    // Handle input
    const vec3 curr_pos = GDF_CameraGetPosition(app->camera);

    const f32 spd = 8;

    vec3 delta = vec3_zero();

    if (GDF_IsKeyDown(GDF_KEYCODE_A))
        delta.x -= 1;
    if (GDF_IsKeyDown(GDF_KEYCODE_D))
        delta.x += 1;

    if (GDF_IsKeyDown(GDF_KEYCODE_W))
        delta.z += 1;
    if (GDF_IsKeyDown(GDF_KEYCODE_S))
        delta.z -= 1;

    if (GDF_IsKeyDown(GDF_KEYCODE_SPACE))
        delta.y += 1;
    if (GDF_IsKeyDown(GDF_KEYCODE_LSHIFT))
        delta.y -= 1;

    delta = vec3_mul_scalar(vec3_normalized(delta), spd * dt);

    quaternion camera_rot = GDF_CameraGetRotationQuaternion(app->camera);
    delta = quaternion_rotate_vector(camera_rot, delta);

    const vec3 new_pos = vec3_add(curr_pos, delta);

    GDF_CameraSetPosition(app->camera, new_pos);

    ivec2 mouse_delta;
    GDF_GetMouseDelta(&mouse_delta);

    GDF_CameraAddPitch(app->camera, mouse_delta.y * dt * 4);
    GDF_CameraAddYaw(app->camera, mouse_delta.x * dt * 4);

    return GDF_TRUE;
}
