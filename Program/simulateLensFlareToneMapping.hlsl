Texture2D<float4> realDistributionSource : register(t0);
RWTexture2D<float4> realDistributionDestination : register(u0);

SamplerState imageSampler : register(s0);

float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void toneMapper(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ID = dispatchThreadID.xy;

    realDistributionDestination[ID] = float4(ACESFilm(realDistributionSource[ID].rgb), 1.0);
}