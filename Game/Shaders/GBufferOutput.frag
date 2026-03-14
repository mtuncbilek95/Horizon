#version 460

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3 inWorldPos;
layout(location = 2) in vec3 inWorldNormal;
layout(location = 3) in vec4 inWorldTangent;
layout(location = 4) flat in uint inMeshletID;

layout(set = 1, binding = 0) uniform sampler2D albedoMap;
layout(set = 1, binding = 1) uniform sampler2D normalMap;
layout(set = 1, binding = 2) uniform sampler2D pbrMap;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outWorldNormal;
layout(location = 2) out vec4 outWorldPos;
layout(location = 3) out vec4 outPBR;
layout(location = 4) out vec4 outMeshletView;

vec3 getMeshletColor(uint id) {
    uint hash = (id ^ 61) ^ (id >> 16);
    hash = hash + (hash << 3);
    hash = hash ^ (hash >> 4);
    hash = hash * 0x27d4eb2d;
    hash = hash ^ (hash >> 15);
    
    float r = float(hash & 255) / 255.0;
    float g = float((hash >> 8) & 255) / 255.0;
    float b = float((hash >> 16) & 255) / 255.0;
    
    return vec3(r, g, b);
}

void main() {
    vec3 N = normalize(inWorldNormal);
    vec3 T = normalize(inWorldTangent.xyz);
    vec3 B = cross(N, T) * inWorldTangent.w;
    
    mat3 TBN = mat3(T, B, N);

    vec3 sampledNormal = texture(normalMap, inUV).xyz * 2.0 - 1.0;
    vec3 finalWorldNormal = normalize(TBN * sampledNormal);

    vec4 albedoColor = texture(albedoMap, inUV);
    vec3 pbrData = texture(pbrMap, inUV).xyz;

    outAlbedo = albedoColor;
    outWorldNormal = vec4(finalWorldNormal, 1.0);
    outWorldPos = vec4(inWorldPos, 1.0);
    outPBR = vec4(pbrData, 1.0);
    
    vec3 meshletColor = getMeshletColor(inMeshletID);
    outMeshletView = vec4(meshletColor, 1.0);
}