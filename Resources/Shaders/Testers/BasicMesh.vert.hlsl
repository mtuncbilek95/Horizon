struct MeshVertex
{
    float3 position;
    float3 normal;
    float4 tangent;
    float4 color;
    float2 uv;
};

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

ConstantBuffer<PushConstant> pushConstant : register(b0, space0);

PixelData VSMain(uint vertexId : SV_VertexID)
{
    StructuredBuffer<MeshVertex> vertices = ResourceDescriptorHeap[pushConstant.vertexBufferIndex];
    const MeshVertex vertex = vertices[pushConstant.firstVertex + vertexId];

    PixelData output;
    
    const float4 worldPos = mul(pushConstant.model, float4(vertex.position, 1.0f));
    
    output.worldPos = worldPos.xyz;
    output.position = mul(pushConstant.viewProj, worldPos);
    output.normal = mul((float3x3) pushConstant.model, vertex.normal);
    output.uv = vertex.uv;
    return output;
}