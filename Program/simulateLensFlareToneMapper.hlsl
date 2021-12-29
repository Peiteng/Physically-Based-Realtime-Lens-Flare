struct VSInput
{
    float4 Position : POSITION;
    float4 UV : TEXCOORD0;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float4 UV : TEXCOORD0;
};

struct SceneParameters
{
    float4x4 proj;
};

ConstantBuffer<SceneParameters> sceneConstants : register(b0);
Texture2D texture : register(t0);

SamplerState imageSampler : register(s0);

PSInput VS(VSInput In, uint instanceID : SV_InstanceID)
{
    PSInput result = (PSInput) 0;

    result.Position = mul(In.Position, sceneConstants.proj);
    result.UV = In.UV;
    return result;
}

float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float4 PS(PSInput In) : SV_TARGET
{
    float2 uv = In.UV.xy;
    
    float4 renderOutput = texture.Sample(imageSampler, uv);
    return renderOutput = float4(ACESFilm(renderOutput.rgb), 1);
}
