struct VertexOut
{
    float4 pos : SV_Position;
    float2 uv : TexCoord;
};

VertexOut VSMain(uint vertexId : SV_VertexID)
{
    VertexOut output;
    output.uv = float2((vertexId << 1) & 2, vertexId & 2);
    output.pos = float4(output.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    
    return output;
}