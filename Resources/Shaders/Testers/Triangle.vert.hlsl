struct VertexData
{
    float3 position;
    float4 color;
};

struct PushConstants
{
    uint bufferIndex;
    uint cameraIndex;
    uint cameraOffset;
    uint indexOffset;
    float deltaTime;
};

struct VertexOut
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

static const uint kVertexStride = 28;
static const uint kBufferStride = 4;

static const uint kMatrixStride = 64;

ConstantBuffer<PushConstants> pushConst : register(b0);

float4x4 LoadMatrix(ByteAddressBuffer source, uint offset)
{
    float4 c0 = asfloat(source.Load4(offset));
    float4 c1 = asfloat(source.Load4(offset + 16));
    float4 c2 = asfloat(source.Load4(offset + 32));
    float4 c3 = asfloat(source.Load4(offset + 48));
    return float4x4(c0, c1, c2, c3);
}

VertexOut VSMain(uint id : SV_VertexID)
{
    ByteAddressBuffer buffer = ResourceDescriptorHeap[pushConst.bufferIndex];
    ByteAddressBuffer camera = ResourceDescriptorHeap[pushConst.cameraIndex];

    uint index = buffer.Load(pushConst.indexOffset + id * kBufferStride);
    uint base = index * kVertexStride;

    float4x4 mvp = LoadMatrix(camera, pushConst.cameraOffset);
    float4 localPos = float4(asfloat(buffer.Load3(base)), 1.0f);

    VertexOut output;
    output.position = mul(localPos, mvp);
    output.color = asfloat(buffer.Load4(base + 12));

    return output;
}