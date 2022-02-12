#include "simulateLensFlareGlobalFunction.hlsl"

struct CBuffer
{
    float glareIntensity;
    float glareLambdaSamplenum;
    float2 padding;
};

ConstantBuffer<CBuffer> computeConstants : register(b0);

Texture2D<float4> realDistributionSource : register(t0);
Texture2D<float4> imaginaryDistributionSource : register(t1);

RWTexture2D<float4> realDistributionDestination : register(u0);
RWTexture2D<float4> imaginaryDistributionDestination : register(u1);

SamplerState imageSampler : register(s0);

float3 srqtPerElem(float3 inputR, float3 inputI)
{
    return float3(sqrt(inputR.r * inputR.r + inputI.r * inputI.r)
		, sqrt(inputR.g * inputR.g + inputI.g * inputI.g)
		, sqrt(inputR.b * inputR.b + inputI.b * inputI.b));
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void raiseValue(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ID = dispatchThreadID.xy;
    float3 inputR = realDistributionSource[ID].rgb;
    float3 inputI = imaginaryDistributionSource[ID].rgb;

    float3 amplitude = srqtPerElem(inputR, inputI);

    float val = (amplitude.r + amplitude.g + amplitude.b) / 3.0f;
    
    inputR = inputR * computeConstants.glareIntensity;
    inputI = inputI * computeConstants.glareIntensity;
    
    realDistributionDestination[ID] = float4(inputR, 1.0);
    imaginaryDistributionDestination[ID] = float4(inputI, 1.0);
}

bool Clamped(float2 floatIndex)
{
    return floatIndex.x < -0 || floatIndex.x > 1 || floatIndex.y < -0 || floatIndex.y > 1;
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void lambdaIntegral(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const float starburst_resolution = 1.0f;
    float2 ID = dispatchThreadID.xy;
    float2 size = float2(WIDTH, HEIGHT);
    float2 pos = ID / size;
    float2 uv = pos.xy / starburst_resolution - 0.5;
    float d = length(uv) * 2;

	// -ve violet, +v reds
    const float scale = 0.50f;

    float3 result = 0.f;
    int num_steps = computeConstants.glareLambdaSamplenum * 3;
    num_steps = 20 * 3;
    for (int i = 0; i <= num_steps; ++i)
    {
        float n = (float) i / (float) num_steps;

        float2 scaled_uv = uv * lerp(1.f + scale, 1.f, n) + 0.5;

        bool clamped = Clamped(scaled_uv);
        float lambda = lerp(LAMBDA_NM_BLUE, LAMBDA_NM_RED, n);

        float cr = 1 / lambda;
        cr *= cr;
        
        result += (cr * realDistributionSource.SampleLevel(imageSampler, scaled_uv, 0).r * !clamped * lerp(1.f, lambda2RGB(lambda), 0.75f));
    }

    result /= (float) num_steps;

    realDistributionDestination[ID] = float4(result, 1.0);
    imaginaryDistributionDestination[ID] = float4(result, 1.0);
}

float2 Rotate(float2 p, float a)
{
    float x = p.x;
    float y = p.y;

    float cosa = cos(a);
    float sina = sin(a);

    float x1 = x * cosa - y * sina;
    float y1 = y * cosa + x * sina;

    return float2(x1, y1);
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void burstFilter(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const float starburst_resolution = 1.0f;
    float2 ID = dispatchThreadID.xy;
    float2 size = float2(WIDTH, HEIGHT);
    float2 pos = ID / size;
    float2 uv = pos.xy / starburst_resolution - 0.5;

    float3 result = 0.f;

    int num_steps = computeConstants.glareLambdaSamplenum * 3;
    num_steps = 50 * 3;
    for (int i = 0; i <= num_steps; ++i)
    {
        float n = (float) i / (float) num_steps;
        float phi = n * 2 * PI * 2.f;
        
        float rotateAngle = n * 0.01f;
        float2 rotatedUV = Rotate(uv + float2(cos(phi), sin(phi)) * 0.01f, rotateAngle) + 0.5f;

        float3 starburst = realDistributionSource.SampleLevel(imageSampler, rotatedUV, 0).rgb * !Clamped(rotatedUV);
        result += starburst * lerp(lambda2RGB(lerp(LAMBDA_NM_BLUE, LAMBDA_NM_RED, n)), 1.f, 0.5f);
    }

    result /= (float) num_steps;

    realDistributionDestination[ID] = float4(result, 1.0);
}