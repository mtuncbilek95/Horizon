struct ShadowBlurConstants
{
    int stepX;
    int stepY;
    int radius;
    float depthSharpness;
    uint width;
    uint height;
    uint maskIndex;
    uint sceneDepthIndex;
    uint outputIndex;
};

ConstantBuffer<ShadowBlurConstants> pushConstant : register(b0, space999);

static const int MAX_RADIUS = 16;
static const float SIGMA_PER_RADIUS = 0.45f;

float BlurShadow(Texture2D<float4> shadowMask, Texture2DMS<float> sceneDepth, int2 center)
{
    const float centerDepth = sceneDepth.Load(center, 0);

    if (centerDepth <= 0.0f)
    {
        return 1.0f;
    }

    const int2 limit = int2(pushConstant.width, pushConstant.height) - 1;
    const int2 direction = int2(pushConstant.stepX, pushConstant.stepY);

    const int reach = clamp(pushConstant.radius, 0, MAX_RADIUS);
    const float sigma = max(float(reach) * SIGMA_PER_RADIUS, 1e-3f);
    const float falloff = 1.0f / (2.0f * sigma * sigma);

    float sum = shadowMask.Load(int3(center, 0)).r;
    float weightSum = 1.0f;

    for (int i = 1; i <= reach; i++)
    {
        const float gauss = exp(-float(i * i) * falloff);

        for (int side = -1; side <= 1; side += 2)
        {
            const int2 pixel = clamp(center + direction * (i * side), int2(0, 0), limit);
            const float depth = sceneDepth.Load(pixel, 0);

            const float difference = abs(depth - centerDepth) / centerDepth;
            const float weight = gauss * exp(-difference * pushConstant.depthSharpness);

            sum += shadowMask.Load(int3(pixel, 0)).r * weight;
            weightSum += weight;
        }
    }

    return sum / weightSum;
}

[numthreads(8, 8, 1)]
void CSMain(uint3 dispatchId : SV_DispatchThreadID)
{
    if (dispatchId.x >= pushConstant.width || dispatchId.y >= pushConstant.height)
    {
        return;
    }

    Texture2D<float4> shadowMask = ResourceDescriptorHeap[pushConstant.maskIndex];
    Texture2DMS<float> sceneDepth = ResourceDescriptorHeap[pushConstant.sceneDepthIndex];
    RWTexture2D<float4> blurTarget = ResourceDescriptorHeap[pushConstant.outputIndex];

    const float lit = BlurShadow(shadowMask, sceneDepth, int2(dispatchId.xy));

    blurTarget[dispatchId.xy] = float4(lit, lit, lit, 1.0f);
}
