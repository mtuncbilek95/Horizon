struct PixelData
{
    float4 position : SV_Position;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
};

float4 PSMain(PixelData input) : SV_Target0
{
    const float3 lightDirection = normalize(float3(0.4f, 0.8f, 0.3f));
    const float shade = dot(normalize(input.normal), lightDirection) * 0.5f + 0.5f;

    return float4(input.color.rgb * shade, 1.0f);
}