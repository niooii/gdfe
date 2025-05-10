#pragma once
#include <gdfe/math/math.h>
#include "types.h"

FORCEINLINE void gdfe_camera_update_orientation_vecs(GDF_Camera_T* camera)
{
    quaternion_orientation(camera->rotation, &camera->forward, &camera->right, &camera->up);
}

FORCEINLINE void gdfe_camera_update(GDF_Camera_T* camera)
{
    GDF_BOOL matrices_changed = GDF_FALSE;

    if (camera->needs_view_recalc)
    {
        camera->view_matrix       = mat4_view_quaternion(camera->pos, camera->rotation);
        camera->needs_view_recalc = GDF_FALSE;
        matrices_changed          = GDF_TRUE;
        if (camera->needs_dir_vecs_recalc)
        {
            gdfe_camera_update_orientation_vecs(camera);
        }
    }

    if (camera->needs_persp_recalc)
    {
        camera->perspective_matrix = mat4_perspective(
            camera->fov, camera->aspect_ratio, camera->near_clip, camera->far_clip);
        camera->needs_persp_recalc = GDF_FALSE;
        matrices_changed           = GDF_TRUE;
    }

    if (matrices_changed)
    {
        camera->view_perspective = mat4_mul(camera->view_matrix, camera->perspective_matrix);
    }
}
