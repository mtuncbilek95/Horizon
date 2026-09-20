struct VertexData
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PushConstant
{
    float4x4 mvp;
};

struct PixelData
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

ConstantBuffer<PushConstant> pushConstant : register(b0, space0);

PixelData VSMain(VertexData input)
{
    const float3 viewRight = normalize(pushConstant.mvp[0].xyz);
    const float3 viewUp = normalize(pushConstant.mvp[1].xyz);
    const float3 viewBack = -normalize(pushConstant.mvp[3].xyz);

    PixelData output;
    output.position = mul(pushConstant.mvp, float4(input.position.xyz, 1.0f));
    output.normal = float3(dot(viewRight, input.normal.xyz), dot(viewUp, input.normal.xyz), dot(viewBack, input.normal.xyz));
    output.color = input.color;
    output.uv = input.uv;
    return output;
}