struct DebugLineVertex
{
    float3 position;
    uint color;
};

struct PhysicsDebugConstants
{
    float4x4 viewProj;
    uint vertexBufferIndex;
    float thickness;
    float invViewportWidth;
    float invViewportHeight;
};

struct PixelData
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

ConstantBuffer<PhysicsDebugConstants> pushConstant : register(b0, space999);

static const float kNearW = 1e-4f;

static const float2 kCorners[6] =
{
    float2(0.0f, -1.0f),
    float2(1.0f, -1.0f),
    float2(0.0f, 1.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, -1.0f),
    float2(1.0f, 1.0f)
};

float4 UnpackColor(uint color)
{
    return float4(
        (color & 0xFF) / 255.0f,
        ((color >> 8) & 0xFF) / 255.0f,
        ((color >> 16) & 0xFF) / 255.0f,
        ((color >> 24) & 0xFF) / 255.0f);
}

PixelData VSMain(uint vertexId : SV_VertexID)
{
    StructuredBuffer<DebugLineVertex> vertices = ResourceDescriptorHeap[pushConstant.vertexBufferIndex];

    const uint lineIndex = vertexId / 6;
    const float2 corner = kCorners[vertexId % 6];

    const DebugLineVertex a = vertices[lineIndex * 2];
    const DebugLineVertex b = vertices[lineIndex * 2 + 1];

    float4 clipA = mul(pushConstant.viewProj, float4(a.position, 1.0f));
    float4 clipB = mul(pushConstant.viewProj, float4(b.position, 1.0f));

    PixelData output;
    output.color = UnpackColor(corner.x == 0.0f ? a.color : b.color);

    if (clipA.w < kNearW && clipB.w < kNearW)
    {
        output.position = float4(0.0f, 0.0f, 0.0f, 0.0f);
        return output;
    }

    if (clipA.w < kNearW)
        clipA = lerp(clipA, clipB, (kNearW - clipA.w) / (clipB.w - clipA.w));

    if (clipB.w < kNearW)
        clipB = lerp(clipB, clipA, (kNearW - clipB.w) / (clipA.w - clipB.w));

    const float2 viewportSize = float2(1.0f / pushConstant.invViewportWidth, 1.0f / pushConstant.invViewportHeight);
    const float2 pixelDelta = (clipB.xy / clipB.w - clipA.xy / clipA.w) * viewportSize;
    const float pixelLength = length(pixelDelta);

    const float2 direction = pixelLength > 1e-5f ? pixelDelta / pixelLength : float2(1.0f, 0.0f);
    const float2 normal = float2(-direction.y, direction.x);
    const float2 ndcOffset = normal * pushConstant.thickness * float2(pushConstant.invViewportWidth, pushConstant.invViewportHeight);

    float4 clipPosition = corner.x == 0.0f ? clipA : clipB;
    clipPosition.xy += ndcOffset * corner.y * clipPosition.w;

    output.position = clipPosition;
    return output;
}