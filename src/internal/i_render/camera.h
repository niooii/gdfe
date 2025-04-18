#pragma once
#include <gdfe/math/math.h>

typedef struct GDF_Camera_T {
    vec3 pos;

    GDF_BOOL constrain_pitch;
    f32 min_pitch;
    f32 max_pitch;
    vec3 euler;
    quaternion rotation;

    vec3 axis;
    vec3 axis_right;
    vec3 axis_forward;

    mat4 view_matrix;
    mat4 perspective_matrix;
    // view_matrix * perspective_matrix
    mat4 view_perspective;

    f32 aspect_ratio;
    f32 fov;
    f32 near_clip;
    f32 far_clip;

    vec3 up;
    vec3 right;
    vec3 forward;

    GDF_BOOL needs_view_recalc;
    GDF_BOOL needs_persp_recalc;
    GDF_BOOL needs_dir_vecs_recalc;
} GDF_Camera_T;

FORCEINLINE void gdfe_camera_update_orientation_vecs(GDF_Camera_T* camera)
{
    quaternion_orientation(
        camera->rotation,
        &camera->forward,
        &camera->right,
        &camera->up
    );
}

FORCEINLINE void gdfe_camera_update(GDF_Camera_T* camera)
{
    GDF_BOOL matrices_changed = GDF_FALSE;

    if (camera->needs_view_recalc) {
        camera->view_matrix = mat4_view_quaternion(
            camera->pos,
            camera->rotation
        );
        camera->needs_view_recalc = GDF_FALSE;
        matrices_changed = GDF_TRUE;
        if (camera->needs_dir_vecs_recalc)
        {
            gdfe_camera_update_orientation_vecs(camera);
        }
    }

    if (camera->needs_persp_recalc) {
        camera->perspective_matrix = mat4_perspective(
            camera->fov,
            camera->aspect_ratio,
            camera->near_clip,
            camera->far_clip
        );
        camera->needs_persp_recalc = GDF_FALSE;
        matrices_changed = GDF_TRUE;
    }

    if (matrices_changed) {
        camera->view_perspective = mat4_mul(camera->view_matrix, camera->perspective_matrix);
    }
}