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

static const float Weights[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };

float4 PSMain(VertexOut vertOut) : SV_Target0
{
    Texture2D srcTex = ResourceDescriptorHeap[pushConst.srcIndex];
    uint width, height;
    srcTex.GetDimensions(width, height);
    float2 texel = 1.0 / float2(width, height);
    float2 dir = (pushConst.horizontal != 0) ? float2(texel.x, 0.0) : float2(0.0, texel.y);

    float3 result = srcTex.Sample(linearClamp, vertOut.texCoord).rgb * Weights[0];
    [unroll]
    for (int i = 1; i < 5; i++)
    {
        result += srcTex.Sample(linearClamp, vertOut.texCoord + dir * i).rgb * Weights[i];
        result += srcTex.Sample(linearClamp, vertOut.texCoord - dir * i).rgb * Weights[i];
    }
    return float4(result, 1.0);
}