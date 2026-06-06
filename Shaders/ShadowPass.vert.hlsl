struct VertexData
{
    float3 position;
    float3 normal;
    float4 tangent;
    float2 texCoord;
};

struct FrameConstants
{
    float4x4 viewProj;
    float4x4 invViewProj;
    float4x4 lightViewProj;
    float4 lightDirection;
    float4 lightColor;
    float3 camPos;
    float _pad;
};

struct ShadowPush
{
    float4x4 model;
    uint vertexBufferIndex;
    uint frameBufferIndex;
    uint frameSlot;
};

ConstantBuffer<ShadowPush> pushConst : register(b0);

float4 VSMain(uint vertId : SV_VertexID) : SV_Position
{
    StructuredBuffer<VertexData> vertices = ResourceDescriptorHeap[pushConst.vertexBufferIndex];
    StructuredBuffer<FrameConstants> frames = ResourceDescriptorHeap[pushConst.frameBufferIndex];
    
    VertexData currVertex = vertices[vertId];
    FrameConstants frame = frames[pushConst.frameSlot];
    
    float4 worldPos = mul(pushConst.model, float4(currVertex.position, 1.0));
    return mul(frame.lightViewProj, worldPos);
}