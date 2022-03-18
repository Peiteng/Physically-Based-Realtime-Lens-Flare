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

StarbustInput starburstVS(uint id : SV_VertexID)
{
    StarbustInput result;

    float aspect = computeConstants.backbufferSize.x / computeConstants.backbufferSize.y;
    
    const float2 vertex = float2(id / 2u, id % 2u);
    StarbustInput Out = (StarbustInput) 0;
    float2 screenPos = float2(4.0f, -4.0f) * vertex.xy + float2(-1.0f, 1.0f);
    Out.pos = float4(screenPos, 0.0f, 1.0f);
    Out.uv.xy = float2(2.0f, 2.0f) * vertex;
    
    Out.pos.xy += -computeConstants.lightDir.xy * 10;
    Out.pos.xy *= computeConstants.apertureRadius * 0.1;
    Out.pos.xy *= float2(1.f, aspect);
    Out.uv.z = 1.f - 2 * lerp(0, 1, length(computeConstants.lightDir.xy));
    
    return Out;
}

float4 starburstPS(StarbustInput input) : SV_Target
{
    float2 uv = input.uv.xy;
    float intensity = input.uv.z;
    float3 starburst = texture.Sample(imageSampler, input.uv.xy).rgb * intensity * computeConstants.color;

    return float4(starburst, 1.f);
}
