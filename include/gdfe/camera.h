#pragma once

#include <gdfe/core.h>
#include <gdfe/math/math.h>

typedef struct GDF_Camera_T* GDF_Camera;

typedef struct GDF_CameraCreateInfo {
    // Rotation around the x-axis
    f32 pitch;
    // Rotation around the y-axis
    f32 yaw;
    // Rotation around the z-axis
    f32 roll;

    // The world position of the camera.
    vec3 pos;

    f32 aspect_ratio;
    f32 fov;
    f32 near_clip;
    f32 far_clip;
} GDF_CameraCreateInfo;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a new camera. Must be explicitly destroyed.
 *
 * @param camera_info Information about the camera to create
 * @return Handle to the newly created camera
 */
GDF_Camera GDF_CameraCreate(GDF_CameraCreateInfo* camera_info);

/**
 * @brief Destroy and deallocate the camera
 *
 * @param camera The camera to destroy
 */
void GDF_CameraDestroy(GDF_Camera camera);

/**
  * @brief Constrain the camera's pitch to a specific range. Set both params to 0 for no constraint.
  *
  * @param camera The camera to modify
  * @param min_pitch The lower bound for the pitch in radians
  * @param max_pitch The upper bound for the pitch in radians
  */
void GDF_CameraConstrainPitch(GDF_Camera camera, f32 min_pitch, f32 max_pitch);

/**
  * @brief Set the global axis of the camera, which all relative pitch/yaw/roll operations are based on
  *
  * @param camera The camera to modify
  * @param axis The axis (default: <0, 1, 0>) to perform the relative rotate operations
  */
void GDF_CameraSetGlobalAxis(GDF_Camera camera, vec3 axis);

/**
  * @brief Get the global axis of the camera, which all relative pitch/yaw/roll operations are based on
  *
  * @param camera The camera to query
  * @return The global axis unit vector
  */
vec3 GDF_CameraGetGlobalAxis(GDF_Camera camera);

/**
  * @brief Get the global axis of the camera, which all relative pitch/yaw/roll operations are based on
  *
  * @param camera The camera to query
  * @param forward The output forward vector
  * @param right The output right vector
  * @param up The output up vector
  */
void GDF_CameraGetGlobalAxes(GDF_Camera camera, vec3* forward, vec3* right, vec3* up);

/**
 * @brief Set the pitch of the camera (rotation around X axis)
 *
 * @param camera The camera to modify
 * @param pitch The new pitch value in radians
 */
void GDF_CameraSetPitch(GDF_Camera camera, f32 pitch);

/**
 * @brief Set the yaw of the camera (rotation around Y axis)
 *
 * @param camera The camera to modify
 * @param yaw The new yaw value in radians
 */
void GDF_CameraSetYaw(GDF_Camera camera, f32 yaw);

/**
 * @brief Set the roll of the camera (rotation around Z/front axis)
 *
 * @param camera The camera to modify
 * @param roll The new roll value in radians
 */
void GDF_CameraSetRoll(GDF_Camera camera, f32 roll);

/**
 * @brief Set the pitch, yaw and roll of the camera all at once in world space
 *
 * @param camera The camera to modify
 * @param pyr The new pitch, yaw, and roll values as a vec3, in world space (in radians)
 */
void GDF_CameraSetAbsoluteRotation(GDF_Camera camera, vec3 pyr);

/**
 * @brief Set the internal rotation quaternion of the camera
 *
 * @param camera The camera to modify
 * @param rotation The new rotation
 */
void GDF_CameraSetRotationQuaternion(GDF_Camera camera, quaternion rotation);

/**
 * @brief Adds to the local pitch of the camera (rotation around the camera's local X axis)
 *
 * @param camera The camera to modify
 * @param pitch The new pitch value in radians
 */
void GDF_CameraAddPitch(GDF_Camera camera, f32 pitch);

/**
 * @brief Adds to the local yaw of the camera (rotation around the camera's local Y axis)
 *
 * @param camera The camera to modify
 * @param yaw The new yaw value in radians
 */
void GDF_CameraAddYaw(GDF_Camera camera, f32 yaw);

/**
 * @brief Adds to the local roll of the camera (rotation around the camera's local Z/front axis)
 *
 * @param camera The camera to modify
 * @param roll The new roll value in radians
 */
void GDF_CameraAddRoll(GDF_Camera camera, f32 roll);

/**
 * @brief Adds to the pitch, yaw and roll of the camera all at once
 *
 * @param camera The camera to modify
 * @param pyr The new pitch, yaw, and roll values as a vec3 (in radians)
 */
