struct MeshVertex
{
    float3 position;
    float3 normal;
    float4 tangent;
    float4 color;
    float2 uv;
};

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

PixelData VSMain(uint vertexId : SV_VertexID)
{
    ConstantBuffer<FrameData> frameData = ResourceDescriptorHeap[pushConstant.frameDataIndex];
    StructuredBuffer<MeshVertex> vertices = ResourceDescriptorHeap[pushConstant.vertexBufferIndex];

    const MeshVertex vertex = vertices[pushConstant.firstVertex + vertexId];
    const float4 worldPos = mul(pushConstant.worldMatrix, float4(vertex.position, 1.0f));

    PixelData output;
    output.position = mul(frameData.viewProj, worldPos);
    output.worldPos = worldPos.xyz;
    output.normal = mul((float3x3)pushConstant.worldMatrix, vertex.normal);
    output.uv = vertex.uv;
    return output;
}