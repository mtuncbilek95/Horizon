struct PixelData
{
    float4 position : SV_Position;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
};

float4 PSMain(PixelData input) : SV_Target0
{
    const float3 lightDirection = normalize(float3(0.4f, 0.8f, 0.3f));
    const float diffuse = saturate(dot(normalize(input.normal), lightDirection));

    return float4(input.color.rgb * (0.15f + 0.85f * diffuse), 1.0f);
}