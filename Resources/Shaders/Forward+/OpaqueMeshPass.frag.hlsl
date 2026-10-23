struct FrameData
{
    float4x4 view;
    float4x4 proj;
    float4x4 viewProj;
    float3 cameraPosition;
    float time;
};

struct OpaqueMeshConstants
{
    float4x4 worldMatrix;
    uint frameDataIndex;
    uint vertexBufferIndex;
    uint firstVertex;
};

struct PixelData
{
    float4 position : SV_Position;
    float3 worldPos : TEXCOORD0;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD1;
};

ConstantBuffer<OpaqueMeshConstants> pushConstant : register(b0, space0);

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

float3 LinearToGamma(float3 color)
{
    return pow(color, 1.0f / 2.2f);
}

float4 PSMain(PixelData input) : SV_Target0
{
    ConstantBuffer<FrameData> frameData = ResourceDescriptorHeap[pushConstant.frameDataIndex];

    const float3 V = normalize(frameData.cameraPosition - input.worldPos);

    float3 N = normalize(input.normal);
    if (dot(N, V) < 0.0f)
        N = -N;

    const float3 L = LIGHT_DIR;
    const float3 H = normalize(V + L);

    const float NdotL = saturate(dot(N, L));
    const float NdotV = max(dot(N, V), 1e-4f);
    const float NdotH = saturate(dot(N, H));
    const float HdotV = saturate(dot(H, V));

    const float roughness = clamp(ROUGHNESS, 0.04f, 1.0f);
    const float metallic = saturate(METALLIC);

    const float3 f0 = lerp(float3(0.04f, 0.04f, 0.04f), BASE_COLOR, metallic);
    const float3 F = FresnelSchlick(HdotV, f0);
    const float D = DistributionGGX(NdotH, roughness);
    const float G = GeometrySmith(NdotV, NdotL, roughness);

    const float3 specular = (D * G * F) / max(4.0f * NdotV * NdotL, 1e-4f);
    const float3 kd = (1.0f - F) * (1.0f - metallic);
    const float3 diffuse = kd * BASE_COLOR / PI;

    float3 color = (diffuse + specular) * LIGHT_COLOR * LIGHT_INTENSITY * NdotL;

    const float hemi = N.y * 0.5f + 0.5f;
    const float3 ambient = lerp(AMBIENT_GROUND, AMBIENT_SKY, hemi);
    color += ambient * BASE_COLOR * (1.0f - metallic * 0.5f);

    return float4(LinearToGamma(color), 1.0f);
}