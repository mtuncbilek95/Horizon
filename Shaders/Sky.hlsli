#ifndef SKY_HLSLI
#define SKY_HLSLI

static const float3 SkyZenith = float3(0.18, 0.38, 0.72);
static const float3 SkyHorizon = float3(0.62, 0.74, 0.90);
static const float3 SkyGround = float3(0.28, 0.26, 0.24);\

float3 SkyGradient(float3 dir)
{
    float up = dir.y;
    return (up >= 0.0)
        ? lerp(SkyHorizon, SkyZenith, pow(saturate(up), 0.5))
        : lerp(SkyHorizon, SkyGround, saturate(-up * 4.0));
}

float3 SkyWithSun(float3 dir, float3 sunDir)
{
    float3 sky = SkyGradient(dir);
    float sun = saturate(dot(dir, sunDir));
    float3 sunCol = float3(1.0, 0.92, 0.75);
    sky += sunCol * pow(sun, 3000.0) * 40.0;
    sky += sunCol * pow(sun, 8.0) * 0.25;
    return sky;
}

#endif 