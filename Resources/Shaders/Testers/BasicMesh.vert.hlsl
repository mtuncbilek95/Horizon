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
    float4x4 mvp;
    uint vertexBufferIndex;
    uint firstVertex;
};


struct PixelData
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

ConstantBuffer<PushConstant> pushConstant : register(b0, space0);

PixelData VSMain(uint vertexId : SV_VertexID)
{
    StructuredBuffer<MeshVertex> vertices = ResourceDescriptorHeap[pushConstant.vertexBufferIndex];
    const MeshVertex vertex = vertices[pushConstant.firstVertex + vertexId];

    const float3 viewRight = normalize(pushConstant.mvp[0].xyz);
    const float3 viewUp = normalize(pushConstant.mvp[1].xyz);
    const float3 viewBack = -normalize(pushConstant.mvp[3].xyz);

    PixelData output;
    output.position = mul(pushConstant.mvp, float4(vertex.position, 1.0f));
    output.normal = float3(dot(viewRight, vertex.normal), dot(viewUp, vertex.normal), dot(viewBack, vertex.normal));
    output.color = vertex.color;
    output.uv = vertex.uv;
    return output;
}