#include "complex.hlsl"

struct CBuffer
{
    float minColOfDustTex;
    float rotAngle;
    float N;
    float apertureRadius;
};

ConstantBuffer<CBuffer> computeConstants : register(b0);

Texture2D<float4> realDistributionSource : register(t0);
Texture2D<float4> imaginaryDistributionSource : register(t1);

RWTexture2D<float4> realDistributionDestination0 : register(u0);
RWTexture2D<float4> imaginaryDistributionDestination0 : register(u1);
RWTexture2D<float4> realDistributionDestination1 : register(u2);
RWTexture2D<float4> imaginaryDistributionDestination1 : register(u3);

SamplerState imageSampler : register(s0);

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void multiplyComplex(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ID = dispatchThreadID.xy;

    float3 color_real1 = realDistributionSource[ID].rgb;
    float3 color_image1 = imaginaryDistributionSource[ID].rgb;

    float3 color_real2 = realDistributionDestination0[ID].rgb;
    float3 color_image2 = imaginaryDistributionDestination0[ID].rgb;
    
    float2 compR1 = float2(color_real1.r, color_image1.r);
    float2 compG1 = float2(color_real1.g, color_image1.g);
    float2 compB1 = float2(color_real1.b, color_image1.b);

    float2 compR2 = float2(color_real2.r, color_image2.r);
    float2 compG2 = float2(color_real2.g, color_image2.g);
    float2 compB2 = float2(color_real2.b, color_image2.b);
    
    float2 mulcompR = complex_mul(compR1, compR2);
    float2 mulcompG = complex_mul(compG1, compG2);
    float2 mulcompB = complex_mul(compB1, compB2);
    
    float3 colorREAL = float3(mulcompR.x, mulcompG.x, mulcompB.x);
    float3 colorIMAGE = float3(mulcompR.y, mulcompG.y, mulcompB.y);

    realDistributionDestination1[ID] = float4(colorREAL, 1.0f);
    imaginaryDistributionDestination1[ID] = float4(colorIMAGE, 1.0f);
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void multiplyReal(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ID = dispatchThreadID.xy;
    float3 input1 = realDistributionSource[ID].rgb;
    float3 input2 = imaginaryDistributionSource[ID].rgb;

    float3 col = input1 * input2;

    realDistributionDestination0[ID] = float4(col, 1.0f);
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void whitening(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ID = dispatchThreadID.xy;
    float3 col = realDistributionSource[ID].rgb;

    float raiseCol = max((col.r + col.g + col.b) / 3.0f, computeConstants.minColOfDustTex);

    realDistributionDestination0[ID] = float4(raiseCol.rrr, 1.0f);
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

    realDistributionDestination0[ID] = float4(0.0, 0.0, 0.0, 1.0);
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
		
    float r_circ = 0.45;
        
    float r_polygon = cos(PI / computeConstants.N) / cos(PI / computeConstants.N - rad);
    r_polygon *= r_circ;

    float r_aperture = lerp(r_polygon, r_circ, computeConstants.apertureRadius);
        
    float col = step(pos, r_aperture);

    realDistributionDestination0[ID] = float4(col, col, col, 1.0);
}