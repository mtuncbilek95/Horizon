#version 460
#extension GL_EXT_mesh_shader : require

layout(location = 0) perprimitiveEXT flat in uint inMeshletID;
layout(location = 1) perprimitiveEXT flat in uint inLodLevel;

layout(location = 0) out vec4 outColor;

void main()
{
    uint id = inMeshletID;

    id = id * 747796405u + 2891336453u;
    id = ((id >> ((id >> 28u) + 4u)) ^ id) * 277803737u;
    id = (id >> 22u) ^ id;

    float r = float((id      ) & 0xFF) / 255.0;
    float g = float((id >>  8) & 0xFF) / 255.0;
    float b = float((id >> 16) & 0xFF) / 255.0;

    float brightness = 1.0 - float(inLodLevel) * 0.25;

    outColor = vec4(r * brightness, g * brightness, b * brightness, 1.0);
}