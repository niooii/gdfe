#pragma once

typedef struct GDF_Camera_T {
    vec3 pos;

    GDF_BOOL   constrain_pitch;
    f32        min_pitch;
    f32        max_pitch;
    vec3       euler;
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