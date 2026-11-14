struct DepthResolveConstants
{
    uint depthIndex;
};

struct PixelData
{
    float4 position : SV_Position;
    float2 uv : TexCoord;
};

ConstantBuffer<DepthResolveConstants> pushConstant : register(b0, space999);

float PSMain(PixelData input) : SV_Depth
{
    Texture2DMS<float> sceneDepth = ResourceDescriptorHeap[pushConstant.depthIndex];

    uint width;
    uint height;
    uint sampleCount;
    sceneDepth.GetDimensions(width, height, sampleCount);

    const int2 pixel = int2(input.position.xy);
    float nearest = 0.0f;

    for (uint i = 0; i < sampleCount; i++)
        nearest = max(nearest, sceneDepth.Load(pixel, i));

    return nearest;
}
