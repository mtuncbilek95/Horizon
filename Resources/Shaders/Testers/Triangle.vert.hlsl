struct VertexData
{
    float3 position;
    float4 color;
};

struct PushConstants
{
    uint bufferIndex;
    uint indexOffset;
};

struct VertexOut
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

static const uint kVertexStride = 28;
static const uint kBufferStride = 4;

ConstantBuffer<PushConstants> pushConst : register(b0);

VertexOut VSMain(uint id : SV_VertexID)
{
    ByteAddressBuffer buffer = ResourceDescriptorHeap[pushConst.bufferIndex];
    
    uint index = buffer.Load(pushConst.indexOffset + id * kBufferStride);
    uint base = index * kVertexStride;
    
    VertexOut output;
    output.position = float4(asfloat(buffer.Load3(base)), 1.0f);
    output.color = asfloat(buffer.Load4(base + 12));
    
    return output;
}