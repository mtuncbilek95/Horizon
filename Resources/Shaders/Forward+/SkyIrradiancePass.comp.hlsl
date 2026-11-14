struct SkyIrradianceConstants
{
    float4 toSun;
    float4 sunColorIntensity;
    uint outputIndex;
};

ConstantBuffer<SkyIrradianceConstants> pushConstant : register(b0, space999);

static const float PI = 3.14159265f;
static const uint kThreadCount = 64;
static const uint kSamplesPerThread = 16;
static const uint kSampleCount = kThreadCount * kSamplesPerThread;
static const float kGoldenAngle = 2.39996323f;

static const float3 kRayleigh = float3(0.116f, 0.270f, 0.662f);
static const float3 kMie = float3(0.05f, 0.05f, 0.05f);
static const float kMieG = 0.76f;
static const float kHorizonBias = 0.12f;

groupshared float3 gsCoefficients[kThreadCount][9];

float OpticalDepth(float cosZenith)
{
    return 1.0f / (max(cosZenith, 0.0f) + kHorizonBias);
}

float3 ComputeSkyAmbient(float3 direction, float3 toSun, float3 sunRadiance)
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
    const float groundFade = lerp(0.35f, 1.0f, smoothstep(-0.1f, 0.0f, direction.y));

    return sunRadiance * sunTransmittance * phase * viewScatter * sunFade * groundFade;
}

void EvaluateBasis(float3 n, out float basis[9])
{
    basis[0] = 0.282095f;
    basis[1] = 0.488603f * n.y;
    basis[2] = 0.488603f * n.z;
    basis[3] = 0.488603f * n.x;
    basis[4] = 1.092548f * n.x * n.y;
    basis[5] = 1.092548f * n.y * n.z;
    basis[6] = 0.315392f * (3.0f * n.z * n.z - 1.0f);
    basis[7] = 1.092548f * n.x * n.z;
    basis[8] = 0.546274f * (n.x * n.x - n.y * n.y);
}

[numthreads(64, 1, 1)]
void CSMain(uint threadIndex : SV_GroupIndex)
{
    const float3 toSun = normalize(pushConstant.toSun.xyz);
    const float3 sunRadiance = pushConstant.sunColorIntensity.rgb * pushConstant.sunColorIntensity.a;

    float3 coefficients[9];

    for (uint c = 0; c < 9; c++)
        coefficients[c] = 0.0f;

    for (uint s = 0; s < kSamplesPerThread; s++)
    {
        const uint i = threadIndex * kSamplesPerThread + s;
        const float y = 1.0f - (2.0f * i + 1.0f) / kSampleCount;
        const float r = sqrt(saturate(1.0f - y * y));
        const float phi = i * kGoldenAngle;
        const float3 direction = float3(r * cos(phi), y, r * sin(phi));

        const float3 radiance = ComputeSkyAmbient(direction, toSun, sunRadiance);

        float basis[9];
        EvaluateBasis(direction, basis);

        for (uint k = 0; k < 9; k++)
            coefficients[k] += radiance * basis[k];
    }

    for (uint w = 0; w < 9; w++)
        gsCoefficients[threadIndex][w] = coefficients[w];

    GroupMemoryBarrierWithGroupSync();

    for (uint stride = kThreadCount / 2; stride > 0; stride >>= 1)
    {
        if (threadIndex < stride)
        {
            for (uint k = 0; k < 9; k++)
                gsCoefficients[threadIndex][k] += gsCoefficients[threadIndex + stride][k];
        }

        GroupMemoryBarrierWithGroupSync();
    }

    if (threadIndex >= 9)
        return;

    RWStructuredBuffer<float4> output = ResourceDescriptorHeap[pushConstant.outputIndex];

    const float weight = 4.0f * PI / kSampleCount;
    const float band = threadIndex == 0 ? 1.0f : (threadIndex < 4 ? 2.0f / 3.0f : 0.25f);

    output[threadIndex] = float4(gsCoefficients[0][threadIndex] * weight * band, 0.0f);
}