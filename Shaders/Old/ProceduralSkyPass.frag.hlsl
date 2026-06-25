#include "Sky.hlsli"

struct SkyPush
{
    uint depthIndex;
    uint frameBufferIndex;
    uint frameSlot;
};

struct FrameConstants
{
    float4x4 viewProj;
    float4x4 invViewProj;
    float4x4 lightViewProj;
    float4 lightDirection;
    float4 lightColor;
    float3 camPos;
    float _pad;
};

struct VertexOut
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
};

ConstantBuffer<SkyPush> pushConst : register(b0);

float4 PSMain(VertexOut vertOut) : SV_Target0
{
    Texture2D<float> depthTex = ResourceDescriptorHeap[pushConst.depthIndex];

    int2 pixel = int2(vertOut.position.xy);
    float depth = depthTex.Load(int3(pixel, 0));

    if (depth != 0.0)
        discard;

    StructuredBuffer<FrameConstants> frames = ResourceDescriptorHeap[pushConst.frameBufferIndex];
    FrameConstants frame = frames[pushConst.frameSlot];

    float2 ndc = vertOut.texCoord * 2.0 - 1.0;
    ndc.y = -ndc.y;
    float4 world = mul(frame.invViewProj, float4(ndc, 0.5, 1.0));
    float3 worldPos = world.xyz / world.w;
    float3 rayDir = normalize(worldPos - frame.camPos);

    float3 sunDir = normalize(-frame.lightDirection.xyz);
    return float4(SkyWithSun(rayDir, sunDir), 1.0);
}