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
    uint drawDebug;
};

ConstantBuffer<CullClusterConstant> pushConst : register(b0, space999);

static const uint GroupSize = 64;

groupshared float4 sLights[GroupSize];
groupshared uint sAnyActive;

[numthreads(GroupSize, 1, 1)]
void CSMain(uint3 dtid : SV_DispatchThreadID, uint3 gtid : SV_GroupThreadID)
{
    RWStructuredBuffer<uint> lightGrid = ResourceDescriptorHeap[pushConst.lightGridIndex];
    RWStructuredBuffer<uint> activeClusters = ResourceDescriptorHeap[pushConst.activeFlagsIndex];
    StructuredBuffer<ClusterAABB> clusters = ResourceDescriptorHeap[pushConst.clusterIndex];
    StructuredBuffer<PointLight> lights = ResourceDescriptorHeap[pushConst.pointLightIndex];

    uint totalClusters = pushConst.clusterCountX * pushConst.clusterCountY * pushConst.clusterCountZ;
    uint idx = dtid.x;
    bool valid = idx < totalClusters;
    bool active = valid && activeClusters[idx] != 0;

    if (gtid.x == 0)
        sAnyActive = 0;
    GroupMemoryBarrierWithGroupSync();

    if (active)
        InterlockedOr(sAnyActive, 1u);
    GroupMemoryBarrierWithGroupSync();

    uint base = idx * (pushConst.maxLightsPerCluster + 1);
    uint count = 0;

    if (sAnyActive != 0)
    {
        float3 aabbMin = 0;
        float3 aabbMax = 0;
        if (active)
        {
            aabbMin = clusters[idx].minViewSpace.xyz;
            aabbMax = clusters[idx].maxViewSpace.xyz;
        }

        for (uint batch = 0; batch < pushConst.lightCount; batch += 64)
        {
            uint li = batch + gtid.x;
            if (li < pushConst.lightCount)
            {
                PointLight l = lights[li];
                float3 vp = mul(pushConst.viewMatrix, float4(l.position, 1.0)).xyz;
                sLights[gtid.x] = float4(vp, l.range * l.range);
            }
            GroupMemoryBarrierWithGroupSync();

            if (active)
            {
                uint n = min(64, pushConst.lightCount - batch);
                for (uint j = 0; j < n && count < pushConst.maxLightsPerCluster; j++)
                {
                    float4 L = sLights[j];
                    float3 d = L.xyz - clamp(L.xyz, aabbMin, aabbMax);
                    if (dot(d, d) <= L.w)
                    {
                        lightGrid[base + 1 + count] = batch + j;
                        count++;
                    }
                }
            }
            GroupMemoryBarrierWithGroupSync();
        }
    }

    if (valid)
    {
        lightGrid[base] = count;
        if (active)
            activeClusters[idx] = 0;
    }
}