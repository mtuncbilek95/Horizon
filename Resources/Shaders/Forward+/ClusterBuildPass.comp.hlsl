struct ClusterConstants
{
    float4x4 inverseProj;
    uint clusterIndex;
    uint width;
    uint height;
    uint tileSize;
    uint clusterCountX;
    uint clusterCountY;
    uint clusterCountZ;
    float nearPlane;
    float farPlane;
};

struct ClusterAABB
{
    float4 minViewSpace;
    float4 maxViewSpace;
};

[[vk::push_constant]]
ConstantBuffer<ClusterConstants> pushConst : register(b0, space999);

float3 ScreenToViewFar(float2 px)
{
    float2 uv = px / float2(pushConst.width, pushConst.height);
    float2 ndc = uv * float2(2.0, -2.0) + float2(-1.0, 1.0);
    float4 view = mul(pushConst.inverseProj, float4(ndc, 1.0, 1.0));
    return view.xyz / view.w;
}

float3 RayAtZ(float3 dir, float zPlane)
{
    return dir * (zPlane / -dir.z);
}

[numthreads(1, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    uint idx = id.x + id.y * pushConst.clusterCountX + id.z * pushConst.clusterCountX * pushConst.clusterCountY;
    
    float3 dMin = ScreenToViewFar(float2(id.xy) * pushConst.tileSize);
    float3 dMax = ScreenToViewFar(float2(id.xy + 1) * pushConst.tileSize);

    float zN = pushConst.nearPlane * pow(pushConst.farPlane / pushConst.nearPlane, (float) id.z / pushConst.clusterCountZ);
    float zF = pushConst.nearPlane * pow(pushConst.farPlane / pushConst.nearPlane, (float) (id.z + 1) / pushConst.clusterCountZ);

    float3 a = RayAtZ(dMin, zN), b = RayAtZ(dMin, zF);
    float3 c = RayAtZ(dMax, zN), d = RayAtZ(dMax, zF);
    
    RWStructuredBuffer<ClusterAABB> clusters = ResourceDescriptorHeap[pushConst.clusterIndex];
    
    clusters[idx].minViewSpace = float4(min(min(a, b), min(c, d)), 0);
    clusters[idx].maxViewSpace = float4(max(max(a, b), max(c, d)), 0);
}