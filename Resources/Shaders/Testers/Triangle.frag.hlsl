struct VertexOut
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

float4 PSMain(VertexOut input) : SV_Target0
{
    return input.color;
}