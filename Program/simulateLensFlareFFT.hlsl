Texture2D<float> realDistributionSource : register(t0);
Texture2D<float> imaginaryDistributionSource : register(t1);

RWTexture2D<float> realDistributionDestination : register(u0);
RWTexture2D<float> imaginaryDistributionDestination : register(u1);

SamplerState imageSampler : register(s0);

void computeSrcID(uint passIndex, uint x, out uint2 indices)
{
    uint regionWidth = 2 << passIndex;
    indices.x = (x & ~(regionWidth - 1)) + (x & (regionWidth / 2 - 1));
    indices.y = indices.x + regionWidth / 2;

    if (passIndex == 0)
    {
        indices = reversebits(indices) >> (32 - BUTTERFLY_COUNT) & (LENGTH - 1);
    }
}

void computeTwiddleFactor(uint passIndex, uint x, out float2 weights)
{
    uint regionWidth = 2 << passIndex;
    sincos(2.0 * PI * float(x & (regionWidth - 1)) / float(regionWidth), weights.y, weights.x);
    weights.y *= -1;
}

#define REAL 0
#define IMAGE 1

#ifdef DOUBLE
groupshared float ButterflyArray[2][2][LENGTH];
#define SharedArray(tmpID, x, realImage) (ButterflyArray[(tmpID)][(realImage)][(x)])
#else
groupshared float ButterflyArray[2][LENGTH];
#define SharedArray(tmpID, x, realImage) (ButterflyArray[(realImage)][(x)])
#endif

void twiddleWeighting(uint passIndex, uint x, uint tmp, out float resultR, out float resultI)
{
    uint2 srcIndices;
    float2 Weights;
    
    computeSrcID(passIndex, x, srcIndices);
    
    float inputR1 = SharedArray(tmp, srcIndices.x, REAL);
    float inputI1 = SharedArray(tmp, srcIndices.x, IMAGE);

    float inputR2 = SharedArray(tmp, srcIndices.y, REAL);
    float inputI2 = SharedArray(tmp, srcIndices.y, IMAGE);
    
    computeTwiddleFactor(passIndex, x, Weights);
    
    #ifndef DOUBLE
    GroupMemoryBarrierWithGroupSync();
    #endif

#if INVERSE
	resultR = (inputR1 + Weights.x * inputR2 + Weights.y * inputI2) * 0.5;
	resultI = (inputI1 - Weights.y * inputR2 + Weights.x * inputI2) * 0.5;
#else
    resultR = inputR1 + Weights.x * inputR2 - Weights.y * inputI2;
    resultI = inputI1 + Weights.y * inputR2 + Weights.x * inputI2;
#endif
}

void prepareButterfly(uint bufferID, inout uint2 texPos)
{
    texPos = (texPos + LENGTH / 2) % LENGTH; //centering spectrum
    SharedArray(0, bufferID, REAL) = realDistributionSource[texPos];

#if ROW && !INVERSE
	SharedArray(0, bufferID, IMAGE) = 0;
#else
    SharedArray(0, bufferID, IMAGE) = imaginaryDistributionSource[texPos];
#endif
}

void executeButterfly(in uint bufferID, out float real, out float image)
{
    for (int butterFlyID = 0; butterFlyID < BUTTERFLY_COUNT - 1; ++butterFlyID)
    {
        GroupMemoryBarrierWithGroupSync();
        twiddleWeighting(butterFlyID, bufferID, butterFlyID % 2,
        SharedArray((butterFlyID + 1) % 2, bufferID, REAL),
        SharedArray((butterFlyID + 1) % 2, bufferID, IMAGE));
    }

    GroupMemoryBarrierWithGroupSync();
    twiddleWeighting(BUTTERFLY_COUNT - 1, bufferID, (BUTTERFLY_COUNT - 1) % 2, real, image);
}

[numthreads(LENGTH, 1, 1)]
void FFT1D(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const uint bufferID = dispatchThreadID.x;

#if ROW
	uint2 texPos = dispatchThreadID.xy;
#else
    uint2 texPos = dispatchThreadID.yx;
#endif

    float r = 0, i = 0;
    prepareButterfly(bufferID, texPos);
    executeButterfly(bufferID, r, i);
    
    realDistributionDestination[texPos] = r;
    imaginaryDistributionDestination[texPos] = i;
}
