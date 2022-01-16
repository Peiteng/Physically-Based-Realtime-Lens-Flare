struct PSInput
{
    float4 pos : SV_POSITION;
    float4 drawInfo : TEXCOORD0;
    float4 coordinates[SAMPLE_LAMBDA_NUM] : WORLD;
    float4 color[SAMPLE_LAMBDA_NUM] : VIEW;
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

PSInput rayTraceVS(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    // (GRID_DIV  * GRID_DIV) vertices per Ghost
    PSInput vertex = traceResult[vertexID + instanceID * GRID_DIV * GRID_DIV];
    vertex.pos.xy *= float2(1.f, computeConstants.backbufferSize.x / computeConstants.backbufferSize.y) * computeConstants.ghostScale;
    return vertex;
}

float4 rayTracePS(in PSInput input) : SV_Target
{
    float4 drawInfo = input.drawInfo;
    if (drawInfo.z > 1 || drawInfo.w == 0)
    {
        discard;
    }
    
    float2 gridPos[SAMPLE_LAMBDA_NUM];
    float2 texPos[SAMPLE_LAMBDA_NUM];
    float2 texUV[SAMPLE_LAMBDA_NUM];
    float aperture[SAMPLE_LAMBDA_NUM];
    
    int i = 0;
    int apertureDiscardCount = 0;
    int uvDiscardCount = 0;
    float vignet = 0;
     [unroll]
    for (i = 0; i < SAMPLE_LAMBDA_NUM; i++)
    {
        gridPos[i] = input.coordinates[i].xy;
        texPos[i] = input.coordinates[i].zw;
        texUV[i] = (texPos[i] + 1.f) / 2.f;
        vignet += vignetting(gridPos[i], texUV[i]);
        
        if (isOutUV(texUV[i]))
        {
            uvDiscardCount++;
        }
        
        aperture[i] = texture.Sample(imageSampler, texUV[i]);
        if (aperture[i] == 0)
        {
            apertureDiscardCount++;
        }
    }
    
    [branch]
    if (apertureDiscardCount == SAMPLE_LAMBDA_NUM || uvDiscardCount == SAMPLE_LAMBDA_NUM)
        discard;
    
    vignet /= (float) (SAMPLE_LAMBDA_NUM);
    
    float alpha = drawInfo.w * vignet * computeConstants.intensity;
    
    [branch]
    if (alpha == 0.f)
        discard;

    float3 v = 0;
#ifdef WIRE_FRAME
    v = 0.1;
#elif UV
    v = float3(texUV[0], 0);
#else
    float3 col = 0;
    [unroll]
    for (i = 0; i < SAMPLE_LAMBDA_NUM; i++)
    {
        col += aperture[i] * input.color[i].rgb;
    }
    col /= (float) (SAMPLE_LAMBDA_NUM);
    v = tonemappedColor(alpha * computeConstants.color * col);
#endif
    return float4(v, 1.f);
}