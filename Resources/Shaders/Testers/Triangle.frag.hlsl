struct PushConstants
{
    uint bufferIndex;
    uint cameraIndex;
    uint indexOffset;
    float deltaTime;
    double totalTimeInSec;
    uint64_t frameIndex;
};

ConstantBuffer<PushConstants> pushConst : register(b0);

struct VertexOut
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

float4 PSMain(VertexOut input) : SV_Target0
{
    return input.color;
}