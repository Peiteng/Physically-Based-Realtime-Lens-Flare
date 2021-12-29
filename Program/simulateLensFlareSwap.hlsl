Texture2D<float4> realDistributionSource : register(t0);
Texture2D<float4> imaginaryDistributionSource : register(t1);

RWTexture2D<float4> realDistributionDestination : register(u0);
RWTexture2D<float4> imaginaryDistributionDestination : register(u1);

SamplerState imageSampler : register(s0);

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void swap(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i, j;
    if (WIDTH / 2 <= dispatchThreadID.x && dispatchThreadID.x < WIDTH)
    {
        i = dispatchThreadID.x - WIDTH / 2;
    }
    else
    {
        i = dispatchThreadID.x + WIDTH / 2;
    }

    if (HEIGHT / 2 <= dispatchThreadID.y && dispatchThreadID.y < HEIGHT)
    {
        j = dispatchThreadID.y - HEIGHT / 2;
    }
    else
    {
        j = dispatchThreadID.y + HEIGHT / 2;
    }

    float2 ID = float2(i, j);

    float3 colorR = realDistributionSource[ID].xyz;
    realDistributionDestination[dispatchThreadID.xy] = float4(colorR, 1);
    float3 colorI = imaginaryDistributionSource[ID].xyz;
    imaginaryDistributionDestination[dispatchThreadID.xy] = float4(colorI, 1);
}
