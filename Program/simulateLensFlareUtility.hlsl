#include "complex.hlsl"

struct CBuffer
{
    float minColOfDustTex;
    float rotAngle;
    float N;
    float apertureRatio;
};

ConstantBuffer<CBuffer> computeConstants : register(b0);

Texture2D<float> realDistributionSource : register(t0);
Texture2D<float> imaginaryDistributionSource : register(t1);

RWTexture2D<float> realDistributionDestination0 : register(u0);
RWTexture2D<float> imaginaryDistributionDestination0 : register(u1);
RWTexture2D<float> realDistributionDestination1 : register(u2);
RWTexture2D<float> imaginaryDistributionDestination1 : register(u3);

SamplerState imageSampler : register(s0);

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void multiplyComplex(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ID = dispatchThreadID.xy;

    float color_real1 = realDistributionSource[ID];
    float color_image1 = imaginaryDistributionSource[ID];

    float color_real2 = realDistributionDestination0[ID];
    float color_image2 = imaginaryDistributionDestination0[ID];
    
    float2 comp1 = float2(color_real1, color_image1);
    float2 comp2 = float2(color_real2, color_image2);
    
    float2 mulcomp = complex_mul(comp1, comp2);

    realDistributionDestination1[ID] = mulcomp.x;
    imaginaryDistributionDestination1[ID] = mulcomp.y;
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void multiplyReal(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ID = dispatchThreadID.xy;
    
    float2 size;
    realDistributionDestination0.GetDimensions(size.x, size.y);
    
    float2 dstUV = ID / size;
    
    float input1 = realDistributionSource.SampleLevel(imageSampler, dstUV, 0);
    float input2 = imaginaryDistributionSource.SampleLevel(imageSampler, dstUV, 0);

    float col = input1 * input2;

    realDistributionDestination0[ID] = col;
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void whitening(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ID = dispatchThreadID.xy;
    float col = realDistributionSource[ID];

    float raiseCol = max(col, computeConstants.minColOfDustTex);

    realDistributionDestination0[ID] = raiseCol;
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void copy(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ID = dispatchThreadID.xy;

    realDistributionDestination0[ID] = realDistributionSource[ID];
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void clear(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ID = dispatchThreadID.xy;

    realDistributionDestination0[ID] = 0.0;
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void polygon(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ID = dispatchThreadID.xy;
    float2 size = float2(WIDTH, HEIGHT);
    float2 uv = ID / size - float2(0.5, 0.5);
        
    float pos = length(uv);
        
    float rad = atan2(uv.x, uv.y) + 2.0 * PI + computeConstants.rotAngle * PI / 180.0f;
    rad = rad % (2.0 * PI / computeConstants.N);
		
    float r_circ = 0.49;
        
    float r_polygon = r_circ * cos(PI / computeConstants.N) / cos(PI / computeConstants.N - rad);
    
    float r_aperture = lerp(r_polygon, r_circ, computeConstants.apertureRatio * computeConstants.apertureRatio * computeConstants.apertureRatio);
        
    float col = step(pos, r_aperture);

    realDistributionDestination0[ID] = col;
}