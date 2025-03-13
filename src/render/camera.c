#include <gdfe/../../include/gdfe/camera.h>
#include "../internal/irender/camera.h"

GDF_Camera GDF_CameraCreate(GDF_CameraCreateInfo* camera_info)
{
    GDF_Camera camera = GDF_Malloc(sizeof(GDF_Camera_T), GDF_MEMTAG_SCENE);

    camera->pos = camera_info->pos;

    camera->pyr.x = camera_info->pitch;
    camera->pyr.y = camera_info->yaw;
    camera->pyr.z = camera_info->roll;

    camera->aspect_ratio = camera_info->aspect_ratio;
    camera->fov = camera_info->fov;
    camera->near_clip = camera_info->near_clip;
    camera->far_clip = camera_info->far_clip;

    camera->needs_view_recalc = GDF_TRUE;
    camera->needs_persp_recalc = GDF_TRUE;
    camera->needs_dir_vecs_recalc = GDF_TRUE;

    gdfe_camera_update(camera);

    return camera;
}

void GDF_CameraDestroy(GDF_Camera camera)
{
    GDF_Free(camera);
}

void GDF_CameraSetPitch(GDF_Camera camera, f32 pitch)
{
    camera->pyr.x = pitch;

    // TODO! i might be stupid for this idk
    camera->needs_view_recalc = GDF_TRUE;
    camera->needs_dir_vecs_recalc = GDF_TRUE;
}

void GDF_CameraSetYaw(GDF_Camera camera, f32 yaw)
{
    camera->pyr.y = yaw;

    camera->needs_view_recalc = GDF_TRUE;
    camera->needs_dir_vecs_recalc = GDF_TRUE;
}

void GDF_CameraSetRoll(GDF_Camera camera, f32 roll)
{
    camera->pyr.z = roll;

    camera->needs_view_recalc = GDF_TRUE;
    camera->needs_dir_vecs_recalc = GDF_TRUE;
}

void GDF_CameraSetRotation(GDF_Camera camera, vec3 pyr)
{
    camera->pyr = pyr;

    camera->needs_view_recalc = GDF_TRUE;
    camera->needs_dir_vecs_recalc = GDF_TRUE;
}

void GDF_CameraAddPitch(GDF_Camera camera, f32 pitch)
{
    camera->pyr.x += pitch;

    camera->needs_view_recalc = GDF_TRUE;
    camera->needs_dir_vecs_recalc = GDF_TRUE;
}

void GDF_CameraAddYaw(GDF_Camera camera, f32 yaw)
{
    camera->pyr.y += yaw;

    camera->needs_view_recalc = GDF_TRUE;
    camera->needs_dir_vecs_recalc = GDF_TRUE;
}

void GDF_CameraAddRoll(GDF_Camera camera, f32 roll)
{
    camera->pyr.z += roll;

    camera->needs_view_recalc = GDF_TRUE;
    camera->needs_dir_vecs_recalc = GDF_TRUE;
}

void GDF_CameraAddRotation(GDF_Camera camera, vec3 pyr)
{
    vec3_add_to(&camera->pyr, pyr);

    camera->needs_view_recalc = GDF_TRUE;
    camera->needs_dir_vecs_recalc = GDF_TRUE;
}

void GDF_CameraSetPosition(GDF_Camera camera, vec3 position)
{
    camera->pos = position;

    camera->needs_view_recalc = GDF_TRUE;
}

void GDF_CameraSetAspectRatio(GDF_Camera camera, f32 aspect_ratio)
{
    camera->aspect_ratio = aspect_ratio;

    camera->needs_persp_recalc = GDF_TRUE;
}

void GDF_CameraSetFOV(GDF_Camera camera, f32 fov)
{
    camera->fov = fov;

    camera->needs_persp_recalc = GDF_TRUE;
}

void GDF_CameraSetNearClip(GDF_Camera camera, f32 near_clip)
{
    camera->near_clip = near_clip;

    camera->needs_persp_recalc = GDF_TRUE;
}

void GDF_CameraSetFarClip(GDF_Camera camera, f32 far_clip)
{
    camera->far_clip = far_clip;

    camera->needs_persp_recalc = GDF_TRUE;
}

f32 GDF_CameraGetPitch(GDF_Camera camera)
{
    return camera->pyr.x;
}

f32 GDF_CameraGetYaw(GDF_Camera camera)
{
    return camera->pyr.y;
}

f32 GDF_CameraGetRoll(GDF_Camera camera)
{
    return camera->pyr.z;
}

vec3 GDF_CameraGetRotation(GDF_Camera camera)
{
    return camera->pyr;
}

vec3 GDF_CameraGetPosition(GDF_Camera camera)
{
    return camera->pos;
}

f32 GDF_CameraGetAspectRatio(GDF_Camera camera)
{
    return camera->aspect_ratio;
}

f32 GDF_CameraGetFOV(GDF_Camera camera)
{
    return camera->fov;
}

f32 GDF_CameraGetNearClip(GDF_Camera camera)
{
    return camera->near_clip;
}

f32 GDF_CameraGetFarClip(GDF_Camera camera)
{
    return camera->far_clip;
}

mat4 GDF_CameraGetViewMatrix(GDF_Camera camera)
{
    if (camera->needs_view_recalc) {
        gdfe_camera_update(camera);
    }
    return camera->view_matrix;
}

mat4 GDF_CameraGetPerspectiveMatrix(GDF_Camera camera)
{
    if (camera->needs_persp_recalc) {
        gdfe_camera_update(camera);
    }
    return camera->perspective_matrix;
}

mat4 GDF_CameraGetViewPerspectiveMatrix(GDF_Camera camera)
{
    if (camera->needs_view_recalc || camera->needs_persp_recalc) {
        gdfe_camera_update(camera);
    }
    return camera->view_perspective;
}

void GDF_CameraOrientation(GDF_Camera camera, vec3* forward, vec3* right, vec3* up)
{
    if (camera->needs_dir_vecs_recalc)
    {
        vec3_orientation(
            camera->pyr.y,
            camera->pyr.x,
            camera->pyr.z,
            &camera->forward,
            &camera->right,
            &camera->up
        );
    }
    *forward = camera->forward;
    *right = camera->right;
    *up = camera->up;
}