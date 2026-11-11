struct ForwardConstants
{
    float4x4 worldViewProj;
    float4x4 worldMat;
    uint vertexBufferIndex;
    uint firstVertex;
    uint pointLightBufferIndex;
    uint lightFrameBufferIndex;
    uint lightGridIndex;
    uint tileSize;
    uint clusterCountX;
    uint clusterCountY;
    uint clusterCountZ;
    uint maxLightsPerCluster;
    float nearPlane;
    float farPlane;
};

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float range;
};

struct GpuLightFrame
{
    float3 sunColor;
    float intensity;
    float3 direction;
    uint pointLightCount;
};

struct PixelData
{
    float4 position : SV_Position;
    centroid float3 worldPosition : TEXCOORD0;
    centroid float3 normal : NORMAL0;
    centroid float4 color : COLOR0;
    centroid float viewDepth : TEXCOORD1;
};

[[vk::push_constant]]
ConstantBuffer<ForwardConstants> pushConstant : register(b0, space999);

uint ClusterIndex(uint2 pixel, float viewZ)
{
    float logRange = log2(pushConstant.farPlane / pushConstant.nearPlane);
    float sliceScale = pushConstant.clusterCountZ / logRange;
    float sliceBias = -(pushConstant.clusterCountZ * log2(pushConstant.nearPlane) / logRange);

    int slice = (int) floor(log2(viewZ) * sliceScale + sliceBias);
    slice = clamp(slice, 0, (int) pushConstant.clusterCountZ - 1);

    uint2 tile = pixel / pushConstant.tileSize;
    tile = min(tile, uint2(pushConstant.clusterCountX - 1, pushConstant.clusterCountY - 1));

    return tile.x + tile.y * pushConstant.clusterCountX + (uint) slice * pushConstant.clusterCountX * pushConstant.clusterCountY;
}

float4 PSMain(PixelData input) : SV_Target0
{
    StructuredBuffer<PointLight> pointLights = ResourceDescriptorHeap[pushConstant.pointLightBufferIndex];
    StructuredBuffer<GpuLightFrame> lightFrames = ResourceDescriptorHeap[pushConstant.lightFrameBufferIndex];
    StructuredBuffer<uint> lightGrid = ResourceDescriptorHeap[pushConstant.lightGridIndex];

    const GpuLightFrame lightFrame = lightFrames[0];

    const float3 normal = normalize(input.normal);
    const float3 ambientDirection = normalize(float3(0.4f, 0.8f, 0.3f));

    float3 lighting = (dot(normal, ambientDirection) * 0.5f + 0.5f) * 0.15f;

    const float3 toSun = -lightFrame.direction;
    const float sunLengthSq = dot(toSun, toSun);
    const float sunDiffuse = saturate(dot(normal, toSun * rsqrt(max(sunLengthSq, 0.00000001f))));

    lighting += lightFrame.sunColor * lightFrame.intensity * sunDiffuse;

    const uint cluster = ClusterIndex(uint2(input.position.xy), input.viewDepth);
    const uint base = cluster * (pushConstant.maxLightsPerCluster + 1);
    const uint lightCount = min(lightGrid[base], pushConstant.maxLightsPerCluster);

    for (uint i = 0; i < lightCount; i++)
    {
        const PointLight light = pointLights[lightGrid[base + 1 + i]];

        const float3 toLight = light.position - input.worldPosition;
        const float distance = length(toLight);
        const float falloff = saturate(1.0f - distance / light.range);
        const float diffuse = saturate(dot(normal, toLight / max(distance, 0.0001f)));

        lighting += light.color.rgb * light.intensity * diffuse * falloff * falloff;
    }

    return float4(input.color.rgb * lighting, 1.0f);
}