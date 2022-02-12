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

//https://www.nikon.com/products/optical-glass/assets/pdf/hikari_catalog.pdf
void constructLens(Lens& lens, const s32 idx)
{
	switch (idx)
	{
	case J_FK5:
	{
		lens.coef.A0 = 2.18826855E+00;
		lens.coef.A1 = -9.19044724E-03;
		lens.coef.A2 = -1.11621071E-04;
		lens.coef.A3 = 9.26372815E-03;
		lens.coef.A4 = 7.34900733E-05;
		lens.coef.A5 = 4.19724242E-06;
		lens.coef.A6 = -1.15412203E-07;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.485343;
		lens.abbe.nd = 1.487490;
		lens.abbe.nF = 1.492276;
	}
	break;
	case J_FK01A:
	{
		lens.coef.A0 = 2.21785004E+00;
		lens.coef.A1 = -5.52619544E-03;
		lens.coef.A2 = -4.04219098E-05;
		lens.coef.A3 = 8.39820345E-03;
		lens.coef.A4 = 8.80190880E-05;
		lens.coef.A5 = 1.15723877E-07;
		lens.coef.A6 = 5.38178618E-08;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.495139;
		lens.abbe.nd = 1.497000;
		lens.abbe.nF = 1.501226;
	}
	break;
	case J_FKH1:
	{
		lens.coef.A0 = 2.22016073E+00;
		lens.coef.A1 = -5.00725473E-03;
		lens.coef.A2 = -3.55507111E-05;
		lens.coef.A3 = 8.42088796E-03;
		lens.coef.A4 = 7.02327459E-05;
		lens.coef.A5 = 2.47007900E-06;
		lens.coef.A6 = -6.50002003E-08;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.495980;
		lens.abbe.nd = 1.497820;
		lens.abbe.nF = 1.502009;
	}
	break;
	case J_FKH2:
	{
		lens.coef.A0 = 2.10149795E+00;
		lens.coef.A1 = -4.68337833E-03;
		lens.coef.A2 = -1.34642385E-05;
		lens.coef.A3 = 6.77542246E-03;
		lens.coef.A4 = 4.53499889E-05;
		lens.coef.A5 = 2.24209054E-06;
		lens.coef.A6 = -6.21790903E-08;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.454469;
		lens.abbe.nd = 1.456000;
		lens.abbe.nF = 1.459460;
	}
	break;
	case J_PKH1:
	{
		lens.coef.A0 = 2.27892705E+00;
		lens.coef.A1 = -9.04327622E-03;
		lens.coef.A2 = -1.10679206E-04;
		lens.coef.A3 = 1.01870033E-02;
		lens.coef.A4 = 9.31149884E-05;
		lens.coef.A5 = 2.72256540E-06;
		lens.coef.A6 = -1.82952398E-08;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.516311;
		lens.abbe.nd = 1.518600;
		lens.abbe.nF = 1.523731;
	}
	break;
	case J_PSK02:
	{
		lens.coef.A0 = 2.57826227E+00;
		lens.coef.A1 = -9.69723449E-03;
		lens.coef.A2 = -1.07085207E-04;
		lens.coef.A3 = 1.43480110E-02;
		lens.coef.A4 = 1.59222199E-04;
		lens.coef.A5 = 5.33085601E-06;
		lens.coef.A6 = -5.80638431E-08;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.615024;
		lens.abbe.nd = 1.618000;
		lens.abbe.nF = 1.624781;
	}
	break;
	case J_PSK03:
	{
		lens.coef.A0 = 2.53267453E+00;
		lens.coef.A1 = -9.50416844E-03;
		lens.coef.A2 = -1.06883723E-04;
		lens.coef.A3 = 1.34397360E-02;
		lens.coef.A4 = 1.41770605E-04;
		lens.coef.A5 = 4.73043880E-06;
		lens.coef.A6 = -8.62000830E-08;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.600183;
		lens.abbe.nd = 1.603000;
		lens.abbe.nF = 1.609398;
	}
	break;
	case J_PSKH1:
	{
		lens.coef.A0 = 2.50208083E+00;
		lens.coef.A1 = -6.72143907E-03;
		lens.coef.A2 = -5.34313751E-05;
		lens.coef.A3 = 1.28264400E-02;
		lens.coef.A4 = 1.56205388E-04;
		lens.coef.A5 = 1.21593549E-06;
		lens.coef.A6 = 9.59550869E-08;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.590540;
		lens.abbe.nd = 1.593190;
		lens.abbe.nF = 1.599276;
	}
	break;
	case J_PSKH4:
	{
		lens.coef.A0 = 2.50453078E+00;
		lens.coef.A1 = -1.01597822E-02;
		lens.coef.A2 = -1.08653142E-04;
		lens.coef.A3 = 1.27723327E-02;
		lens.coef.A4 = 1.33860625E-04;
		lens.coef.A5 = 3.37285381E-06;
		lens.coef.A6 = -2.56491019E-08;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.590771;
		lens.abbe.nd = 1.593490;
		lens.abbe.nF = 1.599629;
	}
	break;
	case J_PSKH8:
	{
		lens.coef.A0 = 2.60815614E+00;
		lens.coef.A1 = -8.16775932E-03;
		lens.coef.A2 = 0.00000000E+00;
		lens.coef.A3 = 1.50613778E-02;
		lens.coef.A4 = 3.69238186E-04;
		lens.coef.A5 = -1.11180030E-05;
		lens.coef.A6 = 1.41616753E-06;
		lens.coef.A7 = -6.52373713E-08;
		lens.coef.A8 = 6.98536029E-09;
		lens.abbe.nC = 1.625268;
		lens.abbe.nd = 1.628460;
		lens.abbe.nF = 1.635889;
	}
	break;
	case J_BK7A:
	{
		lens.coef.A0 = 2.27110883E+00;
		lens.coef.A1 = -9.38988354E-03;
		lens.coef.A2 = -1.00277081E-04;
		lens.coef.A3 = 1.09572221E-02;
		lens.coef.A4 = 1.20210067E-04;
		lens.coef.A5 = 3.31079774E-06;
		lens.coef.A6 = -1.48235581E-08;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.514324;
		lens.abbe.nd = 1.516800;
		lens.abbe.nF = 1.522382;
	}
	break;
	case J_BAK1:
	{
		lens.coef.A0 = 2.43258691E+00;
		lens.coef.A1 = -8.22086723E-03;
		lens.coef.A2 = -9.21764324E-05;
		lens.coef.A3 = 1.43187501E-02;
		lens.coef.A4 = 1.59799832E-04;
		lens.coef.A5 = 8.58344462E-06;
		lens.coef.A6 = -1.00538104E-07;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.569472;
		lens.abbe.nd = 1.572500;
		lens.abbe.nF = 1.579464;
	}
	break;
	case J_BAK2:
	{
		lens.coef.A0 = 2.33616060E+00;
		lens.coef.A1 = -8.18245071E-03;
		lens.coef.A2 = -9.82753897E-05;
		lens.coef.A3 = 1.27499096E-02;
		lens.coef.A4 = 1.22269251E-04;
		lens.coef.A5 = 8.48994057E-06;
		lens.coef.A6 = -1.59525058E-07;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.537199;
		lens.abbe.nd = 1.539960;
		lens.abbe.nF = 1.546271;
	}
	break;
	case J_BAK4:
	{
		lens.coef.A0 = 2.42114503E+00;
		lens.coef.A1 = -8.99959341E-03;
		lens.coef.A2 = -9.30006854E-05;
		lens.coef.A3 = 1.43071120E-02;
		lens.coef.A4 = 1.89993274E-04;
		lens.coef.A5 = 6.09602388E-06;
		lens.coef.A6 = 2.25737069E-07;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.565751;
		lens.abbe.nd = 1.568830;
		lens.abbe.nF = 1.575909;
	}
	break;
	case J_K3:
	{
		lens.coef.A0 = 2.27169182E+00;
		lens.coef.A1 = -8.15289465E-03;
		lens.coef.A2 = -6.46337623E-05;
		lens.coef.A3 = 1.19516164E-02;
		lens.coef.A4 = 1.76673730E-04;
		lens.coef.A5 = 1.45062194E-06;
		lens.coef.A6 = 2.24852090E-07;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.515551;
		lens.abbe.nd = 1.518230;
		lens.abbe.nF = 1.524362;
	}
	break;
	case J_K5:
	{
		lens.coef.A0 = 2.28421062E+00;
		lens.coef.A1 = -8.15537489E-03;
		lens.coef.A2 = -1.05573054E-04;
		lens.coef.A3 = 1.22386101E-02;
		lens.coef.A4 = 1.10833374E-04;
		lens.coef.A5 = 9.05979458E-06;
		lens.coef.A6 = -1.07673777E-07;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.519803;
		lens.abbe.nd = 1.522490;
		lens.abbe.nF = 1.528627;
	}
	break;
	case J_KZFH1:
	{
		lens.coef.A0 = 2.54674023E+00;
		lens.coef.A1 = -1.22652610E-02;
		lens.coef.A2 = -1.34279040E-04;
		lens.coef.A3 = 1.85970683E-02;
		lens.coef.A4 = 5.22959966E-04;
		lens.coef.A5 = -9.93145010E-06;
		lens.coef.A6 = 2.37371768E-06;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.608532;
		lens.abbe.nd = 1.612660;
		lens.abbe.nF = 1.622313;
	}
	break;
	case J_LZFH4:
	{
		lens.coef.A0 = 2.37404487E+00;
		lens.coef.A1 = -1.07631771E-02;
		lens.coef.A2 = -1.28642692E-04;
		lens.coef.A3 = 1.35709369E-02;
		lens.coef.A4 = 2.55765647E-04;
		lens.coef.A5 = -2.23388334E-06;
		lens.coef.A6 = 4.91067955E-07;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.549923;
		lens.abbe.nd = 1.552981;
		lens.abbe.nF = 1.559964;
	}
	break;
	case J_KZFH6:
	{
		lens.coef.A0 = 2.76011543E+00;
		lens.coef.A1 = -1.23775063E-02;
		lens.coef.A2 = 0.00000000E+00;
		lens.coef.A3 = 2.44484473E-02;
		lens.coef.A4 = 1.15656997E-03;
		lens.coef.A5 = -9.74000173E-05;
		lens.coef.A6 = 1.85226731E-05;
		lens.coef.A7 = -1.41521524E-06;
		lens.coef.A8 = 5.92951085E-08;
		lens.abbe.nC = 1.678397;
		lens.abbe.nd = 1.683760;
		lens.abbe.nF = 1.696564;
	}
	break;
	case J_KZFH7:
	{
		lens.coef.A0 = 2.92982429E+00;
		lens.coef.A1 = -1.15064058E-02;
		lens.coef.A2 = -9.01320677E-05;
		lens.coef.A3 = 2.42192502E-02;
		lens.coef.A4 = 4.84339860E-04;
		lens.coef.A5 = 3.92109984E-06;
		lens.coef.A6 = 7.18628425E-07;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.727358;
		lens.abbe.nd = 1.732110;
		lens.abbe.nF = 1.743210;
	}
	break;
	case J_KZFH9:
	{
		lens.coef.A0 = 2.92190512E+00;
		lens.coef.A1 = -1.31913454E-02;
		lens.coef.A2 = -7.94286252E-05;
		lens.coef.A3 = 3.27997049E-02;
		lens.coef.A4 = 7.00950165E-04;
		lens.coef.A5 = 1.24169090E-04;
		lens.coef.A6 = -1.12359582E-05;
		lens.coef.A7 = 9.11052912E-07;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.731309;
		lens.abbe.nd = 1.738000;
		lens.abbe.nF = 1.754185;
	}
	break;
	case J_KF6:
	{
		lens.coef.A0 = 2.26653222E+00;
		lens.coef.A1 = -9.74283829E-03;
		lens.coef.A2 = -8.49115572E-05;
		lens.coef.A3 = 1.27195343E-02;
		lens.coef.A4 = 3.15395806E-04;
		lens.coef.A5 = -8.83703038E-06;
		lens.coef.A6 = 1.84064027E-06;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.514429;
		lens.abbe.nd = 1.517420;
		lens.abbe.nF = 1.524341;
	}
	break;
	case J_BALF4:
	{
		lens.coef.A0 = 2.45156936E+00;
		lens.coef.A1 = -8.35914203E-03;
		lens.coef.A2 = -8.88499407E-05;
		lens.coef.A3 = 1.53016408E-02;
		lens.coef.A4 = 2.24512880E-04;
		lens.coef.A5 = 5.89498036E-06;
		lens.coef.A6 = 2.59209632E-07;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.576316;
		lens.abbe.nd = 1.579570;
		lens.abbe.nF = 1.587100;
	}
	break;
	case J_BAF3:
	{
		lens.coef.A0 = 2.45448839E+00;
		lens.coef.A1 = -8.67148963E-03;
		lens.coef.A2 = -1.04715240E-04;
		lens.coef.A3 = 1.76039752E-02;
		lens.coef.A4 = 1.54610243E-04;
		lens.coef.A5 = 5.59918259E-05;
		lens.coef.A6 = -5.01297284E-06;
		lens.coef.A7 = 3.17557990E-0;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.578929;
		lens.abbe.nd = 1.582670;
		lens.abbe.nF = 1.591464;
	}
	break;
	case J_BAF4:
	{
		lens.coef.A0 = 2.52175840E+00;
		lens.coef.A1 = -9.79498428E-03;
		lens.coef.A2 = -1.34973275E-04;
		lens.coef.A3 = 1.97297837E-02;
		lens.coef.A4 = 7.13034071E-05;
		lens.coef.A5 = 1.03716753E-04;
		lens.coef.A6 = 6.63899530E-07;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.601481;
		lens.abbe.nd = 1.605620;
		lens.abbe.nF = 1.615408;
	}
	break;
	case J_BAF8:
	{
		lens.coef.A0 = 2.58219095E+00;
		lens.coef.A1 = -9.86301021E-03;
		lens.coef.A2 = -1.16286506E-04;
		lens.coef.A3 = 1.89733467E-02;
		lens.coef.A4 = 2.19248923E-04;
		lens.coef.A5 = 4.98624477E-05;
		lens.coef.A6 = -4.45223153E-06;
		lens.coef.A7 = 3.07817299E-07;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.619775;
		lens.abbe.nd = 1.623740;
		lens.abbe.nF = 1.633044;
	}
	break;
	case J_BAF10:
	{
		lens.coef.A0 = 2.72808119E+00;
		lens.coef.A1 = -9.30210914E-03;
		lens.coef.A2 = -7.12221204E-05;
		lens.coef.A3 = 2.08031569E-02;
		lens.coef.A4 = 4.57311835E-04;
		lens.coef.A5 = -2.96273778E-06;
		lens.coef.A6 = 1.63114030E-06;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.665785;
		lens.abbe.nd = 1.670030;
		lens.abbe.nF = 1.679998;
	}
	break;
	case J_BAF11:
	{
		lens.coef.A0 = 2.71886836E+00;
		lens.coef.A1 = -9.21086428E-03;
		lens.coef.A2 = -5.97080099E-05;
		lens.coef.A3 = 2.02512558E-02;
		lens.coef.A4 = 4.23467645E-04;
		lens.coef.A5 = -1.03717059E-06;
		lens.coef.A6 = 1.22100678E-06;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.662593;
		lens.abbe.nd = 1.666720;
		lens.abbe.nF = 1.676388;
	}
	break;
	case J_BAF12:
	{
		lens.coef.A0 = 2.62810335E+00;
		lens.coef.A1 = -9.95087731E-03;
		lens.coef.A2 = -1.44740792E-04;
		lens.coef.A3 = 2.06473464E-02;
		lens.coef.A4 = 1.62531777E-04;
		lens.coef.A5 = 7.85240289E-05;
		lens.coef.A6 = -7.45350927E-06;
		lens.coef.A7 = 4.83617341E-07;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.635055;
		lens.abbe.nd = 1.639300;
		lens.abbe.nF = 1.649314;
	}
	break;
	default:
	{
		lens.coef.A0 = 2.18826855E+00;
		lens.coef.A1 = -9.19044724E-03;
		lens.coef.A2 = -1.11621071E-04;
		lens.coef.A3 = 9.26372815E-03;
		lens.coef.A4 = 7.34900733E-05;
		lens.coef.A5 = 4.19724242E-06;
		lens.coef.A6 = -1.15412203E-07;
		lens.coef.A7 = 0.00000000E+00;
		lens.coef.A8 = 0.00000000E+00;
		lens.abbe.nC = 1.485343;
		lens.abbe.nd = 1.487490;
		lens.abbe.nF = 1.492276;
	}
	break;
	}
}