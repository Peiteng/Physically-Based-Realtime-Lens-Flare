Texture2D<float4> realDistributionSource : register(t0);
Texture2D<float4> imaginaryDistributionSource : register(t1);

RWTexture2D<float4> realDistributionDestination : register(u0);
RWTexture2D<float4> imaginaryDistributionDestination : register(u1);

SamplerState imageSampler : register(s0);

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void amplitude(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ID = dispatchThreadID.xy;
    float3 inputR = realDistributionSource[ID].rgb;
    float3 inputI = imaginaryDistributionSource[ID].rgb;

	float r = inputR.r * inputR.r + inputI.r * inputI.r;
	float g = inputR.g * inputR.g + inputI.g * inputI.g;
	float b = inputR.b * inputR.b + inputI.b * inputI.b;

	float3 col = float3(sqrt(r), sqrt(g), sqrt(b));

    realDistributionDestination[ID] = float4(col, 1.0f);
    imaginaryDistributionDestination[ID] = float4(col, 1.0f);
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void intensity(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ID = dispatchThreadID.xy;
    float3 inputR = realDistributionSource[ID].rgb;
    float3 inputI = imaginaryDistributionSource[ID].rgb;

	float r = inputR.r * inputR.r + inputI.r * inputI.r;
	float g = inputR.g * inputR.g + inputI.g * inputI.g;
	float b = inputR.b * inputR.b + inputI.b * inputI.b;

	float3 col = float3(r, g, b);

    realDistributionDestination[ID] = float4(col, 1.0f);
    imaginaryDistributionDestination[ID] = float4(col, 1.0f);
}