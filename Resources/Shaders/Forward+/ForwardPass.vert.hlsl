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
    float4 worldRow0;
    float4 worldRow1;
    float4 worldRow2;
    uint vertexBufferIndex;
    uint firstVertex;
    uint pointLightBufferIndex;
    uint pointLightCount;
};

struct VertexInput
{
    uint vertexId : SV_VertexID;
};

struct PixelData
{
    float4 position : SV_Position;
    float3 worldPosition : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
};

ConstantBuffer<ForwardConstants> pushConstant : register(b0, space999);

PixelData VSMain(VertexInput input)
{
    StructuredBuffer<MeshVertex> vertices = ResourceDescriptorHeap[pushConstant.vertexBufferIndex];

    const MeshVertex vertex = vertices[pushConstant.firstVertex + input.vertexId];
    const float4 localPosition = float4(vertex.position, 1.0f);

    PixelData output;
    output.position = mul(pushConstant.worldViewProj, localPosition);
    output.worldPosition = float3(dot(pushConstant.worldRow0, localPosition), dot(pushConstant.worldRow1, localPosition), dot(pushConstant.worldRow2, localPosition));
    output.normal = float3(dot(pushConstant.worldRow0.xyz, vertex.normal), dot(pushConstant.worldRow1.xyz, vertex.normal), dot(pushConstant.worldRow2.xyz, vertex.normal));
    output.color = vertex.color;
    return output;
}