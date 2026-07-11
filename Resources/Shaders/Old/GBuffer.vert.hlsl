struct VertexData
{
    float3 position;
    float3 normal;
    float4 tangent;
    float2 texCoord;
};

struct FrameConstants
{
    float4x4 viewProj;
    float4x4 invViewProj;
    float4x4 lightViewProj;
    float4 lightDirection;
    float4 lightColor;
    float3 camPos;
    float _pad;
};

struct GBufferPush
{
    float4x4 model;
    uint vertexBufferIndex;
    uint materialBufferIndex;
    uint materialIndex;
    uint frameBufferIndex;
    uint frameSlot;
};

struct VertexOut
{
    float4 clipPos : SV_Position;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float4 tangent : TEXCOORD2;
    float2 texCoord : TEXCOORD3;
};

ConstantBuffer<GBufferPush> pushConst : register(b0);

VertexOut VSMain(uint vertId : SV_VertexID)
{
    StructuredBuffer<VertexData> vertices = ResourceDescriptorHeap[pushConst.vertexBufferIndex];
    StructuredBuffer<FrameConstants> frames = ResourceDescriptorHeap[pushConst.frameBufferIndex];
    
    VertexData currVertex = vertices[vertId];
    FrameConstants frame = frames[pushConst.frameSlot];
    
    float4 worldPos = mul(pushConst.model, float4(currVertex.position, 1.0));
    float3x3 normalMatrix = (float3x3) pushConst.model;
    
    VertexOut vsOut;
    vsOut.clipPos = mul(frame.viewProj, worldPos);
    vsOut.worldPos = worldPos.xyz;
    vsOut.normal = normalize(mul(normalMatrix, currVertex.normal));
    vsOut.tangent = float4(normalize(mul(normalMatrix, currVertex.tangent.xyz)), currVertex.tangent.w);
    vsOut.texCoord = currVertex.texCoord;
    
    return vsOut;
}