struct PixelData
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

float4 PSMain(PixelData input) : SV_Target0
{
    const float3 lightDir = normalize(float3(0.3f, 0.8f, 0.5f));
    const float3 normal = normalize(input.normal);

    const float diffuse = saturate(dot(normal, lightDir));
    const float lighting = 0.25f + 0.75f * diffuse;

    return float4(input.color.rgb * lighting, input.color.a);
}