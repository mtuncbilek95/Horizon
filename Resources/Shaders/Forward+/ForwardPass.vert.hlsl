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
    float4x4 worldMat;
    uint vertexBufferIndex;
    uint firstVertex;
    uint pointLightBufferIndex;
    uint lightFrameBufferIndex;
};

struct VertexInput
{
    uint vertexId : SV_VertexID;
};

struct PixelData
{
    float4 position : SV_Position;
    centroid float3 worldPosition : TEXCOORD0;
    centroid float3 normal : NORMAL0;
    centroid float4 color : COLOR0;
};

ConstantBuffer<ForwardConstants> pushConstant : register(b0, space999);

PixelData VSMain(VertexInput input)
{
    StructuredBuffer<MeshVertex> vertices = ResourceDescriptorHeap[pushConstant.vertexBufferIndex];

    const MeshVertex vertex = vertices[pushConstant.firstVertex + input.vertexId];
    const float4 localPosition = float4(vertex.position, 1.0f);

    precise float4 clipPosition = mul(pushConstant.worldViewProj, localPosition);

    PixelData output;
    output.position = clipPosition;
    output.worldPosition = mul(pushConstant.worldMat, localPosition).xyz;
    output.normal = normalize(mul((float3x3) pushConstant.worldMat, vertex.normal));
    output.color = vertex.color;
    return output;
}