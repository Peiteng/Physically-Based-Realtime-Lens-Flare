Texture2D<float4> src : register(t0);

RWTexture2D<float> dst : register(u0);

SamplerState imageSampler : register(s0);

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void oneElem(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ID = dispatchThreadID.xy;
    
    float2 dstTexSize = float2(WIDTH, HEIGHT);
    float2 uv = ID / dstTexSize;
    
    float4 color = src.SampleLevel(imageSampler, uv, 0);

    dst[ID] = color.r;
}