struct VertexOut
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
};

VertexOut VSMain(uint id : SV_VertexID)
{
    VertexOut vertOut;
    vertOut.texCoord = float2((id << 1) & 2, id & 2);
    vertOut.position = float4(vertOut.texCoord * float2(2, -2) + float2(-1, 1), 0, 1);
    
    return vertOut;
}