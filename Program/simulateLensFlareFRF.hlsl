struct CBuffer
{
    float lambda;
    float distance;
    float intervalX;
    float intervalY;
};

ConstantBuffer<CBuffer> computeConstants : register(b0);

RWTexture2D<float> realDistribution : register(u0);
RWTexture2D<float> imaginaryDistribution : register(u1);

SamplerState imageSampler : register(s0);

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void drawFRF(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ID = dispatchThreadID.xy;

    float k = 1.0f / computeConstants.lambda / computeConstants.lambda;

    float fourier_intervalx = 1.0f / (computeConstants.intervalX * WIDTH);
    float fourier_intervaly = 1.0f / (computeConstants.intervalY * HEIGHT);

    float w_x = 2 * fourier_intervalx * computeConstants.distance;
    float halfband_x = 1.0f / (sqrt(w_x * w_x + 1.0f) * computeConstants.lambda);
    float w_y = 2 * fourier_intervaly * computeConstants.distance;
    float halfband_y = 1.0f / (sqrt(w_y * w_y + 1.0f) * computeConstants.lambda);

    float kx = (ID.x - WIDTH / 2.0f) * fourier_intervalx;
    float ky = (ID.y - HEIGHT / 2.0f) * fourier_intervaly;

    float w = k - kx * kx - ky * ky;

    float color_real = 0;
    float color_image = 0;

	if (w > 0 && abs(kx) < halfband_x && abs(ky) < halfband_y)//band limiting
	{
        float phase = 2 * PI * sqrt(w) * computeConstants.distance;

        color_real = cos(phase);
        color_image = sin(phase);
    }

    realDistribution[ID] = color_real;
    imaginaryDistribution[ID] = color_image;
}
