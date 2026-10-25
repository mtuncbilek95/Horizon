struct DebugVertex
{
    float3 position;
    uint color;
};

struct FrameData
{
    float4x4 view;
    float4x4 proj;
    float4x4 viewProj;
    float3 cameraPosition;
    float time;
};

struct DebugLineConstants
{
    uint frameDataIndex;
    uint vertexBufferIndex;
    uint firstVertex;
};

struct PixelData
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

ConstantBuffer<DebugLineConstants> pushConstant : register(b0, space0);

PixelData VSMain(uint vertexId : SV_VertexID)
{
    ConstantBuffer<FrameData> frameData = ResourceDescriptorHeap[pushConstant.frameDataIndex];
    StructuredBuffer<DebugVertex> vertices = ResourceDescriptorHeap[pushConstant.vertexBufferIndex];

    const DebugVertex vertex = vertices[pushConstant.firstVertex + vertexId];

    PixelData output;
    output.position = mul(frameData.viewProj, float4(vertex.position, 1.0f));
    output.color = float4(
        (vertex.color & 0xFF) / 255.0f,
        ((vertex.color >> 8) & 0xFF) / 255.0f,
        ((vertex.color >> 16) & 0xFF) / 255.0f,
        ((vertex.color >> 24) & 0xFF) / 255.0f);
    return output;
}