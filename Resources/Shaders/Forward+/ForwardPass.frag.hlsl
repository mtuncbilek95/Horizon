static const float PI = 3.14159265f;

struct ForwardConstants
{
    float4x4 worldViewProj;
    float4x4 worldMat;
    uint vertexBufferIndex;
    uint firstVertex;
    uint pointLightBufferIndex;
    uint lightFrameBufferIndex;
    uint lightGridIndex;
    uint tileSize;
    uint clusterCountX;
    uint clusterCountY;
    uint clusterCountZ;
    uint maxLightsPerCluster;
    float nearPlane;
    float farPlane;
    uint materialBufferIndex;
    uint materialIndex;
    uint samplerIndex;
    uint shadowMaskIndex;
};

struct PointLight
{
    float4 color;
    float3 position;
    float luminousIntensity;
    float range;
    uint drawDebug;
};

struct GpuLightFrame
{
    float3 sunColor;
    float illuminance;
    float3 direction;
    uint pointLightCount;
    float3 cameraPosition;
};

struct MaterialData
{
    float4 baseColorFactor;
    float4 emissiveFactor;
    float metallic;
    float roughness;
    float normalScale;
    float alphaCutoff;
    uint baseColorIndex;
    uint normalIndex;
    uint emissiveIndex;
    uint occlusionIndex;
    uint roughnessIndex;
    uint metallicIndex;
    uint occlusionChannel;
    uint roughnessChannel;
    uint metallicChannel;
    uint reserved;
};

struct PixelData
{
    float4 position : SV_Position;
    centroid float3 worldPosition : TEXCOORD0;
    centroid float3 normal : NORMAL0;
    centroid float4 color : COLOR0;
    centroid float viewDepth : TEXCOORD1;
    centroid float2 uv : TEXCOORD2;
};

ConstantBuffer<ForwardConstants> pushConstant : register(b0, space999);

uint ClusterIndex(uint2 pixel, float viewZ)
{
    float logRange = log2(pushConstant.farPlane / pushConstant.nearPlane);
    float sliceScale = pushConstant.clusterCountZ / logRange;
    float sliceBias = -(pushConstant.clusterCountZ * log2(pushConstant.nearPlane) / logRange);

    int slice = (int) floor(log2(viewZ) * sliceScale + sliceBias);
    slice = clamp(slice, 0, (int) pushConstant.clusterCountZ - 1);

    uint2 tile = pixel / pushConstant.tileSize;
    tile = min(tile, uint2(pushConstant.clusterCountX - 1, pushConstant.clusterCountY - 1));

    return tile.x + tile.y * pushConstant.clusterCountX + (uint) slice * pushConstant.clusterCountX * pushConstant.clusterCountY;
}

static const float3 kAmbient = float3(0.25f, 0.25f, 0.25f);

static const float kSpecularAAVariance = 0.15f;
static const float kSpecularAAThreshold = 0.2f;

float SpecularAntiAliasing(float3 N, float alpha)
{
    const float3 du = ddx(N);
    const float3 dv = ddy(N);
    const float variance = kSpecularAAVariance * (dot(du, du) + dot(dv, dv));
    const float kernel = min(2.0f * variance, kSpecularAAThreshold);
    return sqrt(saturate(alpha * alpha + kernel));
}

static const float kMinLightDistance = 0.01f;

float DistanceAttenuation(float distanceSq, float invRangeSq)
{
    const float factor = distanceSq * invRangeSq;
    const float window = saturate(1.0f - factor * factor);
    return window * window / max(distanceSq, kMinLightDistance * kMinLightDistance);
}

float Pow5(float x)
{
    const float x2 = x * x;
    return x2 * x2 * x;
}

float3 DiffuseHammon(float3 albedo, float NoV, float NoL, float NoH, float LoV, float alpha)
{
    const float facing = 0.5f + 0.5f * LoV;
    const float rough = facing * (0.9f - 0.4f * facing) * ((0.5f + NoH) / NoH);
    const float smooth = 1.05f * (1.0f - Pow5(1.0f - NoL)) * (1.0f - Pow5(1.0f - NoV));
    const float single = lerp(smooth, rough, alpha) / PI;
    const float multi = 0.1159f * alpha;
    return albedo * (single + albedo * multi);
}

float DistributionGGX(float NoH, float alpha)
{
    const float a2 = alpha * alpha;
    const float d = NoH * NoH * (a2 - 1.0f) + 1.0f;
    return a2 / (PI * d * d);
}

float VisibilityHammon(float NoV, float NoL, float alpha)
{
    return 0.5f / max(lerp(2.0f * NoL * NoV, NoL + NoV, alpha), 0.0001f);
}

float3 FresnelSchlick(float3 f0, float LoH)
{
    return f0 + (1.0f - f0) * Pow5(1.0f - LoH);
}

