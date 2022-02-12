#include <math.h>
typedef float f32;
typedef int s32;
typedef unsigned int u32;
typedef double f64;

f32 NanoToMicro(f32 NanoMeter);
f32 MeterToNano(f32 Meter);
f32 MeterToMicro(f32 Meter);

struct DispersionCurveEquationCoef
{
	f32 A0;
	f32 A1;
	f32 A2;
	f32 A3;
	f32 A4;
	f32 A5;
	f32 A6;
	f32 A7;
	f32 A8;

	f32 computeRefIndex(f32 lambdaInMicroMeter);
};

struct AbbeParam
{
	f32 nC;
	f32 nd;
	f32 nF;

	f32 computeAbbeNumber_dLine();
};

struct Lens
{
	DispersionCurveEquationCoef coef;
	AbbeParam abbe;
};

enum LensName
{
	J_FK5 = 0,
	J_FK01A,
	J_FKH1,
	J_FKH2,
	J_PKH1,
	J_PSK02,
	J_PSK03,
	J_PSKH1,
	J_PSKH4,
	J_PSKH8,
	J_BK7A,
	J_BAK1,
	J_BAK2,
	J_BAK4,
	J_K3,
	J_K5,
	J_KZFH1,
	J_LZFH4,
	J_KZFH6,
	J_KZFH7,
	J_KZFH9,
	J_KF6,
	J_BALF4,
	J_BAF3,
	J_BAF4,
	J_BAF8,
	J_BAF10,
	J_BAF11,
	J_BAF12,
	LENS_NAME_MAX
};

void constructLens(Lens& lens, const s32 idx);