void GDF_CameraAddRotation(GDF_Camera camera, vec3 pyr);

/**
 * @brief Set the position of the camera in world space
 *
 * @param camera The camera to modify
 * @param position The new position as a vec3
 */
void GDF_CameraSetPosition(GDF_Camera camera, vec3 position);

/**
 * @brief Set the aspect ratio of the camera
 *
 * @param camera The camera to modify
 * @param aspect_ratio The new aspect ratio (width/height)
 */
void GDF_CameraSetAspectRatio(GDF_Camera camera, f32 aspect_ratio);

/**
 * @brief Set the field of view of the camera
 *
 * @param camera The camera to modify
 * @param fov The new field of view in radians
 */
void GDF_CameraSetFOV(GDF_Camera camera, f32 fov);

/**
 * @brief Set the near clip plane distance
 *
 * @param camera The camera to modify
 * @param near_clip The distance to the near clip plane
 */
void GDF_CameraSetNearClip(GDF_Camera camera, f32 near_clip);

/**
 * @brief Set the far clip plane distance
 *
 * @param camera The camera to modify
 * @param far_clip The distance to the far clip plane
 */
void GDF_CameraSetFarClip(GDF_Camera camera, f32 far_clip);

/**
 * @brief Get the pitch of the camera (rotation around X axis)
 *
 * @param camera The camera to get the pitch from
 * @return f32 The current pitch value in radians
 */
f32 GDF_CameraGetPitch(GDF_Camera camera);

/**
 * @brief Get the yaw of the camera (rotation around Y axis)
 *
 * @param camera The camera to get the yaw from
 * @return f32 The current yaw value in radians
 */
f32 GDF_CameraGetYaw(GDF_Camera camera);

/**
 * @brief Get the roll of the camera (rotation around Z/front axis)
 *
 * @param camera The camera to get the roll from
 * @return f32 The current roll value in radians
 */
f32 GDF_CameraGetRoll(GDF_Camera camera);

/**
 * @brief Get the complete rotation of the camera
 *
 * @param camera The camera to get the rotation from
 * @return vec3 The current pitch, yaw, and roll values as a vec3 (in radians)
 */
vec3 GDF_CameraGetRotation(GDF_Camera camera);

/**
 * @brief Get the position of the camera in world space
 *
 * @param camera The camera to get the position from
 * @return vec3 The current position as a vec3
 */
vec3 GDF_CameraGetPosition(GDF_Camera camera);

/**
 * @brief Get the aspect ratio of the camera
 *
 * @param camera The camera to get the aspect ratio from
 * @return f32 The current aspect ratio (width/height)
 */
f32 GDF_CameraGetAspectRatio(GDF_Camera camera);

/**
 * @brief Get the field of view of the camera
 *
 * @param camera The camera to get the FOV from
 * @return f32 The current field of view in radians
 */
f32 GDF_CameraGetFOV(GDF_Camera camera);

/**
 * @brief Get the near clip plane distance
 *
 * @param camera The camera to get the near clip plane from
 * @return f32 The distance to the near clip plane
 */
f32 GDF_CameraGetNearClip(GDF_Camera camera);

/**
 * @brief Get the far clip plane distance
 *
 * @param camera The camera to get the far clip plane from
 * @return f32 The distance to the far clip plane
 */
f32 GDF_CameraGetFarClip(GDF_Camera camera);

/**
 * @brief Get the view matrix of the camera
 *
 * @param camera The camera to get the view matrix from
 * @return mat4 The current view matrix
 */
mat4 GDF_CameraGetViewMatrix(GDF_Camera camera);

/**
 * @brief Get the perspective matrix of the camera
 *
 * @param camera The camera to get the perspective matrix from
 * @return mat4 The current perspective matrix
 */
mat4 GDF_CameraGetPerspectiveMatrix(GDF_Camera camera);

/**
 * @brief Get the combined view-perspective matrix of the camera
 *
 * @param camera The camera to get the combined matrix from
 * @return mat4 The current view-perspective matrix
 */
mat4 GDF_CameraGetViewPerspectiveMatrix(GDF_Camera camera);

/**
 * @brief Get the three orientation vectors
 *
 * @param camera The camera to retrieve the vectors from
 * @param forward Output parameter for the forward direction vector
 * @param right Output parameter for the right direction vector
 * @param up Output parameter for the up direction vector
 */
void GDF_CameraOrientation(GDF_Camera camera, vec3* forward, vec3* right, vec3* up);

#ifdef __cplusplus
}
#endif