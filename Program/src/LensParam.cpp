#include "../include/LensParam.h"

f32 NanoToMicro(f32 NanoMeter)
{
	return NanoMeter * 1e-3;
}

f32 MeterToNano(f32 Meter)
{
	return Meter * 1e9;
}

f32 MeterToMicro(f32 Meter)
{
	return Meter * 1e6;
}

f32 DispersionCurveEquationCoef::computeRefIndex(f32 lambdaInMicroMeter)
{
	return sqrt(A0
		+ A1 * pow(lambdaInMicroMeter, 2)
		+ A2 * pow(lambdaInMicroMeter, 4)
		+ A3 * pow(lambdaInMicroMeter, -2)
		+ A4 * pow(lambdaInMicroMeter, -4)
		+ A5 * pow(lambdaInMicroMeter, -6)
		+ A6 * pow(lambdaInMicroMeter, -8)
		+ A7 * pow(lambdaInMicroMeter, -10)
		+ A8 * pow(lambdaInMicroMeter, -12));
}

f32 AbbeParam::computeAbbeNumber_dLine()
{
	return (nd - 1) / (nF - nC);
}

