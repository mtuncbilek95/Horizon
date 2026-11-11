struct SkyCompositeConstants
{
    uint skyTextureIndex;
};

struct PixelData
{
    float4 position : SV_Position;
    float2 uv : TexCoord;
};

ConstantBuffer<SkyCompositeConstants> pushConstant : register(b0, space999);

float4 PSMain(PixelData input) : SV_Target0
{
    Texture2D<float4> sky = ResourceDescriptorHeap[pushConstant.skyTextureIndex];

    return float4(sky.Load(int3(input.position.xy, 0)).rgb, 1.0f);
}