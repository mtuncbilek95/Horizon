struct PixelData
{
    float4 position : SV_Position;
    float2 local : TEXCOORD0;
    float3 color : COLOR0;
};

float4 PSMain(PixelData input) : SV_Target0
{
    const float distance = length(input.local);
    if (distance > 1.0f)
        discard;

    const float3 color = distance > 0.75f ? float3(0.0f, 0.0f, 0.0f) : input.color;
    return float4(color, 1.0f);
}