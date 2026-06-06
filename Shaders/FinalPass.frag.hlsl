struct FinalPush
{
    uint hdrIndex;
    uint bloomIndex;
    uint frameBufferIndex;
    uint frameSlot;
};

struct VertexOut
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
};

ConstantBuffer<FinalPush> pushConst : register(b0);
SamplerState linearClamp : register(s2);

static const float Exposure = 1.0;
static const float Contrast = 1.05;
static const float Saturation = 1.12;
static const float VignetteStart = 0.60;
static const float VignetteStrength = 0.35;
static const float BloomIntensity = 0.2;

static const float FXAASpanMax = 8.0;
static const float FXAAReduceMul = 1.0 / 8.0;
static const float FXAAReduceMin = 1.0 / 128.0;
static const float FXAAThreshold = 1.0 / 8.0;
static const float FXAAThresholdMin = 1.0 / 24.0;

float3 Tonemap(float3 x)
{
    return x / (x + 1.0);
}

float Luma(float3 c)
{
    return dot(c, float3(0.299, 0.587, 0.114));
}

float3 ACES(float3 x)
{
    const float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// Sample HDR to LDR
float3 SampleLDR(Texture2D hdrTex, Texture2D bloomTex, float2 uv)
{
    float3 hdr = hdrTex.Sample(linearClamp, uv).rgb;
    float3 bloom = bloomTex.Sample(linearClamp, uv).rgb;
    float3 c = (hdr + bloom * BloomIntensity) * Exposure;
    return pow(ACES(c), 1.0 / 2.2);
}

float3 Grade(float3 c)
{
    c = saturate((c - 0.5) * Contrast + 0.5);
    float luma = Luma(c);
    return lerp(float3(luma, luma, luma), c, Saturation);
}

float Vignette(float2 uv)
{
    float dist = length(uv - 0.5) * 1.41421356;
    return 1.0 - smoothstep(VignetteStart, 1.0, dist) * VignetteStrength;
}

float4 PSMain(VertexOut vertOut) : SV_Target0
{
    Texture2D hdrTex = ResourceDescriptorHeap[pushConst.hdrIndex];
    Texture2D bloomTex = ResourceDescriptorHeap[pushConst.bloomIndex];

    int2 pixel = int2(vertOut.position.xy);
    float3 hdr = hdrTex.Load(int3(pixel, 0)).rgb;

    uint width, height;
    hdrTex.GetDimensions(width, height);
    float2 reciprocal = 1.0 / float2(width, height);
    
    // RGB Directions
    float3 rgbM = SampleLDR(hdrTex, bloomTex, vertOut.texCoord);
    float3 rgbNW = SampleLDR(hdrTex, bloomTex, vertOut.texCoord + float2(-0.5, -0.5) * reciprocal);
    float3 rgbNE = SampleLDR(hdrTex, bloomTex, vertOut.texCoord + float2(0.5, -0.5) * reciprocal);
    float3 rgbSW = SampleLDR(hdrTex, bloomTex, vertOut.texCoord + float2(-0.5, 0.5) * reciprocal);
    float3 rgbSE = SampleLDR(hdrTex, bloomTex, vertOut.texCoord + float2(0.5, 0.5) * reciprocal);
    
    // Luma directions
    float lumaM = Luma(rgbM);
    float lumaNW = Luma(rgbNW);
    float lumaNE = Luma(rgbNE);
    float lumaSW = Luma(rgbSW);
    float lumaSE = Luma(rgbSE);
    
    // Luma min-max
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    
    // Early return for luma check
    float3 color;
    if (lumaMax - lumaMin < max(FXAAThresholdMin, lumaMax * FXAAThreshold))
    {
        color = rgbM;
    }
    else
    {
        float2 dir;
        dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
        dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

        float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAAReduceMul), FXAAReduceMin);
        float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
        dir = clamp(dir * rcpDirMin, -FXAASpanMax, FXAASpanMax) * reciprocal;

        float3 rgbA = 0.5 * (SampleLDR(hdrTex, bloomTex, vertOut.texCoord + dir * (1.0 / 3.0 - 0.5)) +
                             SampleLDR(hdrTex, bloomTex, vertOut.texCoord + dir * (2.0 / 3.0 - 0.5)));
        float3 rgbB = rgbA * 0.5 + 0.25 * (SampleLDR(hdrTex, bloomTex, vertOut.texCoord + dir * -0.5) +
                                           SampleLDR(hdrTex, bloomTex, vertOut.texCoord + dir * 0.5));
        float lumaB = Luma(rgbB);
        color = (lumaB < lumaMin || lumaB > lumaMax) ? rgbA : rgbB;
    }

    color = Grade(color);
    color *= Vignette(vertOut.texCoord);

    return float4(color, 1.0);
}