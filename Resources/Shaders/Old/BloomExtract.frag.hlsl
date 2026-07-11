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

static const float BloomBoost = 2.5;
static const float SoftKnee = 0.5;

float4 PSMain(VertexOut vertOut) : SV_Target0
{
    Texture2D hdrTex = ResourceDescriptorHeap[pushConst.hdrIndex];
    float3 extracted = hdrTex.Sample(linearClamp, vertOut.texCoord).rgb;
    float luma = dot(extracted, float3(0.299, 0.587, 0.114));
    
    float knee = pushConst.threshold * SoftKnee + 1e-4;
    float soft = clamp(luma - pushConst.threshold + knee, 0.0, 2.0 * knee);
    soft = (soft * soft) / (4.0 * knee);
    float contribution = max(soft, luma - pushConst.threshold) / max(luma, 1e-4);

    float3 bloom = extracted * contribution * BloomBoost;
    bloom = min(bloom, 8.0);
    return float4(bloom, 1.0);
}