struct PSInput
{
    float4 pos : SV_POSITION;
    float4 drawInfo : TEXCOORD0;
    float4 coordinates : TEXCOORD1;
    float4 reflectance : TEXCOORD2;
};

struct CBuffer
{
    float ghostScale;
    float2 backbufferSize;
    float intensity;
    float3 color;
};

ConstantBuffer<CBuffer> computeConstants : register(b0);

StructuredBuffer<PSInput> traceResult : register(t0);
Texture2D<float> texture : register(t1);

SamplerState imageSampler : register(s0);

float3 Uncharted2Tonemap(float3 x)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 tonemappedColor(float3 origin)
{
    float ExposureBias = 2.0f;
    float3 curr = ExposureBias * Uncharted2Tonemap(origin);
    
    const float W = 11.2;
    float3 whiteScale = 1.0f / Uncharted2Tonemap(W);
    
    return curr * whiteScale;
}

PSInput rayTraceVS(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    // (GRID_DIV  * GRID_DIV) vertices per Ghost
    PSInput vertex = traceResult[vertexID + instanceID * GRID_DIV * GRID_DIV];
    vertex.pos.xy *= float2(1.f, computeConstants.backbufferSize.x / computeConstants.backbufferSize.y) * computeConstants.ghostScale;
    return vertex;
}

float vignetting(float2 gridPos, float2 texUV)
{
    const float fade = 0.2; //make u mean it
    float gridDistance = length(gridPos);
    float gridVignet = 1 - saturate((gridDistance - 1.f) / fade + 1.f);
    gridVignet = smoothstep(0, 1, gridVignet);
    gridVignet *= lerp(0.5, 1, saturate(gridDistance));

    float texVignet = saturate(length(texUV - 0.5) * 0.5);
    texVignet = smoothstep(0, 1, texVignet);
    texVignet = lerp(0.5, 1, texVignet);
    
    return gridVignet * texVignet;
}

bool isOutUV(float2 uv)
{
    return uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1;
}

float4 rayTracePS(in PSInput input) : SV_Target
{
    float4 drawInfo = input.drawInfo;
    if (drawInfo.z > 1 || drawInfo.w == 0)
    {
        discard;
    }
    
    float2 gridPos = input.coordinates.xy;
    float2 texPos = input.coordinates.zw;
    float2 texUV = (texPos + 1.f) / 2.f;
    if (isOutUV(texUV))
        discard;
    
    float aperture = texture.Sample(imageSampler, texUV);
    if (aperture == 0.f)
        discard;
    
    float alpha = drawInfo.w * vignetting(gridPos, texUV) * computeConstants.intensity;
    if (alpha == 0.f)
        discard;

    float3 v = 0;
    #ifdef WIRE_FRAME
    v = 0.1;
    #elif UV
    v = float3(texUV, 0);
    #else
    v = tonemappedColor(alpha * input.reflectance.xyz * computeConstants.color * aperture);
    #endif
    return float4(v, 1.f);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              