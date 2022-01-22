float3 lambda2RGB(float lambda)
{
    float3 colRGB;

    if (lambda < 350.0)
        colRGB = float3(0.5, 0.0, 1.0);
    else if ((lambda >= 350.0) && (lambda < 440.0))
        colRGB = float3((440.0 - lambda) / 90.0, 0.0, 1.0);
    else if ((lambda >= 440.0) && (lambda <= 490.0))
        colRGB = float3(0.0, (lambda - 440.0) / 50.0, 1.0);
    else if ((lambda >= 490.0) && (lambda < 510.0))
        colRGB = float3(0.0, 1.0, (-(lambda - 510.0)) / 20.0);
    else if ((lambda >= 510.0) && (lambda < 580.0))
        colRGB = float3((lambda - 510.0) / 70.0, 1.0, 0.0);
    else if ((lambda >= 580.0) && (lambda < 645.0))
        colRGB = float3(1.0, (-(lambda - 645.0)) / 65.0, 0.0);
    else
        colRGB = float3(1.0, 0.0, 0.0);

    if (lambda < 350.0)
        colRGB *= 0.3;
    else if ((lambda >= 350.0) && (lambda < 420.0))
        colRGB *= 0.3 + (0.7 * ((lambda - 350.0) / 70.0));
    else if ((lambda >= 420.0) && (lambda <= 700.0))
        colRGB *= 1.0;
    else if ((lambda > 700.0) && (lambda <= 780.0))
        colRGB *= 0.3 + (0.7 * ((780.0 - lambda) / 80.0));
    else
        colRGB *= 0.3;

    return colRGB;
}