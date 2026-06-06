struct GBufferPush
{
    float4x4 model;
    uint vertexBufferIndex;
    uint materialBufferIndex;
    uint materialIndex;
    uint frameBufferIndex;
    uint frameSlot;
};

struct GBufferMaterial
{
    uint albedo, normal, metalRough, ao;
    uint emissive, flags;
    float alphaCutoff;
    uint _pad0;
    float4 baseColorFactor;
    float3 emissiveFactor;
    float metallic;
    float roughness;
    float3 _pad1;
};

struct VertexOut
{
    float4 clipPos : SV_Position;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float4 tangent : TEXCOORD2;
    float2 uv : TEXCOORD3;
};

struct PSOutput
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    float4 material : SV_Target2;
    float3 emissive : SV_Target3;
};

static const uint MaterialAlphaMask = 1u << 1;
static const uint INVALID = 0xffffffff;

ConstantBuffer<GBufferPush> pushConst : register(b0);
SamplerState samp : register(s4);

PSOutput PSMain(VertexOut vertOut)
{
    StructuredBuffer<GBufferMaterial> materials = ResourceDescriptorHeap[pushConst.materialBufferIndex];
    GBufferMaterial currMaterial = materials[pushConst.materialIndex];
    
    float4 albedo = currMaterial.baseColorFactor;
    if (currMaterial.albedo != INVALID)
    {
        Texture2D tex = ResourceDescriptorHeap[currMaterial.albedo];
        albedo *= tex.Sample(samp, vertOut.uv);
    }
    
    if (currMaterial.flags & MaterialAlphaMask)
        clip(albedo.a - currMaterial.alphaCutoff);
    
    float3 N = normalize(vertOut.normal);
    if (currMaterial.normal != INVALID)
    {
        Texture2D tex = ResourceDescriptorHeap[currMaterial.normal];
        float3 n = tex.Sample(samp, vertOut.uv).xyz * 2.0 - 1.0;
        float3 T = normalize(vertOut.tangent.xyz);
        float3 B = cross(N, T) * vertOut.tangent.w;
        N = normalize(n.x * T + n.y * B + n.z * N);
    }

    float metallic = currMaterial.metallic;
    float roughness = currMaterial.roughness;
    if (currMaterial.metalRough != INVALID)
    {
        Texture2D tex = ResourceDescriptorHeap[currMaterial.metalRough];
        float4 mr = tex.Sample(samp, vertOut.uv);
        roughness *= mr.g;
        metallic *= mr.b;
    }
    roughness = clamp(roughness, 0.04, 1.0);

    float ambientOcc = 1.0;
    if (currMaterial.ao != INVALID)
    {
        Texture2D tex = ResourceDescriptorHeap[currMaterial.ao];
        ambientOcc = tex.Sample(samp, vertOut.uv).r;
    }

    float3 emissive = currMaterial.emissiveFactor;
    if (currMaterial.emissive != INVALID)
    {
        Texture2D tex = ResourceDescriptorHeap[currMaterial.emissive];
        emissive *= tex.Sample(samp, vertOut.uv).rgb;
    }
    
    PSOutput psOut;
    
    psOut.albedo = float4(albedo.rgb, 1.0);
    psOut.normal = float4(N, 0.0);
    psOut.material = float4(metallic, roughness, ambientOcc, 0.0);
    psOut.emissive = emissive.rgb;
    
    return psOut;
}