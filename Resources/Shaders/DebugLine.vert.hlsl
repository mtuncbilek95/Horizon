struct DebugVertex
{
    float3 position;
    uint color;
};

struct PushConstant
{
    float4x4 viewProj;
    uint vertexBufferIndex;
};

struct PixelData
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

ConstantBuffer<PushConstant> pushConstant : register(b0, space0);

PixelData VSMain(uint vertexId : SV_VertexID)
{
    StructuredBuffer<DebugVertex> vertices = ResourceDescriptorHeap[pushConstant.vertexBufferIndex];
    const DebugVertex vertex = vertices[vertexId];

    PixelData output;
    output.position = mul(pushConstant.viewProj, float4(vertex.position, 1.0f));
    output.color = float4(
        (vertex.color & 0xFF) / 255.0f,
        ((vertex.color >> 8) & 0xFF) / 255.0f,
        ((vertex.color >> 16) & 0xFF) / 255.0f,
        ((vertex.color >> 24) & 0xFF) / 255.0f);
    return output;
}