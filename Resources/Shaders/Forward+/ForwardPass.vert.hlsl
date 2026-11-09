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

struct PixelData
{
    float4 position : SV_Position;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
};

ConstantBuffer<ForwardConstants> pushConstant : register(b0, space999);

PixelData VSMain(VertexInput input)
{
    StructuredBuffer<MeshVertex> vertices = ResourceDescriptorHeap[pushConstant.vertexBufferIndex];

    const MeshVertex vertex = vertices[pushConstant.firstVertex + input.vertexId];

    PixelData output;
    output.position = mul(pushConstant.worldViewProj, float4(vertex.position, 1.0f));
    output.normal = vertex.normal;
    output.color = vertex.color;
    return output;
}