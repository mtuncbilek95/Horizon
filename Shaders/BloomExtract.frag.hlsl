struct BloomExtractPush
{
    uint hdrIndex;
    float threshold;
};

struct VertexOut
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
};

ConstantBuffer<BloomExtractPush> pushConst : register(b0);
SamplerState linearClamp : register(s2);


float4 PSMain(VertexOut vertOut) : SV_Target0
{
    Texture2D hdrTex = ResourceDescriptorHeap[pushConst.hdrIndex];
    float3 c = hdrTex.Sample(linearClamp, vertOut.texCoord).rgb;
    float luma = dot(c, float3(0.299, 0.587, 0.114));
    float contribution = max(0.0, luma - pushConst.threshold) / max(luma, 1e-4);
    return float4(c * contribution, 1.0);
}