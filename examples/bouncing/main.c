#include <gdfe/gdfe.h>

#include "gdfe/input.h"

/* A simple scene containing quite a few bouncing cubes. */

typedef struct BouncingCube {
    GDF_Object object;
    vec3       velocity;
} BouncingCube;

typedef struct App {
    GDF_Camera camera;

    GDF_LIST(BouncingCube) cubes;
} App;

#define NUM_CUBES 1000
#define XZ_BOUNDS 150

GDF_BOOL on_loop(const GDF_AppState* app_state, f64 dt, void* state);

int main()
{
    App app;

    GDF_InitInfo init_info = {
        .callbacks = {
            .on_loop = on_loop,
            .on_loop_state = &app,
        },
        .config = {
            .updates_per_sec = 144,
        }
    };

    GDF_AppState* gdf_app = GDF_Init(init_info);

    GDF_CameraCreateInfo camera_info = {
        .pos          = vec3_new(0, 1, 0),
        .fov          = 70.0f,
        .aspect_ratio = 1.777f,
        .far_clip     = 1000,
        .near_clip    = 0.01f,
    };

    GDF_Camera cam = GDF_CameraCreate(&camera_info);
    GDF_RendererSetActiveCamera(cam);

    GDF_Mesh cube_mesh = GDF_MeshGetPrimitive(GDF_PRIMITIVE_MESH_TYPE_CUBE);
    GDF_Mesh sphere_mesh = GDF_MeshGetPrimitive(GDF_PRIMITIVE_MESH_TYPE_SPHERE);

    GDF_Object sphere = GDF_ObjFromMesh(sphere_mesh);
    GDF_Object cube = GDF_ObjFromMesh(cube_mesh);

    app.cubes = GDF_ListReserve(BouncingCube, NUM_CUBES);

    for (int i = 0; i < NUM_CUBES; i++)
    {
        GDF_Mesh mesh = i % 2 == 0 ? cube_mesh : sphere_mesh;
        app.cubes[i].object   = GDF_ObjFromMesh(mesh);
        app.cubes[i].velocity = vec3_new(0.0f, 0.0f, 0.0f);

        GDF_Transform* transform = GDF_ObjGetTransform(app.cubes[i].object);
        transform->pos = vec3_new(GDF_FRandomRange(-XZ_BOUNDS, XZ_BOUNDS), GDF_FRandomRange(2, 30),
            GDF_FRandomRange(-XZ_BOUNDS, XZ_BOUNDS));
    }

    app.camera = cam;

    GDF_Run();

    GDF_FlushLogBuffer();

    return 0;
}

GDF_BOOL on_loop(const GDF_AppState* app_state, f64 dt, void* state)
{
    const App* app = (App*)state;

    // Handle input
    const vec3 curr_pos = GDF_CameraGetPosition(app->camera);

    const f32 spd = GDF_IsKeyDown(GDF_KEYCODE_LCONTROL) ? 16 : 8;

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
    delta                 = quaternion_rotate_vector(camera_rot, delta);

    const vec3 new_pos = vec3_add(curr_pos, delta);

    GDF_CameraSetPosition(app->camera, new_pos);

    ivec2 mouse_delta;
    GDF_GetMouseDelta(&mouse_delta);

    GDF_CameraAddPitch(app->camera, mouse_delta.y * 0.002);
    GDF_CameraAddYaw(app->camera, mouse_delta.x * 0.002);

    // Update cube simulation

    const vec3 accel = vec3_new(0, -9.81f, 0);

    // scaling the dt makes the bounce height higher?
    // dt *= 2;
    for (int i = 0; i < NUM_CUBES; i++)
    {
        BouncingCube*  cube      = &app->cubes[i];
        GDF_Transform* transform = GDF_ObjGetTransform(app->cubes[i].object);

        cube->velocity = vec3_add(cube->velocity, vec3_mul_scalar(accel, dt));

        transform->pos = vec3_add(transform->pos, vec3_mul_scalar(cube->velocity, dt));

        if (transform->pos.y < 0)
        {
            cube->velocity.y *= -1;

            f32 rebound_y = -transform->pos.y;
            transform->pos.y = rebound_y;
        }
    }

    return GDF_TRUE;
}
