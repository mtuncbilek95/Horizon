struct ShadowResolveConstants
{
    float4x4 invViewProj;
    float4x4 lightViewProj;
    float4 cameraPos;
    float4 lightDirection;
    float2 shadowMapSize;
    float shadowWorldSize;
    float shadowDepthRange;
    float lightSize;
    float maxPenumbra;
    float depthBias;
    float normalBias;
    uint width;
    uint height;
    uint sceneDepthIndex;
    uint shadowDepthIndex;
    uint outputIndex;
};

ConstantBuffer<ShadowResolveConstants> pushConstant : register(b0, space999);

static const int BLOCKER_SAMPLES = 32;
static const int PCF_SAMPLES = 32;
static const float MIN_FILTER_TEXELS = 1.5f;
static const float MAX_SLOPE = 4.0f;
static const float TWO_PI = 6.28318531f;
static const float GOLDEN_ANGLE = 2.39996323f;

float3 ReconstructWorld(float2 uv, float depth)
{
    const float4 clip = float4(uv.x * 2.0f - 1.0f, 1.0f - uv.y * 2.0f, depth, 1.0f);
    const float4 world = mul(pushConstant.invViewProj, clip);

    return world.xyz / world.w;
}

float3 WorldAt(Texture2DMS<float> sceneDepth, int2 pixel)
{
    const float2 uv = (float2(pixel) + 0.5f) / float2(pushConstant.width, pushConstant.height);
    const float depth = sceneDepth.Load(pixel, 0);

    return ReconstructWorld(uv, max(depth, 1e-6f));
}

float InterleavedGradientNoise(float2 pixel)
{
    return frac(52.9829189f * frac(dot(pixel, float2(0.06711056f, 0.00583715f))));
}

float2 DiskSample(int index, int count, float rotation)
{
    const float radius = sqrt((float(index) + 0.5f) / float(count));
    const float angle = float(index) * GOLDEN_ANGLE + rotation;

    float2 sinCos;
    sincos(angle, sinCos.x, sinCos.y);

    return float2(sinCos.y, sinCos.x) * radius;
}

float LoadOccluder(Texture2D<float> shadowDepth, float2 uv)
{
    const int2 limit = int2(pushConstant.shadowMapSize) - 1;
    const int2 texel = clamp(int2(floor(uv * pushConstant.shadowMapSize)), int2(0, 0), limit);

    return shadowDepth.Load(int3(texel, 0));
}

float ResolveShadow(Texture2DMS<float> sceneDepth, Texture2D<float> shadowDepth, int2 pixel)
{
    const float depth = sceneDepth.Load(pixel, 0);

    if (depth <= 0.0f)
    {
        return 1.0f;
    }

    const float3 worldPos = WorldAt(sceneDepth, pixel);

    const int2 quad = min(pixel & ~1, int2(pushConstant.width, pushConstant.height) - 2);
    const float3 worldDx = WorldAt(sceneDepth, int2(quad.x + 1, pixel.y)) - WorldAt(sceneDepth, int2(quad.x, pixel.y));
    const float3 worldDy = WorldAt(sceneDepth, int2(pixel.x, quad.y + 1)) - WorldAt(sceneDepth, int2(pixel.x, quad.y));

    float3 N = normalize(cross(worldDx, worldDy));
    N = dot(N, pushConstant.cameraPos.xyz - worldPos) < 0.0f ? -N : N;

    const float texelWorld = pushConstant.shadowWorldSize / pushConstant.shadowMapSize.x;
    const float4 lightClip = mul(pushConstant.lightViewProj, float4(worldPos + N * pushConstant.normalBias * texelWorld, 1.0f));
    const float2 shadowUv = float2(lightClip.x * 0.5f + 0.5f, 0.5f - lightClip.y * 0.5f);
    const float receiver = lightClip.z;

    if (any(shadowUv < 0.0f) || any(shadowUv > 1.0f) || receiver < 0.0f || receiver > 1.0f)
    {
        return 1.0f;
    }

    const float ndl = saturate(dot(N, -pushConstant.lightDirection.xyz));
    const float slope = min(sqrt(1.0f - ndl * ndl) / max(ndl, 1e-3f), MAX_SLOPE);
    const float baseBias = pushConstant.depthBias / pushConstant.shadowDepthRange;
    const float slopeBias = slope * pushConstant.shadowWorldSize / pushConstant.shadowDepthRange;

    const float rotation = InterleavedGradientNoise(float2(pixel) + 0.5f) * TWO_PI;

    const float searchRadius = 0.5f * pushConstant.maxPenumbra / pushConstant.shadowWorldSize;

    float blockerSum = 0.0f;
    float blockerCount = 0.0f;

    for (int i = 0; i < BLOCKER_SAMPLES; i++)
    {
        const float2 offset = DiskSample(i, BLOCKER_SAMPLES, rotation) * searchRadius;
        const float occluder = LoadOccluder(shadowDepth, shadowUv + offset);

        if (occluder > receiver + baseBias + slopeBias * length(offset))
        {
            blockerSum += occluder;
            blockerCount += 1.0f;
        }
    }

    if (blockerCount == 0.0f)
    {
        return 1.0f;
    }

    const float blockerGap = (blockerSum / blockerCount - receiver) * pushConstant.shadowDepthRange;
    const float penumbra = clamp(blockerGap * pushConstant.lightSize, texelWorld * MIN_FILTER_TEXELS, pushConstant.maxPenumbra);

    const float filterRadius = 0.5f * penumbra / pushConstant.shadowWorldSize;

    float lit = 0.0f;

    for (int j = 0; j < PCF_SAMPLES; j++)
    {
        const float2 offset = DiskSample(j, PCF_SAMPLES, rotation) * filterRadius;
        const float occluder = LoadOccluder(shadowDepth, shadowUv + offset);

        lit += occluder > receiver + baseBias + slopeBias * length(offset) ? 0.0f : 1.0f;
    }

    return lit / float(PCF_SAMPLES);
}

[numthreads(8, 8, 1)]
void CSMain(uint3 dispatchId : SV_DispatchThreadID)
{
    if (dispatchId.x >= pushConstant.width || dispatchId.y >= pushConstant.height)
    {
        return;
    }

    Texture2DMS<float> sceneDepth = ResourceDescriptorHeap[pushConstant.sceneDepthIndex];
    Texture2D<float> shadowDepth = ResourceDescriptorHeap[pushConstant.shadowDepthIndex];
    RWTexture2D<float4> shadowMaskRaw = ResourceDescriptorHeap[pushConstant.outputIndex];

    const float lit = ResolveShadow(sceneDepth, shadowDepth, int2(dispatchId.xy));

    shadowMaskRaw[dispatchId.xy] = float4(lit, lit, lit, 1.0f);
}
