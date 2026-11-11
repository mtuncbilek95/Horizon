struct CullClusterConstant
{
    float4x4 viewMatrix;
    uint clusterIndex;
    uint pointLightIndex;
    uint lightGridIndex;
    uint lightCount;
    uint clusterCountX;
    uint clusterCountY;
    uint clusterCountZ;
    uint maxLightsPerCluster;
    uint activeFlagsIndex;
};

struct ClusterAABB
{
    float4 minViewSpace;
    float4 maxViewSpace;
};

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float range;
};

ConstantBuffer<CullClusterConstant> pushConst : register(b0, space999);

[numthreads(1, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    uint idx = id.x + id.y * pushConst.clusterCountX + id.z * pushConst.clusterCountX * pushConst.clusterCountY;
    uint base = idx * (pushConst.maxLightsPerCluster + 1);

    RWStructuredBuffer<uint> lightGrid = ResourceDescriptorHeap[pushConst.lightGridIndex];
    RWStructuredBuffer<uint> activeClusters = ResourceDescriptorHeap[pushConst.activeFlagsIndex];

    if (activeClusters[idx] == 0)
    {
        lightGrid[base] = 0;
        return;
    }

    activeClusters[idx] = 0;

    StructuredBuffer<ClusterAABB> clusters = ResourceDescriptorHeap[pushConst.clusterIndex];
    StructuredBuffer<PointLight> lights = ResourceDescriptorHeap[pushConst.pointLightIndex];

    float3 aabbMin = clusters[idx].minViewSpace.xyz;
    float3 aabbMax = clusters[idx].maxViewSpace.xyz;

    uint count = 0;

    for (uint i = 0; i < pushConst.lightCount && count < pushConst.maxLightsPerCluster; i++)
    {
        PointLight light = lights[i];

        float3 viewPos = mul(pushConst.viewMatrix, float4(light.position, 1.0)).xyz;
        float3 closest = clamp(viewPos, aabbMin, aabbMax);
        float3 delta = viewPos - closest;

        if (dot(delta, delta) <= light.range * light.range)
        {
            lightGrid[base + 1 + count] = i;
            count++;
        }
    }

    lightGrid[base] = count;
}