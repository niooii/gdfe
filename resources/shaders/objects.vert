#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view_projection;
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 transform;
} pc;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;

void main()
{
    gl_Position = ubo.view_projection * pc.transform * vec4(in_position, 1.0);
}