float3 EvaluateLight(float3 N, float3 V, float3 L, float3 diffuseColor, float3 f0, float alpha)
{
    const float NoL = saturate(dot(N, L));

    if (NoL <= 0.0f)
        return float3(0.0f, 0.0f, 0.0f);

    const float3 halfSum = V + L;
    const float3 H = halfSum * rsqrt(max(dot(halfSum, halfSum), 0.00000001f));

    const float NoV = max(dot(N, V), 0.0001f);
    const float NoH = max(dot(N, H), 0.5f * (NoL + NoV));
    const float LoH = saturate(dot(L, H));
    const float LoV = dot(L, V);

    const float3 diffuse = DiffuseHammon(diffuseColor, NoV, NoL, NoH, LoV, alpha);
    const float3 specular = DistributionGGX(NoH, alpha) * VisibilityHammon(NoV, NoL, alpha) * FresnelSchlick(f0, LoH);

    return (diffuse + specular) * NoL;
}

float4 PSMain(PixelData input) : SV_Target0
{
    StructuredBuffer<PointLight> pointLights = ResourceDescriptorHeap[pushConstant.pointLightBufferIndex];
    StructuredBuffer<GpuLightFrame> lightFrames = ResourceDescriptorHeap[pushConstant.lightFrameBufferIndex];
    StructuredBuffer<uint> lightGrid = ResourceDescriptorHeap[pushConstant.lightGridIndex];
    StructuredBuffer<MaterialData> materials = ResourceDescriptorHeap[pushConstant.materialBufferIndex];
    Texture2D<float4> shadowMask = ResourceDescriptorHeap[pushConstant.shadowMaskIndex];

    const GpuLightFrame lightFrame = lightFrames[0];
    const MaterialData material = materials[pushConstant.materialIndex];

    SamplerState materialSampler = SamplerDescriptorHeap[pushConstant.samplerIndex];

    float4 baseColor = material.baseColorFactor * input.color;

    if (material.baseColorIndex != 0xFFFFFFFF)
    {
        Texture2D<float4> baseColorTexture = ResourceDescriptorHeap[material.baseColorIndex];
        baseColor *= baseColorTexture.Sample(materialSampler, input.uv);
    }

    if (material.normalIndex != 0xFFFFFFFF)
    {
        Texture2D<float4> normalTexture = ResourceDescriptorHeap[material.normalIndex];
    }

    float roughness = material.roughness;

    if (material.roughnessIndex != 0xFFFFFFFF)
    {
        Texture2D<float4> roughnessTexture = ResourceDescriptorHeap[material.roughnessIndex];
        roughness *= roughnessTexture.Sample(materialSampler, input.uv)[material.roughnessChannel];
    }

    float metallic = material.metallic;

    if (material.metallicIndex != 0xFFFFFFFF)
    {
        Texture2D<float4> metallicTexture = ResourceDescriptorHeap[material.metallicIndex];
        metallic *= metallicTexture.Sample(materialSampler, input.uv)[material.metallicChannel];
    }

    roughness = clamp(roughness, 0.045f, 1.0f);
    metallic = saturate(metallic);

    const float3 shadingNormal = normalize(input.normal);
    const float alpha = SpecularAntiAliasing(shadingNormal, roughness * roughness);
    const float3 diffuseColor = baseColor.rgb * (1.0f - metallic);
    const float3 f0 = lerp(float3(0.04f, 0.04f, 0.04f), baseColor.rgb, metallic);

    const float3 view = normalize(lightFrame.cameraPosition - input.worldPosition);
    const float3 normal = dot(shadingNormal, view) < 0.0f ? -shadingNormal : shadingNormal;

    float3 color = baseColor.rgb * kAmbient * 0.15f;

    const float3 toSun = -lightFrame.direction;
    const float sunLengthSq = dot(toSun, toSun);
    const float3 sunDirection = toSun * rsqrt(max(sunLengthSq, 0.00000001f));
    const float shadow = shadowMask.Load(int3(input.position.xy, 0)).r;

    color += lightFrame.sunColor * lightFrame.illuminance * shadow * EvaluateLight(normal, view, sunDirection, diffuseColor, f0, alpha);

    const uint cluster = ClusterIndex(uint2(input.position.xy), input.viewDepth);
    const uint base = cluster * (pushConstant.maxLightsPerCluster + 1);
    const uint lightCount = min(lightGrid[base], pushConstant.maxLightsPerCluster);

    for (uint i = 0; i < lightCount; i++)
    {
        const PointLight light = pointLights[lightGrid[base + 1 + i]];

        const float3 toLight = light.position - input.worldPosition;
        const float distanceSq = dot(toLight, toLight);
        const float3 lightDirection = toLight * rsqrt(max(distanceSq, 0.00000001f));
        const float attenuation = DistanceAttenuation(distanceSq, 1.0f / max(light.range * light.range, 0.0001f));

        color += light.color.rgb * light.luminousIntensity * attenuation * EvaluateLight(normal, view, lightDirection, diffuseColor, f0, alpha);
    }
    
    float3 emissive = material.emissiveFactor.rgb;
    
    if (material.emissiveIndex != 0xFFFFFFFF)
    {
        Texture2D<float4> emissiveTexture = ResourceDescriptorHeap[material.emissiveIndex];
        emissive *= emissiveTexture.Sample(materialSampler, input.uv).rgb;
    }
    color += emissive;

    return float4(color, 1.0f);
}