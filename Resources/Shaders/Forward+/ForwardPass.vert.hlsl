struct MeshVertex
{
    float3 position;
    float3 normal;
    float4 tangent;
    float4 color;
    float2 uv;
};

struct DrawData
{
    float4x4 world;
    uint firstVertex;
};

struct ForwardConstants
{
    float4x4 viewProj;
    uint drawBufferIndex;
    uint vertexBufferIndex;
};

struct VertexInput
{
    uint vertexId : SV_VertexID;
    uint drawId : DRAW_ID;
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
    StructuredBuffer<DrawData> draws = ResourceDescriptorHeap[pushConstant.drawBufferIndex];
    StructuredBuffer<MeshVertex> vertices = ResourceDescriptorHeap[pushConstant.vertexBufferIndex];

    const DrawData draw = draws[input.drawId];
    const MeshVertex vertex = vertices[draw.firstVertex + input.vertexId];

    const float4 worldPosition = mul(draw.world, float4(vertex.position, 1.0f));

    PixelData output;
    output.position = mul(pushConstant.viewProj, worldPosition);
    output.normal = mul((float3x3)draw.world, vertex.normal);
    output.color = vertex.color;
    return output;
}