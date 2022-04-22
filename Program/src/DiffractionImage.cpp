#include "../include/PBLensFlare.h"

void PBLensFlare::generateBurst()
{
	PIXBeginEvent(mCommandList.Get(), 0, "GenerateBurst");

	for (auto& tex : mRWFullsizeTex)
	{
		clear(tex);
	}

	polygon(mRWFullsizeTex.at(1));
	copy(mFullsizeTex.at(ReadOmlyImage_ApertureDust), mRWFullsizeTex.at(4));
	whitening(mRWFullsizeTex.at(4), mRWFullsizeTex.at(3));
	complexMultiply(
		mRWFullsizeTex.at(1), mRWFullsizeTex.at(2)
		, mRWFullsizeTex.at(3), mRWFullsizeTex.at(4)
		, mRWFullsizeTex.at(5), mRWFullsizeTex.at(6));

	copy(mRWFullsizeTex.at(5), mRWdisplayTex.at(DisplayImage_Aperture));

	clear(mRWFullsizeTex.at(6));

	//Fraunhofer Diffraction
	FFT2D(mRWFullsizeTex.at(5), mRWFullsizeTex.at(6));

	intensity(
		mRWFullsizeTex.at(5), mRWFullsizeTex.at(6)
		, mRWFullsizeTex.at(0));

	lambdaIntegral(
		mRWFullsizeTex.at(0), mPreFiltterdBurstTex);
	burstFiltering(
		mPreFiltterdBurstTex, mBurstCachedTex);

	mBurstKernelRegenerate = false;

	PIXEndEvent(mCommandList.Get());
}
void PBLensFlare::generateGhost()
{
	PIXBeginEvent(mCommandList.Get(), 0, "GenerateGhost");

	for (auto& tex : mRWFullsizeTex)
	{
		clear(tex);
	}

	polygon(mRWFullsizeTex.at(1));
	copy(mRWFullsizeTex.at(1), mRWFullsizeTex.at(7));

	//Angular Spectrum Method
	FFT2D(mRWFullsizeTex.at(1), mRWFullsizeTex.at(2));
	drawFRF(mRWFullsizeTex.at(3), mRWFullsizeTex.at(4));
	complexMultiply(
		mRWFullsizeTex.at(1), mRWFullsizeTex.at(2)
		, mRWFullsizeTex.at(3), mRWFullsizeTex.at(4)
		, mRWFullsizeTex.at(5), mRWFullsizeTex.at(6));
	FFT2D(mRWFullsizeTex.at(5), mRWFullsizeTex.at(6), true);

	intensity(
		mRWFullsizeTex.at(5), mRWFullsizeTex.at(6)
		, mRWFullsizeTex.at(3));

	multiply(mRWFullsizeTex.at(3), mRWFullsizeTex.at(7), mGhostCachedTex);
	//oneElem(mRWFullsizeTex.at(1), mGhostCachedTex);

	mGhostKernelRegenerate = false;

	PIXEndEvent(mCommandList.Get());
}

//fft test
//void PBLensFlare::generateGhost()
//{
//	PIXBeginEvent(mCommandList.Get(), 0, "FFTTest");
//
//	for (auto& tex : mRWFullsizeTex)
//	{
//		clear(tex);
//	}
//
//	polygon(mRWFullsizeTex.at(1));
//	copy(mRWFullsizeTex.at(1), mRWFullsizeTex.at(7));
//
//	//Angular Spectrum Method
//	FFT2D(mRWFullsizeTex.at(1), mRWFullsizeTex.at(2));
//	FFT2D(mRWFullsizeTex.at(1), mRWFullsizeTex.at(2), false);
//
//	intensity(
//		mRWFullsizeTex.at(1), mRWFullsizeTex.at(2)
//		, mRWFullsizeTex.at(3));
//
//	multiply(mRWFullsizeTex.at(3), mRWFullsizeTex.at(7), mGhostCachedTex);
//	//oneElem(mRWFullsizeTex.at(1), mGhostCachedTex);
//
//	//mGhostKernelRegenerate = false;
//
//	PIXEndEvent(mCommandList.Get());
//}

