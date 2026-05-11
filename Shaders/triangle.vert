#version 460

layout(set = 0, binding = 0) uniform CameraData {
    mat4 view;
    mat4 projection;
    vec3 position;
} camera;

layout(push_constant) uniform PushData {
    mat4 model;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;

void main() {
    gl_Position = camera.projection * camera.view * push.model * vec4(inPosition, 1.0);
    outNormal = mat3(push.model) * inNormal;
    outUV = inUV;
}