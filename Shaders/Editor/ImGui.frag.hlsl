struct ImGuiPush 
{ 
    float2 scale; 
    float2 translate; 
    uint vertexIndex; 
    uint textureIndex; 
};

struct VertexOut 
{ 
    float4 position : SV_Position; 
    float2 uv : TEXCOORD0; 
    float4 color : COLOR0; 
};

ConstantBuffer<ImGuiPush> pushConst : register(b0);
SamplerState linearClamp : register(s3);

float4 PSMain(VertexOut input) : SV_Target0
{
    Texture2D tex = ResourceDescriptorHeap[pushConst.textureIndex];
    return input.color * tex.Sample(linearClamp, input.uv);
}