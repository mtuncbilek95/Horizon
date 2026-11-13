struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float range;
    uint drawDebug;
};

struct PointLightDebugConstants
{
    float4x4 viewProj;
    uint pointLightBufferIndex;
    float pixelRadius;
    float invViewportWidth;
    float invViewportHeight;
};

struct VertexInput
{
    uint vertexId : SV_VertexID;
    uint instanceId : SV_InstanceID;
};

struct PixelData
{
    float4 position : SV_Position;
    float2 local : TEXCOORD0;
    float3 color : COLOR0;
};

ConstantBuffer<PointLightDebugConstants> pushConstant : register(b0, space999);

static const float2 kCorners[6] =
{
    float2(-1.0f, -1.0f),
    float2(1.0f, -1.0f),
    float2(-1.0f, 1.0f),
    float2(-1.0f, 1.0f),
    float2(1.0f, -1.0f),
    float2(1.0f, 1.0f)
};

PixelData VSMain(VertexInput input)
{
    StructuredBuffer<PointLight> pointLights = ResourceDescriptorHeap[pushConstant.pointLightBufferIndex];

    const PointLight light = pointLights[input.instanceId];
    const float2 corner = kCorners[input.vertexId];
    const float2 ndcOffset = corner * pushConstant.pixelRadius * 2.0f * float2(pushConstant.invViewportWidth, pushConstant.invViewportHeight);

    float4 clipPosition = mul(pushConstant.viewProj, float4(light.position, 1.0f));
    clipPosition.xy += ndcOffset * clipPosition.w;

    if (light.drawDebug == 0)
        clipPosition = float4(0.0f, 0.0f, 0.0f, 0.0f);

    PixelData output;
    output.position = clipPosition;
    output.local = corner;
    output.color = light.color.rgb;
    return output;
}