#include <gdfe/camera.h>
#include "irender/camera.h"

GDF_Camera GDF_CameraCreate(GDF_CameraCreateInfo* camera_info)
{
    GDF_Camera camera = GDF_Malloc(sizeof(GDF_Camera_T), GDF_MEMTAG_SCENE);

    camera->pos = camera_info->pos;

    camera->euler = vec3_new(camera_info->pitch, camera_info->yaw, camera_info->roll);
    camera->rotation = quaternion_from_euler(camera->euler);
    camera->axis = vec3_new(0, 1, 0);
    camera->axis_right = vec3_new(1, 0, 0);
    camera->axis_forward = vec3_new(0, 0, 1);

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

void GDF_CameraConstrainPitch(GDF_Camera camera, f32 min_pitch, f32 max_pitch)
{
    camera->constrain_pitch = min_pitch || max_pitch;

    if (camera->constrain_pitch)
    {
        camera->min_pitch = min_pitch;
        camera->max_pitch = max_pitch;

        if (camera->euler.x < min_pitch)
        {
            camera->euler.x = min_pitch;
            camera->rotation = quaternion_from_euler(camera->euler);
            return;
        }
        if (camera->euler.x > max_pitch)
        {
            camera->euler.x = max_pitch;
            camera->rotation = quaternion_from_euler(camera->euler);
        }
    }
}

void GDF_CameraSetGlobalAxis(GDF_Camera camera, vec3 new_axis)
{
    vec3 old_axis = camera->axis;
    vec3 normalized_new_axis = vec3_normalized(new_axis);

    if (vec3_distance(old_axis, normalized_new_axis) < FLOAT_EPSILON)
        return;

    // find rotation from the old axis to the new axis
    vec3 rotation_axis = vec3_cross(old_axis, normalized_new_axis);

    // if vectors are parallel, pick an arbitrary perpendicular axis
    if (vec3_length(rotation_axis) < FLOAT_EPSILON) {
        if (fabs(old_axis.x) < fabs(old_axis.y)) {
            rotation_axis = vec3_cross(old_axis, (vec3){1, 0, 0});
        } else {
            rotation_axis = vec3_cross(old_axis, (vec3){0, 1, 0});
        }

        // if axes are opposite, rotate 180 degrees
        float angle = PI;

        quaternion rot = quaternion_from_axis_angle(rotation_axis, angle);
        quaternion_orientation(
            rot, &camera->axis_forward, &camera->axis_right, &camera->axis
        );
        camera->rotation = quaternion_mul(rot, camera->rotation);
        return;
    }

    float dot = vec3_dot(old_axis, normalized_new_axis);
    float angle = acos(CLAMP(dot, -1.0f, 1.0f));

    camera->axis = normalized_new_axis;

    quaternion rot = quaternion_from_axis_angle(rotation_axis, angle);
    quaternion_orientation(
        rot, &camera->axis_forward, &camera->axis_right, &camera->axis
    );
    camera->rotation = quaternion_mul(rot, camera->rotation);
}

vec3 GDF_CameraGetGlobalAxis(GDF_Camera camera)
{
    return camera->axis;
}

void GDF_CameraGetGlobalAxes(GDF_Camera camera, vec3* forward, vec3* right, vec3* up)
{
    *forward = camera->axis_forward;
    *right = camera->axis_right;
    *up = camera->axis;
}

void GDF_CameraSetPitch(GDF_Camera camera, f32 pitch)
{
    if (pitch == camera->euler.x)
        return;

    TODO("This is incorrect.")

    camera->euler.x = pitch;

    quaternion pitch_quat = quaternion_from_axis_angle(camera->axis_right, pitch);
    camera->rotation = quaternion_mul(pitch_quat, camera->rotation);

    // TODO! i might be stupid for this idk
    camera->needs_view_recalc = GDF_TRUE;
    camera->needs_dir_vecs_recalc = GDF_TRUE;
}

void GDF_CameraSetYaw(GDF_Camera camera, f32 yaw)
{
    if (yaw == camera->euler.y)
        return;

    TODO("This is incorrect.")

    camera->euler.y = yaw;

    quaternion yaw_quat = quaternion_from_axis_angle(camera->axis, yaw);
    camera->rotation = quaternion_mul(yaw_quat, camera->rotation);

    camera->needs_view_recalc = GDF_TRUE;
    camera->needs_dir_vecs_recalc = GDF_TRUE;
}

void GDF_CameraSetRoll(GDF_Camera camera, f32 roll)
{
    if (roll == camera->euler.z)
        return;

    TODO("This is incorrect.")

    camera->euler.z = roll;

    quaternion roll_quat = quaternion_from_axis_angle(camera->forward, roll);
    camera->rotation = quaternion_mul(roll_quat, camera->rotation);

    camera->needs_view_recalc = GDF_TRUE;
    camera->needs_dir_vecs_recalc = GDF_TRUE;
}

void GDF_CameraSetAbsoluteRotation(GDF_Camera camera, vec3 pyr)
{
    camera->rotation = quaternion_from_euler(pyr);

    camera->needs_view_recalc = GDF_TRUE;
    camera->needs_dir_vecs_recalc = GDF_TRUE;
}

void GDF_CameraSetRotationQuaternion(GDF_Camera camera, quaternion rotation)
{
    camera->rotation = rotation;

    camera->needs_view_recalc = GDF_TRUE;
    camera->needs_dir_vecs_recalc = GDF_TRUE;
}

void GDF_CameraAddPitch(GDF_Camera camera, f32 pitch)
{
    if (pitch == 0)
        return;

    f32 new_pitch = camera->euler.x + pitch;

    if (camera->needs_dir_vecs_recalc)
        gdfe_camera_update_orientation_vecs(camera);

    quaternion pitch_quat;
    if (camera->constrain_pitch)
    {
        if (new_pitch > camera->max_pitch)
        {
            if (camera->max_pitch == camera->euler.x)
                return;
            new_pitch = camera->max_pitch;
        }
        else if (new_pitch < camera->min_pitch)
        {
            if (camera->min_pitch == camera->euler.x)
                return;
            new_pitch = camera->min_pitch;
        }
        f32 d_pitch = new_pitch - camera->euler.x;
        camera->euler.x = new_pitch;
        // TODO! this breaks down when roll is non zero. need to
        // rotate by camera right projected onto the same plane as
        // axis forward and axis right. MAN wtf
        pitch_quat = quaternion_from_axis_angle(camera->right, d_pitch);
    }
    else
    {
        pitch_quat = quaternion_from_axis_angle(camera->right, pitch);
    }

    camera->rotation = quaternion_mul(pitch_quat, camera->rotation);

    camera->needs_view_recalc = GDF_TRUE;
    camera->needs_dir_vecs_recalc = GDF_TRUE;
}

void GDF_CameraAddYaw(GDF_Camera camera, f32 yaw)
{
    if (yaw == 0)
        return;

    camera->euler.y += yaw;

    quaternion yaw_quat = quaternion_from_axis_angle(camera->axis, yaw);
    camera->rotation = quaternion_mul(yaw_quat, camera->rotation);

    camera->needs_view_recalc = GDF_TRUE;
    camera->needs_dir_vecs_recalc = GDF_TRUE;
}

void GDF_CameraAddRoll(GDF_Camera camera, f32 roll)
{
    if (roll == 0)
        return;

    if (camera->needs_dir_vecs_recalc)
        gdfe_camera_update_orientation_vecs(camera);

    camera->euler.z += roll;

    quaternion roll_quat = quaternion_from_axis_angle(camera->axis_forward, roll);
    camera->rotation = quaternion_mul(roll_quat, camera->rotation);

    camera->needs_view_recalc = GDF_TRUE;
    camera->needs_dir_vecs_recalc = GDF_TRUE;
}

void GDF_CameraAddRotation(GDF_Camera camera, vec3 pyr)
{
    // vec3_add_to(&camera->pyr, pyr);

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
    // return camera->pyr.x;
}

f32 GDF_CameraGetYaw(GDF_Camera camera)
{
    // return camera->pyr.y;
}

f32 GDF_CameraGetRoll(GDF_Camera camera)
{
    // return camera->pyr.z;
}

vec3 GDF_CameraGetRotation(GDF_Camera camera)
{
    // return camera->pyr;
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
        quaternion_orientation(
            camera->rotation,
            &camera->forward,
            &camera->right,
            &camera->up
        );
    }
    *forward = camera->forward;
    *right = camera->right;
    *up = camera->up;
}