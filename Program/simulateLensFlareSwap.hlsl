Texture2D<float> realDistributionSource : register(t0);
Texture2D<float> imaginaryDistributionSource : register(t1);

RWTexture2D<float> realDistributionDestination : register(u0);
RWTexture2D<float> imaginaryDistributionDestination : register(u1);

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
    
    realDistributionDestination[dispatchThreadID.xy] = realDistributionSource[ID];
    imaginaryDistributionDestination[dispatchThreadID.xy] = imaginaryDistributionSource[ID];
}
