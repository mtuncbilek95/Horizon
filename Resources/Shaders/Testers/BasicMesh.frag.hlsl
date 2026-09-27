struct PushConstant
{
    float4x4 model;
    float4x4 viewProj;
    float3 cameraPos;
    uint vertexBufferIndex;
    uint firstVertex;
    float fogDensity;
    float fogStartDistance;
    float fogEndDistance;
    float3 fogColor;
    float fogMaxOpacity;
    uint fogMode;
};

struct PixelData
{
    float4 position : SV_Position;
    float3 worldPos : TEXCOORD0;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD1;
};

enum FogMode : uint
{
    None = 0,
    Linear = 1,
    Exponential = 2
};

ConstantBuffer<PushConstant> pushConstant : register(b0, space0);

static const float PI = 3.14159265f;

static const float3 LIGHT_DIR = normalize(float3(0.3f, 0.8f, 0.5f));
static const float LIGHT_INTENSITY = 3.0f;
static const float3 LIGHT_COLOR = float3(1.0f, 0.95f, 0.85f);
static const float3 AMBIENT_SKY = float3(0.35f, 0.45f, 0.6f);
static const float3 AMBIENT_GROUND = float3(0.15f, 0.12f, 0.10f);

static const float3 BASE_COLOR = float3(0.7f, 0.7f, 0.7f);
static const float ROUGHNESS = 0.5f;
static const float METALLIC = 0.0f;

float DistributionGGX(float NdotH, float roughness)
{
    const float a = roughness * roughness;
    const float a2 = a * a;
    const float d = NdotH * NdotH * (a2 - 1.0f) + 1.0f;
    return a2 / max(PI * d * d, 1e-5f);
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    const float r = roughness + 1.0f;
    const float k = (r * r) / 8.0f;
    const float gv = NdotV / (NdotV * (1.0f - k) + k);
    const float gl = NdotL / (NdotL * (1.0f - k) + k);
    return gv * gl;
}

float3 FresnelSchlick(float cosTheta, float3 f0)
{
    return f0 + (1.0f - f0) * pow(1.0f - cosTheta, 5.0f);
}

float4 PSMain(PixelData input) : SV_Target0
{
    const float3 N = normalize(input.normal);
    const float3 V = normalize(pushConstant.cameraPos.xyz - input.worldPos);
    const float3 L = LIGHT_DIR;
    const float3 H = normalize(V + L);

    const float NdotL = saturate(dot(N, L));
    const float NdotV = max(dot(N, V), 1e-4f);
    const float NdotH = saturate(dot(N, H));
    const float HdotV = saturate(dot(H, V));

    float3 baseColor = BASE_COLOR;
    const float roughness = clamp(ROUGHNESS, 0.04f, 1.0f);
    const float metallic = saturate(METALLIC);

    const float3 f0 = lerp(float3(0.04f, 0.04f, 0.04f), baseColor, metallic);
    const float3 F = FresnelSchlick(HdotV, f0);
    const float D = DistributionGGX(NdotH, roughness);
    const float G = GeometrySmith(NdotV, NdotL, roughness);

    const float3 specular = (D * G * F) / max(4.0f * NdotV * NdotL, 1e-4f);
    const float3 kd = (1.0f - F) * (1.0f - metallic);
    const float3 diffuse = kd * baseColor / PI;

    const float3 radiance = LIGHT_COLOR * LIGHT_INTENSITY;
    float3 color = (diffuse + specular) * radiance * NdotL;

    const float hemi = N.y * 0.5f + 0.5f;
    const float3 ambient = lerp(AMBIENT_GROUND, AMBIENT_SKY, hemi);
    color += ambient * baseColor * (1.0f - metallic * 0.5f);

    const float rim = pow(1.0f - NdotV, 3.0f);
    color += rim * ambient * 0.3f;

    const float dist = distance(pushConstant.cameraPos.xyz, input.worldPos);
    const float fogDist = max(0.0f, dist - pushConstant.fogStartDistance);

    float fog = 0.0f;

    if (pushConstant.fogMode == FogMode::Linear)
    {
        fog = saturate(fogDist / max(pushConstant.fogEndDistance - pushConstant.fogStartDistance, 1e-4f));
    }
    
    if (pushConstant.fogMode == FogMode::Exponential)
    {
        fog = 1.0f - exp(-pushConstant.fogDensity * fogDist);
    }
    
    fog = min(fog, pushConstant.fogMaxOpacity);
    color = lerp(color, pushConstant.fogColor, fog);
    
    color = pow(color, 1.0f / 2.2f);
    return float4(color, 1.0f);
}