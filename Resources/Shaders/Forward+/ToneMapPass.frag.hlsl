struct TonemapConstants
{
    uint textureIndex;
};

struct PixelData
{
    float4 position : SV_Position;
    float2 uv : TexCoord;
};

ConstantBuffer<TonemapConstants> pushConstant : register(b0, space999);

static const float kAgxMinEv = -12.47393f;
static const float kAgxMaxEv = 4.026069f;

static const float3x3 kAgxInset = float3x3(
    0.842479062253094f, 0.0423282422610123f, 0.0423756549057051f,
    0.0784335999999992f, 0.878468636469772f, 0.0784336f,
    0.0792237451477643f, 0.0791661274605434f, 0.879142973793104f);

static const float3x3 kAgxOutset = float3x3(
    1.19687900512017f, -0.0528968517574562f, -0.0529716355144438f,
    -0.0980208811401368f, 1.15190312990417f, -0.0980434501171241f,
    -0.0990297440797205f, -0.0989611768448433f, 1.15107367264116f);

float3 AgxContrast(float3 x)
{
    const float3 x2 = x * x;
    const float3 x4 = x2 * x2;
    return 15.5f * x4 * x2 - 40.14f * x4 * x + 31.96f * x4 - 6.868f * x2 * x + 0.4298f * x2 + 0.1191f * x - 0.00232f;
}

float3 Agx(float3 color)
{
    float3 encoded = mul(color, kAgxInset);
    encoded = clamp(log2(max(encoded, 1e-10f)), kAgxMinEv, kAgxMaxEv);
    encoded = (encoded - kAgxMinEv) / (kAgxMaxEv - kAgxMinEv);
    encoded = AgxContrast(encoded);
    return saturate(mul(encoded, kAgxOutset));
}

float4 PSMain(PixelData input) : SV_Target0
{
    Texture2D<float4> sceneColor = ResourceDescriptorHeap[pushConstant.textureIndex];
    const float3 hdr = sceneColor.Load(int3(input.position.xy, 0)).rgb;

    return float4(Agx(hdr), 1.0f);
}