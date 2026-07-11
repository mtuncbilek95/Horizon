struct BloomBlurPush
{
    uint srcIndex;
    uint horizontal;
};

struct VertexOut
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
};

ConstantBuffer<BloomBlurPush> pushConst : register(b0);
SamplerState linearClamp : register(s2);

static const float Offsets[3] = { 0.0, 1.3846153846, 3.2307692308 };
static const float Weights[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };
static const float BlurScale = 1.0;

float4 PSMain(VertexOut vertOut) : SV_Target0
{
    Texture2D srcTex = ResourceDescriptorHeap[pushConst.srcIndex];
    uint width, height;
    srcTex.GetDimensions(width, height);
    float2 texel = 1.0 / float2(width, height);
    float2 dir = (pushConst.horizontal != 0) ? float2(texel.x, 0.0) : float2(0.0, texel.y) * BlurScale;

    float3 result = srcTex.Sample(linearClamp, vertOut.texCoord).rgb * Weights[0];
    [unroll]
    for (int i = 1; i < 3; i++)
    {
        float2 off = dir * Offsets[i];
        result += srcTex.Sample(linearClamp, vertOut.texCoord + off).rgb * Weights[i];
        result += srcTex.Sample(linearClamp, vertOut.texCoord - off).rgb * Weights[i];
    }
    return float4(result, 1.0);
}