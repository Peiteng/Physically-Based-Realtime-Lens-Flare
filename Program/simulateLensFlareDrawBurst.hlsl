struct CBuffer
{
    float3 lightDir;
    float apertureRadius;
    
    float2 backbufferSize;
    float2 padding;
    
    float3 color;
};

ConstantBuffer<CBuffer> computeConstants : register(b0);

Texture2D texture : register(t0);

SamplerState imageSampler : register(s0);

struct StarbustInput
{
    float4 pos : SV_POSITION;
    float3 uv : TEXCOORD0;
};

// Pixel Shader
// ----------------------------------------------------------------------------------
StarbustInput starburstVS(float4 vertexPos : POSITION, uint id : SV_VertexID)
{
    StarbustInput result;

    float aspect = computeConstants.backbufferSize.x / computeConstants.backbufferSize.y;
    
    result.pos = float4(vertexPos.xy * computeConstants.apertureRadius * 0.1, 0.f, 1.f);
    result.pos.xy += -computeConstants.lightDir.xy * 10;
    result.pos.xy *= float2(1.f, aspect);

    result.uv.xy = (vertexPos.xy + 1.f) * 0.5f;
    result.uv.z = 1.f - 2 * lerp(0, 1, length(computeConstants.lightDir.xy));
    
    return result;
}

float4 starburstPS(StarbustInput input) : SV_Target
{
    float2 uv = input.uv.xy;
    float intensity = input.uv.z;
    float3 starburst = texture.Sample(imageSampler, input.uv.xy).rgb * intensity * computeConstants.color;

    return float4(starburst, 1.f);
}
