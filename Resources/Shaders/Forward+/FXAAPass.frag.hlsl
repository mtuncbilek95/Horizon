struct FXAAConstants
{
    float2 rcpFrame;
    uint textureIndex;
    uint samplerIndex;
};

struct PixelData
{
    float4 position : SV_Position;
    float2 uv : TexCoord;
};

ConstantBuffer<FXAAConstants> pushConstant : register(b0, space999);

static const float edgeThreshold = 1.0f / 8.0f;
static const float edgeThresholdMin = 1.0f / 24.0f;
static const uint searchSteps = 32;
static const float searchThreshold = 1.0f / 4.0f;
static const float subpixCap = 3.0f / 4.0f;
static const float subpixTrim = 1.0f / 4.0f;
static const float subpixTrimScale = 1.0f / (1.0f - subpixTrim);

float Luma(float3 rgb) { return rgb.y * (0.587f / 0.299f) + rgb.x; }

float4 PSMain(PixelData input) : SV_Target0
{
    Texture2D<float4> sourceTexture = ResourceDescriptorHeap[pushConstant.textureIndex];
    SamplerState sourceSampler = SamplerDescriptorHeap[pushConstant.samplerIndex];

    const float2 uv = input.uv;
    const float2 rcpFrame = pushConstant.rcpFrame;

    const float3 rgbN = sourceTexture.SampleLevel(sourceSampler, uv, 0.0f, int2(0, -1)).rgb;
    const float3 rgbW = sourceTexture.SampleLevel(sourceSampler, uv, 0.0f, int2(-1, 0)).rgb;
    const float3 rgbM = sourceTexture.SampleLevel(sourceSampler, uv, 0.0f).rgb;
    const float3 rgbE = sourceTexture.SampleLevel(sourceSampler, uv, 0.0f, int2(1, 0)).rgb;
    const float3 rgbS = sourceTexture.SampleLevel(sourceSampler, uv, 0.0f, int2(0, 1)).rgb;

    float lumaN = Luma(rgbN);
    const float lumaW = Luma(rgbW);
    const float lumaM = Luma(rgbM);
    const float lumaE = Luma(rgbE);
    float lumaS = Luma(rgbS);

    const float rangeMin = min(lumaM, min(min(lumaN, lumaW), min(lumaS, lumaE)));
    const float rangeMax = max(lumaM, max(max(lumaN, lumaW), max(lumaS, lumaE)));
    const float range = rangeMax - rangeMin;

    if (range < max(edgeThresholdMin, rangeMax * edgeThreshold))
        return float4(rgbM, 1.0f);

    const float lumaL = (lumaN + lumaW + lumaE + lumaS) * 0.25f;
    const float rangeL = abs(lumaL - lumaM);
    const float blendL = min(subpixCap, max(0.0f, (rangeL / range) - subpixTrim) * subpixTrimScale);

    const float3 rgbNW = sourceTexture.SampleLevel(sourceSampler, uv, 0.0f, int2(-1, -1)).rgb;
    const float3 rgbNE = sourceTexture.SampleLevel(sourceSampler, uv, 0.0f, int2(1, -1)).rgb;
    const float3 rgbSW = sourceTexture.SampleLevel(sourceSampler, uv, 0.0f, int2(-1, 1)).rgb;
    const float3 rgbSE = sourceTexture.SampleLevel(sourceSampler, uv, 0.0f, int2(1, 1)).rgb;

    const float3 rgbL = (rgbN + rgbW + rgbM + rgbE + rgbS + rgbNW + rgbNE + rgbSW + rgbSE) / 9.0f;

    const float lumaNW = Luma(rgbNW);
    const float lumaNE = Luma(rgbNE);
    const float lumaSW = Luma(rgbSW);
    const float lumaSE = Luma(rgbSE);

    const float edgeVert =
        abs((0.25f * lumaNW) + (-0.5f * lumaN) + (0.25f * lumaNE)) +
        abs((0.50f * lumaW) + (-1.0f * lumaM) + (0.50f * lumaE)) +
        abs((0.25f * lumaSW) + (-0.5f * lumaS) + (0.25f * lumaSE));
    const float edgeHorz =
        abs((0.25f * lumaNW) + (-0.5f * lumaW) + (0.25f * lumaSW)) +
        abs((0.50f * lumaN) + (-1.0f * lumaM) + (0.50f * lumaS)) +
        abs((0.25f * lumaNE) + (-0.5f * lumaE) + (0.25f * lumaSE));

    const bool horzSpan = edgeHorz >= edgeVert;
    float lengthSign = horzSpan ? -rcpFrame.y : -rcpFrame.x;

    if (!horzSpan)
    {
        lumaN = lumaW;
        lumaS = lumaE;
    }

    float gradientN = abs(lumaN - lumaM);
    const float gradientS = abs(lumaS - lumaM);
    lumaN = (lumaN + lumaM) * 0.5f;
    lumaS = (lumaS + lumaM) * 0.5f;

    if (gradientN < gradientS)
    {
        lumaN = lumaS;
        gradientN = gradientS;
        lengthSign *= -1.0f;
    }

    const float2 startPos = uv + (horzSpan ? float2(0.0f, lengthSign * 0.5f) : float2(lengthSign * 0.5f, 0.0f));

    gradientN *= searchThreshold;

    const float2 offNP = horzSpan ? float2(rcpFrame.x, 0.0f) : float2(0.0f, rcpFrame.y);
    float2 posN = startPos - offNP;
    float2 posP = startPos + offNP;
    float lumaEndN = lumaN;
    float lumaEndP = lumaN;
    bool doneN = false;
    bool doneP = false;

    for (uint i = 0; i < searchSteps; i++)
    {
        if (!doneN)
            lumaEndN = Luma(sourceTexture.SampleLevel(sourceSampler, posN, 0.0f).rgb);

        if (!doneP)
            lumaEndP = Luma(sourceTexture.SampleLevel(sourceSampler, posP, 0.0f).rgb);

        doneN = doneN || (abs(lumaEndN - lumaN) >= gradientN);
        doneP = doneP || (abs(lumaEndP - lumaN) >= gradientN);

        if (doneN && doneP)
            break;

        if (!doneN)
            posN -= offNP;

        if (!doneP)
            posP += offNP;
    }

    const float dstN = horzSpan ? uv.x - posN.x : uv.y - posN.y;
    const float dstP = horzSpan ? posP.x - uv.x : posP.y - uv.y;
    const bool directionN = dstN < dstP;
    const float lumaEnd = directionN ? lumaEndN : lumaEndP;

    if (((lumaM - lumaN) < 0.0f) == ((lumaEnd - lumaN) < 0.0f))
        lengthSign = 0.0f;

    const float spanLength = dstP + dstN;
    const float dstNearest = directionN ? dstN : dstP;
    const float subPixelOffset = (0.5f - dstNearest / spanLength) * lengthSign;
    const float2 finalUv = uv + (horzSpan ? float2(0.0f, subPixelOffset) : float2(subPixelOffset, 0.0f));
    const float3 rgbF = sourceTexture.SampleLevel(sourceSampler, finalUv, 0.0f).rgb;

    return float4(lerp(rgbF, rgbL, blendL), 1.0f);
}