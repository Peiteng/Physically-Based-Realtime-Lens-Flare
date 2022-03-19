Texture2D<float> realDistributionSource : register(t0);
Texture2D<float> imaginaryDistributionSource : register(t1);

RWTexture2D<float> destDestination : register(u0);

SamplerState imageSampler : register(s0);

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void amplitude(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ID = dispatchThreadID.xy;
    float inputR = realDistributionSource[ID];
    float inputI = imaginaryDistributionSource[ID];

	float inten = inputR.r * inputR.r + inputI.r * inputI.r;

	float col = sqrt(inten);

    destDestination[ID] = col;
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void intensity(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ID = dispatchThreadID.xy;
    float inputR = realDistributionSource[ID];
    float inputI = imaginaryDistributionSource[ID];

	float inten = inputR.r * inputR.r + inputI.r * inputI.r;

    destDestination[ID] = inten;
}