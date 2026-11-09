struct PostProcessConstants
{
    uint textureIndex;
    float exposure;
};

ConstantBuffer<PostProcessConstants> pushConstant : register(b0, space999);

struct PS_IN
{
    float4 pos : SV_Position;
    float2 uv : Texcoord;
};

float3 ApplyExposure(float3 color)
{
    return color * pushConstant.exposure;
}

float3 AgXContrast(float3 x)
{
    float3 x2 = x * x;
    float3 x4 = x2 * x2;

    return 15.5f * x4 * x2
        - 40.14f * x4 * x
        + 31.96f * x4
        - 6.868f * x2 * x
        + 0.4298f * x2
        + 0.1191f * x
        - 0.00232f;
}

float3 ApplyAgX(float3 color)
{
    const float3x3 inset = float3x3(
        0.842479062253094f, 0.0784335999999992f, 0.0792237451477643f,
        0.0423282422610123f, 0.878468636469772f, 0.0791661274605434f,
        0.0423756549057051f, 0.0784336f, 0.879142973793104f);

    const float3x3 outset = float3x3(
        1.19687900512017f, -0.0980208811401368f, -0.0990297440797205f,
        -0.0528968517574562f, 1.15190312990417f, -0.0989611768448433f,
        -0.0529716355144438f, -0.0980434501171241f, 1.15107367264116f);

    const float minEv = -12.47393f;
    const float maxEv = 4.026069f;

    float3 value = mul(inset, color);
    value = clamp(log2(max(value, 1e-10f)), minEv, maxEv);
    value = (value - minEv) / (maxEv - minEv);
    value = AgXContrast(value);

    return saturate(mul(outset, value));
}

float4 PSMain(PS_IN input) : SV_Target0
{
    Texture2D tex = ResourceDescriptorHeap[pushConstant.textureIndex];

    float3 color = tex.Load(int3(input.pos.xy, 0)).rgb;

    color = ApplyExposure(color);
    color = ApplyAgX(color);

    return float4(color, 1.0f);
}