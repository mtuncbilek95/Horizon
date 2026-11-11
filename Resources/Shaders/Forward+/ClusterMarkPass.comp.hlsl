struct MarkClusterConstant
{
    uint depthIndex;
    uint activeFlagsIndex;
    uint width;
    uint height;
    uint tileSize;
    uint clusterX;
    uint clusterY;
    uint clusterZ;
    float nPlane;
    float fPlane;
};

[[vk::push_constant]]
ConstantBuffer<MarkClusterConstant> pushConst : register(b0, space999);

uint ClusterIndex(uint2 pixel, float viewZ)
{
    float logRange = log2(pushConst.fPlane / pushConst.nPlane);
    float sliceScale = pushConst.clusterZ / logRange;
    float sliceBias = -(pushConst.clusterZ * log2(pushConst.nPlane) / logRange);

    int slice = (int) floor(log2(viewZ) * sliceScale + sliceBias);
    slice = clamp(slice, 0, (int) pushConst.clusterZ - 1);

    uint2 tile = pixel / pushConst.tileSize;
    tile = min(tile, uint2(pushConst.clusterX - 1, pushConst.clusterY - 1));

    return tile.x + tile.y * pushConst.clusterX + (uint) slice * pushConst.clusterX * pushConst.clusterY;
}

float LinearViewZ(float d)
{
    return pushConst.nPlane * pushConst.fPlane / (pushConst.fPlane - d * (pushConst.fPlane - pushConst.nPlane));
}

[numthreads(8, 8, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    if (id.x >= pushConst.width || id.y >= pushConst.height)
    {
        return;
    }

    Texture2DMS<float> depthTex = ResourceDescriptorHeap[pushConst.depthIndex];
    RWStructuredBuffer<uint> activeClusters = ResourceDescriptorHeap[pushConst.activeFlagsIndex];

    uint texWidth;
    uint texHeight;
    uint sampleCount;
    depthTex.GetDimensions(texWidth, texHeight, sampleCount);

    for (uint s = 0; s < sampleCount; s++)
    {
        float d = depthTex.Load(id.xy, s);
        if (d <= 0.0)
            continue;

        float viewZ = pushConst.nPlane / d;
        activeClusters[ClusterIndex(id.xy, viewZ)] = 1;
    }
}