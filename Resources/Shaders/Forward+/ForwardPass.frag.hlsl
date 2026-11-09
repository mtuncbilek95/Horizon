struct ForwardConstants
{
    float4x4 worldViewProj;
    float4 worldRow0;
    float4 worldRow1;
    float4 worldRow2;
    uint vertexBufferIndex;
    uint firstVertex;
    uint pointLightBufferIndex;
    uint pointLightCount;
};

struct GpuPointLight
{
    float4 colorIntensity;
    float4 positionRange;
};

struct PixelData
{
    float4 position : SV_Position;
    float3 worldPosition : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
};

ConstantBuffer<ForwardConstants> pushConstant : register(b0, space999);

float4 PSMain(PixelData input) : SV_Target0
{
    StructuredBuffer<GpuPointLight> pointLights = ResourceDescriptorHeap[pushConstant.pointLightBufferIndex];

    const float3 normal = normalize(input.normal);
    const float3 ambientDirection = normalize(float3(0.4f, 0.8f, 0.3f));

    float3 lighting = (dot(normal, ambientDirection) * 0.5f + 0.5f) * 0.05f;

    for (uint i = 0; i < pushConstant.pointLightCount; i++)
    {
        const GpuPointLight light = pointLights[i];

        const float3 toLight = light.positionRange.xyz - input.worldPosition;
        const float distance = length(toLight);
        const float falloff = saturate(1.0f - distance / light.positionRange.w);
        const float diffuse = saturate(dot(normal, toLight / max(distance, 0.0001f)));

        lighting += light.colorIntensity.rgb * light.colorIntensity.a * diffuse * falloff * falloff;
    }

    return float4(input.color.rgb * lighting, 1.0f);
}