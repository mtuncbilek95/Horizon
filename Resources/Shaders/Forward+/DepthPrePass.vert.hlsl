struct MeshVertex
{
    float3 position;
    float3 normal;
    float4 tangent;
    float4 color;
    float2 uv;
};

struct ForwardConstants
{
    float4x4 worldViewProj;
    uint vertexBufferIndex;
    uint firstVertex;
};

struct VertexInput
{
    uint vertexId : SV_VertexID;
};

ConstantBuffer<ForwardConstants> pushConstant : register(b0, space999);

float4 VSMain(VertexInput input) : SV_Position
{
    StructuredBuffer<MeshVertex> vertices = ResourceDescriptorHeap[pushConstant.vertexBufferIndex];

    const MeshVertex vertex = vertices[pushConstant.firstVertex + input.vertexId];
    const float4 localPosition = float4(vertex.position, 1.0f);

    precise float4 clipPosition = mul(pushConstant.worldViewProj, localPosition);
    return clipPosition;
}