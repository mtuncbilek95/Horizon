#version 460
#extension GL_EXT_mesh_shader : require

layout(location = 0) flat in uint inMeshletID;

layout(location = 0) out vec4 outColor;

void main()
{
    float r = fract(sin(float(inMeshletID) * 12.9898) * 43758.5453);
    float g = fract(sin(float(inMeshletID) * 78.233) * 43758.5453);
    float b = fract(sin(float(inMeshletID) * 45.164) * 43758.5453);
    
    outColor = vec4(r, g, b, 1.0);
}