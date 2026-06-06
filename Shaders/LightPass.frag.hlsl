#include "Sky.hlsli"

struct LightPush
{
    uint albedoIndex;
    uint normalIndex;
    uint materialIndex;
    uint emissiveIndex;
    uint depthIndex;
    uint shadowIndex;
    uint frameBufferIndex;
    uint frameSlot;
};

struct FrameConstants
{
    float4x4 viewProj;
    float4x4 invViewProj;
    float4x4 lightViewProj;
    float4 lightDirection;
    float4 lightColor;
    float3 camPos;
    float _pad;
};

struct VertexOut
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
};

// 1 -> 3x3
// 2 -> 5x5
// 3 -> 7x7
static const int ShadowPCF = 2;
static const float PI = 3.14159265359;
static const float AmbientIntensity = 0.2;

ConstantBuffer<LightPush> pushConst : register(b0);
SamplerState samp : register(s4);
SamplerComparisonState shadowSamp : register(s5);

float3 ReconstructWorld(float2 texCoord, float depth, float4x4 inViewProj)
{
    float2 ndc = texCoord * 2.0 - 1.0;
    ndc.y = -ndc.y;
    float4 clipPos = float4(ndc, depth, 1.0);
    float4 world = mul(inViewProj, clipPos);
    
    return world.xyz / world.w;
}

float3 FresnelShlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = saturate(dot(N, H));
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

float GeometrySchlickGGX(float NdotX, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotX / (NdotX * (1.0 - k) + k);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    return GeometrySchlickGGX(saturate(dot(N, V)), roughness) *
           GeometrySchlickGGX(saturate(dot(N, L)), roughness);
}

float ShadowFactor(float3 worldPos, float4x4 lightViewProj, Texture2D<float> shadowTex)
{
    float4 lc = mul(lightViewProj, float4(worldPos, 1.0));
    float3 ndc = lc.xyz / lc.w;
    float2 suv = ndc.xy * float2(0.5, -0.5) + 0.5;

    if (any(suv < 0.0) || any(suv > 1.0))
        return 1.0;

    float current = ndc.z;
    float bias = 0.0015;

    uint width, height;
    shadowTex.GetDimensions(width, height);
    float2 texel = 1.0 / float2(width, height);

    float sum = 0.0;
    [unroll]
    for (int y = -ShadowPCF; y <= ShadowPCF; y++)
        [unroll]
        for (int x = -ShadowPCF; x <= ShadowPCF; x++)
            sum += shadowTex.SampleCmpLevelZero(shadowSamp, suv + float2(x, y) * texel, current + bias);

    return sum / 9.0;
}

float4 PSMain(VertexOut vertOut) : SV_Target0
{
    StructuredBuffer<FrameConstants> frames = ResourceDescriptorHeap[pushConst.frameBufferIndex];
    FrameConstants frame = frames[pushConst.frameSlot];
    
    Texture2D albedoTex = ResourceDescriptorHeap[pushConst.albedoIndex];
    Texture2D normalTex = ResourceDescriptorHeap[pushConst.normalIndex];
    Texture2D materialTex = ResourceDescriptorHeap[pushConst.materialIndex];
    Texture2D emissiveTex = ResourceDescriptorHeap[pushConst.emissiveIndex];
    Texture2D<float> depthTex = ResourceDescriptorHeap[pushConst.depthIndex];
    Texture2D<float> shadowTex = ResourceDescriptorHeap[pushConst.shadowIndex];
    
    int2 pixel = int2(vertOut.position.xy);
    float depth = depthTex.Load(int3(pixel, 0));
    
    if (depth == 0.0)
        return float4(0, 0, 0, 1);
    
    float3 worldPos = ReconstructWorld(vertOut.texCoord, depth, frame.invViewProj);
    float3 albedo = albedoTex.Load(int3(pixel, 0)).rgb;
    float3 N = normalize(normalTex.Load(int3(pixel, 0)).xyz);
    float3 metalRough = materialTex.Load(int3(pixel, 0)).rgb;
    float3 emissive = emissiveTex.Load(int3(pixel, 0)).rgb;

    float metallic = metalRough.r;
    float roughness = max(metalRough.g, 0.04);
    float ao = metalRough.b;
    
    float3 V = normalize(frame.camPos - worldPos);
    float3 L = normalize(-frame.lightDirection.xyz);
    float3 H = normalize(V + L);
    float NdotL = saturate(dot(N, L));
    
    float3 F0 = lerp(0.04, albedo, metallic);
    
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 F = FresnelShlick(saturate(dot(H, V)), F0);
    
    float3 specular = (D * G * F) / (4.0 * saturate(dot(N, V)) * NdotL + 0.0001);
    float3 kD = (1.0 - F) * (1.0 - metallic);
    
    float3 radiance = frame.lightColor.rgb * frame.lightColor.a;
    float shadow = ShadowFactor(worldPos, frame.lightViewProj, shadowTex);
    
    float3 Lo = (kD * albedo / PI + specular) * radiance * NdotL * shadow;
    float3 ambient = albedo * ao * SkyGradient(N) * AmbientIntensity;
    
    return float4(ambient + Lo + emissive, 1.0);
}