void PBLensFlare::drawFRF(DX12Buffer& Real, DX12Buffer& Image)
{
	PIXBeginEvent(mCommandList.Get(), 0, "drawFRF");
	setPipelineState(PipelineType_Compute, ShaderNameCompute_FRF);
	setPipelineConstantResource("computeConstants", mFRFCB[0]->GetGPUVirtualAddress());
	setPipelineResource("realDistribution", Real.getUAV(mCommandList));
	setPipelineResource("imaginaryDistribution", Image.getUAV(mCommandList));
	dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::FFT2D(DX12Buffer& Real, DX12Buffer& Image, const bool inverse)
{
	PIXBeginEvent(mCommandList.Get(), 0, inverse ? "ifftCS_ROW" : "fftCS_ROW");
	setPipelineState(PipelineType_Compute, inverse ? ShaderNameCompute_InvFFTrow : ShaderNameCompute_FFTrow);
	setPipelineResource("realDistributionSource", Real.getSRV(mCommandList));
	setPipelineResource("imaginaryDistributionSource", Image.getSRV(mCommandList));
	setPipelineResource("realDistributionDestination", mRWfullsizeInnerTex.at(2).getUAV(mCommandList));
	setPipelineResource("imaginaryDistributionDestination", mRWfullsizeInnerTex.at(3).getUAV(mCommandList));
	dispatch(1, mTexheight, 1);
	PIXEndEvent(mCommandList.Get());

	PIXBeginEvent(mCommandList.Get(), 0, inverse ? "ifftCS_COL" : "fftCS_COL");
	setPipelineState(PipelineType_Compute, inverse ? ShaderNameCompute_InvFFTcol : ShaderNameCompute_FFTcol);
	setPipelineResource("realDistributionSource", mRWfullsizeInnerTex.at(2).getSRV(mCommandList));
	setPipelineResource("imaginaryDistributionSource", mRWfullsizeInnerTex.at(3).getSRV(mCommandList));
	setPipelineResource("realDistributionDestination", Real.getUAV(mCommandList));
	setPipelineResource("imaginaryDistributionDestination", Image.getUAV(mCommandList));
	dispatch(1, mTexheight, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::raiseValue(DX12Buffer& InReal, DX12Buffer& InImage, DX12Buffer& OutReal, DX12Buffer& OutImage)
{
	PIXBeginEvent(mCommandList.Get(), 0, "raiseRICS");
	setPipelineState(PipelineType_Compute, ShaderNameCompute_RaiseValue);
	setPipelineConstantResource("computeConstants", mBurstCB[0]->GetGPUVirtualAddress());
	setPipelineResource("realDistributionSource", InReal.getSRV(mCommandList));
	setPipelineResource("imaginaryDistributionSource", InImage.getSRV(mCommandList));
	setPipelineResource("realDistributionDestination", OutReal.getUAV(mCommandList));
	setPipelineResource("imaginaryDistributionDestination", OutImage.getUAV(mCommandList));
	dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::lambdaIntegral(DX12Buffer& In, DX12Buffer& Out)
{
	PIXBeginEvent(mCommandList.Get(), 0, "lambdaIntegral");
	setPipelineState(PipelineType_Compute, ShaderNameCompute_LambdaIntegral);
	setPipelineConstantResource("computeConstants", mBurstCB[0]->GetGPUVirtualAddress());
	setPipelineResource("sourceDistribution", In.getSRV(mCommandList));
	setPipelineResource("realDistributionDestination", Out.getUAV(mCommandList));
	dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::burstFiltering(DX12Buffer& In, DX12Buffer& Out)
{
	PIXBeginEvent(mCommandList.Get(), 0, "spectrumFilterling");
	setPipelineState(PipelineType_Compute, ShaderNameCompute_BurstFilter);
	setPipelineConstantResource("computeConstants", mBurstCB[0]->GetGPUVirtualAddress());
	setPipelineResource("realDistributionSource", In.getSRV(mCommandList));
	setPipelineResource("realDistributionDestination", Out.getUAV(mCommandList));
	dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::complexMultiply(DX12Buffer& InReal0, DX12Buffer& InImage0, DX12Buffer& InReal1, DX12Buffer& InImage1, DX12Buffer& OutReal, DX12Buffer& OutImage)
{
	PIXBeginEvent(mCommandList.Get(), 0, "mulCS");
	setPipelineState(PipelineType_Compute, ShaderNameCompute_ComplexMultiply);
	setPipelineConstantResource("computeConstants", mUtilityCB[0]->GetGPUVirtualAddress());
	setPipelineResource("realDistributionSource", InReal0.getSRV(mCommandList));
	setPipelineResource("imaginaryDistributionSource", InImage0.getSRV(mCommandList));
	setPipelineResource("realDistributionDestination0", InReal1.getSRV(mCommandList));
	setPipelineResource("imaginaryDistributionDestination0", InImage1.getSRV(mCommandList));
	setPipelineResource("realDistributionDestination1", OutReal.getUAV(mCommandList));
	setPipelineResource("imaginaryDistributionDestination1", OutImage.getUAV(mCommandList));
	dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::multiply(DX12Buffer& In1, DX12Buffer& In2, DX12Buffer& Out)
{
	PIXBeginEvent(mCommandList.Get(), 0, "MulCS");
	setPipelineState(PipelineType_Compute, ShaderNameCompute_Multiply);
	setPipelineConstantResource("computeConstants", mUtilityCB[0]->GetGPUVirtualAddress());
	setPipelineResource("realDistributionSource", In1.getSRV(mCommandList));
	setPipelineResource("imaginaryDistributionSource", In2.getSRV(mCommandList));
	setPipelineResource("realDistributionDestination0", Out.getUAV(mCommandList));
	dispatch(mGhostTexWidth / mLensFlareComputeInformation.NUM_THREADS, mGhostTexHeight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::whitening(DX12Buffer& In, DX12Buffer& Out)
{
	PIXBeginEvent(mCommandList.Get(), 0, "whitening");
	setPipelineState(PipelineType_Compute, ShaderNameCompute_TextureWhitening);
	setPipelineConstantResource("computeConstants", mUtilityCB[0]->GetGPUVirtualAddress());
	setPipelineResource("realDistributionSource", In.getSRV(mCommandList));
	setPipelineResource("realDistributionDestination0", Out.getUAV(mCommandList));
	dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::copy(TextureData& In, DX12Buffer& Out)
{
	PIXBeginEvent(mCommandList.Get(), 0, "copyCS");
	setPipelineState(PipelineType_Compute, ShaderNameCompute_Copy);
	setPipelineConstantResource("computeConstants", mUtilityCB[0]->GetGPUVirtualAddress());
	setPipelineResource("realDistributionSource", In.SRV);
	setPipelineResource("realDistributionDestination0", Out.getUAV(mCommandList));
	dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::copy(DX12Buffer& In, DX12Buffer& Out)
{
	PIXBeginEvent(mCommandList.Get(), 0, "copyCS");
	setPipelineState(PipelineType_Compute, ShaderNameCompute_Copy);
	setPipelineConstantResource("computeConstants", mUtilityCB[0]->GetGPUVirtualAddress());
	setPipelineResource("realDistributionSource", In.getSRV(mCommandList));
	setPipelineResource("realDistributionDestination0", Out.getUAV(mCommandList));
	dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::clear(DX12Buffer& Tex)
{
	PIXBeginEvent(mCommandList.Get(), 0, "clearCS");
	setPipelineState(PipelineType_Compute, ShaderNameCompute_Clear);
	setPipelineConstantResource("computeConstants", mUtilityCB[0]->GetGPUVirtualAddress());
	setPipelineResource("realDistributionDestination0", Tex.getUAV(mCommandList));
	dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::polygon(DX12Buffer& Tex)
{
	PIXBeginEvent(mCommandList.Get(), 0, "drawfixpolygonCS");
	setPipelineState(PipelineType_Compute, ShaderNameCompute_Polygon);
	setPipelineConstantResource("computeConstants", mUtilityCB[0]->GetGPUVirtualAddress());
	setPipelineResource("realDistributionDestination0", Tex.getUAV(mCommandList));
	dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}



void PBLensFlare::amplitude(DX12Buffer& InReal, DX12Buffer& InImage, DX12Buffer& Amp)
{
	PIXBeginEvent(mCommandList.Get(), 0, "ampCS");
	setPipelineState(PipelineType_Compute, ShaderNameCompute_Amplitude);
	setPipelineResource("realDistributionSource", InReal.getSRV(mCommandList));
	setPipelineResource("imaginaryDistributionSource", InImage.getSRV(mCommandList));
	setPipelineResource("destDestination", Amp.getUAV(mCommandList));
	dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::intensity(DX12Buffer& InReal, DX12Buffer& InImage, DX12Buffer& Int)
{
	PIXBeginEvent(mCommandList.Get(), 0, "intenCS");
	setPipelineState(PipelineType_Compute, ShaderNameCompute_Intensity);
	setPipelineResource("realDistributionSource", InReal.getSRV(mCommandList));
	setPipelineResource("imaginaryDistributionSource", InImage.getSRV(mCommandList));
	setPipelineResource("destDestination", Int.getUAV(mCommandList));
	dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::oneElem(DX12Buffer& In, DX12Buffer& Out)
{
	PIXBeginEvent(mCommandList.Get(), 0, "oneElemCS");
	setPipelineState(PipelineType_Compute, ShaderNameCompute_OneElem);
	setPipelineResource("src", In.getSRV(mCommandList));
	setPipelineResource("dst", Out.getUAV(mCommandList));
	mCommandList->Dispatch(mGhostTexWidth / mLensFlareComputeInformation.NUM_THREADS, mGhostTexHeight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}