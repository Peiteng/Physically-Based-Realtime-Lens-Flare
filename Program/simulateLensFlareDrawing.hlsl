struct PSInput
{
    float4 pos : SV_POSITION;
    float4 drawInfo : TEXCOORD0;
    float4 coordinates[SAMPLE_LAMBDA_NUM] : RAYPOS;
    float4 color[SAMPLE_LAMBDA_NUM] : RAYCOLOR;
};

struct CBuffer
{
    float ghostScale;
    float3 color;
    
    float aspect;
    float intensity;
    float2 padding;
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
    vertex.pos.xy *= float2(1.f, computeConstants.aspect) * computeConstants.ghostScale;
    
    return vertex;
}

float4 rayTracePS(in PSInput input) : SV_Target
{
    float4 drawInfo = input.drawInfo;
    
      [branch]
    if (drawInfo.z > 1 || drawInfo.w == 0)
    {
        //return float4(1,0,0,1);
        discard;
    }
    //return float4(0, 1, 0, 1);
#ifdef WIRE_FRAME
    return 1;
#endif
    
#ifdef UV
    return float4((input.coordinates[0].zw + 1.f) * 0.5f, 0, 1);
#endif
    int lambda = 0;
    int apertureDiscardCount = 0;
    int uvDiscardCount = 0;
    float3 col = 0;
     [unroll]
    for (lambda = 0; lambda < SAMPLE_LAMBDA_NUM; lambda++)
    {
        float2 texUV = (input.coordinates[lambda].zw + 1.f) * 0.5f;
        [branch]
        if (isOutUV(texUV))
        {
            uvDiscardCount++;
        }
        
        float aperture = texture.Sample(imageSampler, texUV);
        col += aperture * input.color[lambda].rgb;
        [branch]
        if (aperture < 1e-3)
        {
            apertureDiscardCount++;
        }
    }
    
    [branch]
    if (apertureDiscardCount == SAMPLE_LAMBDA_NUM || uvDiscardCount == SAMPLE_LAMBDA_NUM)
        discard;
    
    return float4(tonemappedColor(drawInfo.w * vignetting(input.coordinates[0].xy, input.coordinates[0].zw) * computeConstants.intensity * computeConstants.color * col / (float) (SAMPLE_LAMBDA_NUM)), 1);
}