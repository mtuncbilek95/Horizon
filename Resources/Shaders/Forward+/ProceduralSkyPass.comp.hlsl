struct ProceduralSkyConstants
{
    float4x4 invViewProj;
    float4 toSun;
    float4 sunColorIntensity;
    uint outputIndex;
    uint width;
    uint height;
    uint padding;
};

[[vk::push_constant]]
ConstantBuffer<ProceduralSkyConstants> pushConstant : register(b0, space999);

static const float3 kRayleigh = float3(0.116f, 0.270f, 0.662f);
static const float3 kMie = float3(0.05f, 0.05f, 0.05f);
static const float kMieG = 0.76f;
static const float kHorizonBias = 0.12f;

float OpticalDepth(float cosZenith)
{
    return 1.0f / (max(cosZenith, 0.0f) + kHorizonBias);
}

float3 ComputeSky(float3 direction, float3 toSun, float3 sunRadiance)
{
    const float cosSun = dot(direction, toSun);
    const float3 extinction = kRayleigh + kMie;

    const float3 sunTransmittance = exp(-extinction * OpticalDepth(toSun.y));
    const float3 viewScatter = 1.0f - exp(-extinction * OpticalDepth(direction.y));

    const float rayleighPhase = 0.75f * (1.0f + cosSun * cosSun);
    const float mieDenominator = 1.0f + kMieG * kMieG - 2.0f * kMieG * cosSun;
    const float miePhase = (1.0f - kMieG * kMieG) / (mieDenominator * sqrt(mieDenominator));

    const float3 phase = (kRayleigh * rayleighPhase + kMie * miePhase) / extinction;
    const float sunFade = smoothstep(-0.15f, 0.05f, toSun.y);

    float3 color = sunRadiance * sunTransmittance * phase * viewScatter * sunFade;

    const float sunDisk = smoothstep(0.9997f, 0.9999f, cosSun);
    color += sunRadiance * sunTransmittance * sunDisk * 20.0f * sunFade;

    const float groundFade = lerp(0.35f, 1.0f, smoothstep(-0.1f, 0.0f, direction.y));

    return color * groundFade;
}

[numthreads(8, 8, 1)]
void CSMain(uint3 dispatchId : SV_DispatchThreadID)
{
    if (dispatchId.x >= pushConstant.width || dispatchId.y >= pushConstant.height)
    {
        return;
    }

    RWTexture2D<float4> output = ResourceDescriptorHeap[pushConstant.outputIndex];

    const float2 uv = (float2(dispatchId.xy) + 0.5f) / float2(pushConstant.width, pushConstant.height);
    const float2 ndc = uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);

    const float4 nearPoint = mul(pushConstant.invViewProj, float4(ndc, 1.0f, 1.0f));
    const float4 midPoint = mul(pushConstant.invViewProj, float4(ndc, 0.5f, 1.0f));

    const float3 direction = normalize(midPoint.xyz / midPoint.w - nearPoint.xyz / nearPoint.w);
    const float3 toSun = normalize(pushConstant.toSun.xyz);
    const float3 sunRadiance = pushConstant.sunColorIntensity.rgb * pushConstant.sunColorIntensity.a;

    output[dispatchId.xy] = float4(ComputeSky(direction, toSun, sunRadiance), 1.0f);
}