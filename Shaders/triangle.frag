#version 460

layout(set = 0, binding = 1) uniform SceneData {
    vec4 ambientColor;
    vec4 lightDirection;
    vec4 lightColor;
} scene;

layout(set = 1, binding = 0) uniform sampler2D albedoMap;

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 normal = normalize(inNormal);
    float diffuse = max(dot(normal, scene.lightDirection.xyz), 0.0);
    vec4 albedo = texture(albedoMap, inUV);
    outColor = albedo * (scene.ambientColor + scene.lightColor * diffuse);
}