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

//https://www.nikon.com/products/optical-glass/assets/pdf/hikari_catalog.pdf
const static Lens LENS_TABLE[LENS_NAME_MAX] =
{
	{
		 2.18826855E+00,
		 -9.19044724E-03,
		 -1.11621071E-04,
		 9.26372815E-03,
		 7.34900733E-05,
		 4.19724242E-06,
		 -1.15412203E-07,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.485343,
		 1.487490,
		 1.492276
	},
	{
		 2.21785004E+00,
		 -5.52619544E-03,
		 -4.04219098E-05,
		 8.39820345E-03,
		 8.80190880E-05,
		 1.15723877E-07,
		 5.38178618E-08,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.495139,
		 1.497000,
		 1.501226
	},
	{
		 2.22016073E+00,
		 -5.00725473E-03,
		 -3.55507111E-05,
		 8.42088796E-03,
		 7.02327459E-05,
		 2.47007900E-06,
		 -6.50002003E-08,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.495980,
		 1.497820,
		 1.502009
	},
	{
		 2.10149795E+00,
		 -4.68337833E-03,
		 -1.34642385E-05,
		 6.77542246E-03,
		 4.53499889E-05,
		 2.24209054E-06,
		 -6.21790903E-08,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.454469,
		 1.456000,
		 1.459460
	},
	{
		 2.27892705E+00,
		 -9.04327622E-03,
		 -1.10679206E-04,
		 1.01870033E-02,
		 9.31149884E-05,
		 2.72256540E-06,
		 -1.82952398E-08,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.516311,
		 1.518600,
		 1.523731
	},
	{
		 2.57826227E+00,
		 -9.69723449E-03,
		 -1.07085207E-04,
		 1.43480110E-02,
		 1.59222199E-04,
		 5.33085601E-06,
		 -5.80638431E-08,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.615024,
		 1.618000,
		 1.624781
	},
	{
		 2.53267453E+00,
		 -9.50416844E-03,
		 -1.06883723E-04,
		 1.34397360E-02,
		 1.41770605E-04,
		 4.73043880E-06,
		 -8.62000830E-08,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.600183,
		 1.603000,
		 1.609398
	},
	{
		 2.50208083E+00,
		 -6.72143907E-03,
		 -5.34313751E-05,
		 1.28264400E-02,
		 1.56205388E-04,
		 1.21593549E-06,
		 9.59550869E-08,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.590540,
		 1.593190,
		 1.599276
	},
	{
		 2.50453078E+00,
		 -1.01597822E-02,
		 -1.08653142E-04,
		 1.27723327E-02,
		 1.33860625E-04,
		 3.37285381E-06,
		 -2.56491019E-08,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.590771,
		 1.593490,
		 1.599629
	},
	{
		 2.60815614E+00,
		 -8.16775932E-03,
		 0.00000000E+00,
		 1.50613778E-02,
		 3.69238186E-04,
		 -1.11180030E-05,
		 1.41616753E-06,
		 -6.52373713E-08,
		 6.98536029E-09,
		 1.625268,
		 1.628460,
		 1.635889
	},
	{
		 2.27110883E+00,
		 -9.38988354E-03,
		 -1.00277081E-04,
		 1.09572221E-02,
		 1.20210067E-04,
		 3.31079774E-06,
		 -1.48235581E-08,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.514324,
		 1.516800,
		 1.522382
	},
	{
		 2.43258691E+00,
		 -8.22086723E-03,
		 -9.21764324E-05,
		 1.43187501E-02,
		 1.59799832E-04,
		 8.58344462E-06,
		 -1.00538104E-07,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.569472,
		 1.572500,
		 1.579464
	},
	{
		 2.33616060E+00,
		 -8.18245071E-03,
		 -9.82753897E-05,
		 1.27499096E-02,
		 1.22269251E-04,
		 8.48994057E-06,
		 -1.59525058E-07,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.537199,
		 1.539960,
		 1.546271
	},
	{
		 2.42114503E+00,
		 -8.99959341E-03,
		 -9.30006854E-05,
		 1.43071120E-02,
		 1.89993274E-04,
		 6.09602388E-06,
		 2.25737069E-07,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.565751,
		 1.568830,
		 1.575909
	},
	{
		 2.27169182E+00,
		 -8.15289465E-03,
		 -6.46337623E-05,
		 1.19516164E-02,
		 1.76673730E-04,
		 1.45062194E-06,
		 2.24852090E-07,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.515551,
		 1.518230,
		 1.524362
	},
	{
		 2.28421062E+00,
		 -8.15537489E-03,
		 -1.05573054E-04,
		 1.22386101E-02,
		 1.10833374E-04,
		 9.05979458E-06,
		 -1.07673777E-07,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.519803,
		 1.522490,
		 1.528627
	},
	{
		 2.54674023E+00,
		 -1.22652610E-02,
		 -1.34279040E-04,
		 1.85970683E-02,
		 5.22959966E-04,
		 -9.93145010E-06,
		 2.37371768E-06,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.608532,
		 1.612660,
		 1.622313
	},
	{
		 2.37404487E+00,
		 -1.07631771E-02,
		 -1.28642692E-04,
		 1.35709369E-02,
		 2.55765647E-04,
		 -2.23388334E-06,
		 4.91067955E-07,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.549923,
		 1.552981,
		 1.559964
	},
	{
		 2.76011543E+00,
		 -1.23775063E-02,
		 0.00000000E+00,
		 2.44484473E-02,
		 1.15656997E-03,
		 -9.74000173E-05,
		 1.85226731E-05,
		 -1.41521524E-06,
		 5.92951085E-08,
		 1.678397,
		 1.683760,
		 1.696564
	},
	{
		 2.92982429E+00,
		 -1.15064058E-02,
		 -9.01320677E-05,
		 2.42192502E-02,
		 4.84339860E-04,
		 3.92109984E-06,
		 7.18628425E-07,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.727358,
		 1.732110,
		 1.743210
	},
	{
		 2.92190512E+00,
		 -1.31913454E-02,
		 -7.94286252E-05,
		 3.27997049E-02,
		 7.00950165E-04,
		 1.24169090E-04,
		 -1.12359582E-05,
		 9.11052912E-07,
		 0.00000000E+00,
		 1.731309,
		 1.738000,
		 1.754185
	},
	{
		 2.26653222E+00,
		 -9.74283829E-03,
		 -8.49115572E-05,
		 1.27195343E-02,
		 3.15395806E-04,
		 -8.83703038E-06,
		 1.84064027E-06,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.514429,
		 1.517420,
		 1.524341
	},
	{
		 2.45156936E+00,
		 -8.35914203E-03,
		 -8.88499407E-05,
		 1.53016408E-02,
		 2.24512880E-04,
		 5.89498036E-06,
		 2.59209632E-07,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.576316,
		 1.579570,
		 1.587100
	},
	{
		 2.45448839E+00,
		 -8.67148963E-03,
		 -1.04715240E-04,
		 1.76039752E-02,
		 1.54610243E-04,
		 5.59918259E-05,
		 -5.01297284E-06,
		 3.17557990E-0,
		 0.00000000E+00,
		 1.578929,
		 1.582670,
		 1.591464
	},
	{
		 2.52175840E+00,
		 -9.79498428E-03,
		 -1.34973275E-04,
		 1.97297837E-02,
		 7.13034071E-05,
		 1.03716753E-04,
		 6.63899530E-07,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.601481,
		 1.605620,
		 1.615408
	},
	{
		 2.58219095E+00,
		 -9.86301021E-03,
		 -1.16286506E-04,
		 1.89733467E-02,
		 2.19248923E-04,
		 4.98624477E-05,
		 -4.45223153E-06,
		 3.07817299E-07,
		 0.00000000E+00,
		 1.619775,
		 1.623740,
		 1.633044
	},
	{
		 2.72808119E+00,
		 -9.30210914E-03,
		 -7.12221204E-05,
		 2.08031569E-02,
		 4.57311835E-04,
		 -2.96273778E-06,
		 1.63114030E-06,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.665785,
		 1.670030,
		 1.679998
	},
	{
		 2.71886836E+00,
		 -9.21086428E-03,
		 -5.97080099E-05,
		 2.02512558E-02,
		 4.23467645E-04,
		 -1.03717059E-06,
		 1.22100678E-06,
		 0.00000000E+00,
		 0.00000000E+00,
		 1.662593,
		 1.666720,
		 1.676388
	},
	{
		 2.62810335E+00,
		 -9.95087731E-03,
		 -1.44740792E-04,
		 2.06473464E-02,
		 1.62531777E-04,
		 7.85240289E-05,
		 -7.45350927E-06,
		 4.83617341E-07,
		 0.00000000E+00,
		 1.635055,
		 1.639300,
		 1.649314
	}
};

void constructLens(Lens& lens, const s32 idx);