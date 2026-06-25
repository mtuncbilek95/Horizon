struct ImGuiVertex
{
    float2 pos;
    float2 uv;
    uint col;
};

struct ImGuiPush
{
    float2 scale;
    float2 translate;
    uint vertexIndex;
    uint textureIndex;
};

struct VertexOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

ConstantBuffer<ImGuiPush> pushConst : register(b0);

float4 UnpackColor(uint col)
{
    return float4(((col >> 0) & 0xFF) / 255.0, ((col >> 8) & 0xFF) / 255.0,
                  ((col >> 16) & 0xFF) / 255.0, ((col >> 24) & 0xFF) / 255.0);
}

VertexOut VSMain(uint vertId : SV_VertexID)
{
    StructuredBuffer<ImGuiVertex> vertices = ResourceDescriptorHeap[pushConst.vertexIndex];
    ImGuiVertex vtx = vertices[vertId];

    VertexOut result;
    result.position = float4(vtx.pos * pushConst.scale + pushConst.translate, 0.0, 1.0);
    result.uv = vtx.uv;
    result.color = UnpackColor(vtx.col);

    return result;
}