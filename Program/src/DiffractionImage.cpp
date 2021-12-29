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
	FFT2D(mRWFullsizeTex.at(5), mRWFullsizeTex.at(6));
	intensity(
		mRWFullsizeTex.at(5), mRWFullsizeTex.at(6)
		, mRWFullsizeTex.at(0), mRWFullsizeTex.at(1));

	lambdaIntegral(
		mRWFullsizeTex.at(0), mRWFullsizeTex.at(1)
		, mRWFullsizeTex.at(2), mRWFullsizeTex.at(3));
	burstFiltering
	(mRWFullsizeTex.at(2), mRWFullsizeTex.at(3));
	raiseValue(
		mRWFullsizeTex.at(3), mRWFullsizeTex.at(4)
		, mRWFullsizeTex.at(5), mRWFullsizeTex.at(6));
	clear(mRWFullsizeTex.at(1));
	clear(mRWFullsizeTex.at(6));
	amplitude(
		mRWFullsizeTex.at(5), mRWFullsizeTex.at(6)
		, mBurstCachedTex, mRWFullsizeTex.at(1));

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
	FFT2D(mRWFullsizeTex.at(1), mRWFullsizeTex.at(2));
	drawFRF(mRWFullsizeTex.at(3), mRWFullsizeTex.at(4));
	complexMultiply(
		mRWFullsizeTex.at(1), mRWFullsizeTex.at(2)
		, mRWFullsizeTex.at(3), mRWFullsizeTex.at(4)
		, mRWFullsizeTex.at(5), mRWFullsizeTex.at(6));
	invFFT2D(mRWFullsizeTex.at(5), mRWFullsizeTex.at(6));

	intensity(
		mRWFullsizeTex.at(5), mRWFullsizeTex.at(6)
		, mRWFullsizeTex.at(3), mRWFullsizeTex.at(4));

	multiply(mRWFullsizeTex.at(3), mRWFullsizeTex.at(7), mRWFullsizeTex.at(1));
	oneElem(mRWFullsizeTex.at(1), mGhostCachedTex);

	mGhostKernelRegenerate = false;

	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::drawFRF(DX12Buffer& Real, DX12Buffer& Image)
{
	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_FRF].nameAtPipeline].Get());
	mCommandList->SetComputeRootConstantBufferView(mShaderSettingComputeTbl[ShaderNameCompute_FRF].descriptors["computeConstants"].rootParamIndex, mFRFCB[0]->GetGPUVirtualAddress());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_FRF].descriptors["realDistribution"].rootParamIndex, Real.getUAV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_FRF].descriptors["imaginaryDistribution"].rootParamIndex, Image.getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "drawFRF");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_FRF].nameAtPipeline].Get());
	mCommandList->Dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::FFT2D(DX12Buffer& Real, DX12Buffer& Image)
{
	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_FFTrow].nameAtPipeline].Get());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_FFTrow].descriptors["realDistributionSource"].rootParamIndex, Real.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_FFTrow].descriptors["imaginaryDistributionSource"].rootParamIndex, Image.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_FFTrow].descriptors["realDistributionDestination"].rootParamIndex, mRWfullsizeInnerTex.at(2).getUAV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_FFTrow].descriptors["imaginaryDistributionDestination"].rootParamIndex, mRWfullsizeInnerTex.at(3).getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "fftCS_ROW");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_FFTrow].nameAtPipeline].Get());
	mCommandList->Dispatch(1, mTexheight, 1);
	PIXEndEvent(mCommandList.Get());

	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_FFTcol].nameAtPipeline].Get());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_FFTcol].descriptors["realDistributionSource"].rootParamIndex, mRWfullsizeInnerTex.at(2).getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_FFTcol].descriptors["imaginaryDistributionSource"].rootParamIndex, mRWfullsizeInnerTex.at(3).getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_FFTcol].descriptors["realDistributionDestination"].rootParamIndex, Real.getUAV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_FFTcol].descriptors["imaginaryDistributionDestination"].rootParamIndex, Image.getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "fftCS_COL");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_FFTcol].nameAtPipeline].Get());
	mCommandList->Dispatch(1, mTexwidth, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::invFFT2D(DX12Buffer& Real, DX12Buffer& Image)
{
	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_InvFFTrow].nameAtPipeline].Get());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_InvFFTrow].descriptors["realDistributionSource"].rootParamIndex, Real.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_InvFFTrow].descriptors["imaginaryDistributionSource"].rootParamIndex, Image.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_InvFFTrow].descriptors["realDistributionDestination"].rootParamIndex, mRWfullsizeInnerTex.at(2).getUAV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_InvFFTrow].descriptors["imaginaryDistributionDestination"].rootParamIndex, mRWfullsizeInnerTex.at(3).getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "ifftCS_ROW");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_InvFFTrow].nameAtPipeline].Get());
	mCommandList->Dispatch(1, mTexheight, 1);
	PIXEndEvent(mCommandList.Get());

	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_InvFFTcol].nameAtPipeline].Get());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_InvFFTcol].descriptors["realDistributionSource"].rootParamIndex, mRWfullsizeInnerTex.at(2).getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_InvFFTcol].descriptors["imaginaryDistributionSource"].rootParamIndex, mRWfullsizeInnerTex.at(3).getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_InvFFTcol].descriptors["realDistributionDestination"].rootParamIndex, Real.getUAV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_InvFFTcol].descriptors["imaginaryDistributionDestination"].rootParamIndex, Image.getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "ifftCS_COL");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_InvFFTcol].nameAtPipeline].Get());
	mCommandList->Dispatch(1, mTexwidth, 1);
	PIXEndEvent(mCommandList.Get());
}



void PBLensFlare::raiseValue(DX12Buffer& InReal, DX12Buffer& InImage, DX12Buffer& OutReal, DX12Buffer& OutImage)
{
	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_RaiseValue].nameAtPipeline].Get());
	mCommandList->SetComputeRootConstantBufferView(mShaderSettingComputeTbl[ShaderNameCompute_RaiseValue].descriptors["computeConstants"].rootParamIndex, mBurstCB[0]->GetGPUVirtualAddress());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_RaiseValue].descriptors["realDistributionSource"].rootParamIndex, InReal.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_RaiseValue].descriptors["imaginaryDistributionSource"].rootParamIndex, InImage.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_RaiseValue].descriptors["realDistributionDestination"].rootParamIndex, OutReal.getUAV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_RaiseValue].descriptors["imaginaryDistributionDestination"].rootParamIndex, OutImage.getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "raiseRICS");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_RaiseValue].nameAtPipeline].Get());
	mCommandList->Dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::lambdaIntegral(DX12Buffer& InReal, DX12Buffer& InImage, DX12Buffer& OutReal, DX12Buffer& OutImage)
{
	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_LambdaIntegral].nameAtPipeline].Get());
	mCommandList->SetComputeRootConstantBufferView(mShaderSettingComputeTbl[ShaderNameCompute_LambdaIntegral].descriptors["computeConstants"].rootParamIndex, mBurstCB[0]->GetGPUVirtualAddress());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_LambdaIntegral].descriptors["realDistributionSource"].rootParamIndex, InReal.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_LambdaIntegral].descriptors["imaginaryDistributionSource"].rootParamIndex, InImage.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_LambdaIntegral].descriptors["realDistributionDestination"].rootParamIndex, OutReal.getUAV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_LambdaIntegral].descriptors["imaginaryDistributionDestination"].rootParamIndex, OutImage.getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "lambdaIntegral");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_LambdaIntegral].nameAtPipeline].Get());
	mCommandList->Dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::burstFiltering(DX12Buffer& In, DX12Buffer& Out)
{
	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_BurstFilter].nameAtPipeline].Get());
	mCommandList->SetComputeRootConstantBufferView(mShaderSettingComputeTbl[ShaderNameCompute_BurstFilter].descriptors["computeConstants"].rootParamIndex, mBurstCB[0]->GetGPUVirtualAddress());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_BurstFilter].descriptors["realDistributionSource"].rootParamIndex, In.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_BurstFilter].descriptors["realDistributionDestination"].rootParamIndex, Out.getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "spectrumFilterling");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_BurstFilter].nameAtPipeline].Get());
	mCommandList->Dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::complexMultiply(DX12Buffer& InReal0, DX12Buffer& InImage0, DX12Buffer& InReal1, DX12Buffer& InImage1, DX12Buffer& OutReal, DX12Buffer& OutImage)
{
	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_ComplexMultiply].nameAtPipeline].Get());
	mCommandList->SetComputeRootConstantBufferView(mShaderSettingComputeTbl[ShaderNameCompute_ComplexMultiply].descriptors["computeConstants"].rootParamIndex, mUtilityCB[0]->GetGPUVirtualAddress());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_ComplexMultiply].descriptors["realDistributionSource"].rootParamIndex, InReal0.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_ComplexMultiply].descriptors["imaginaryDistributionSource"].rootParamIndex, InImage0.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_ComplexMultiply].descriptors["realDistributionDestination0"].rootParamIndex, InReal1.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_ComplexMultiply].descriptors["imaginaryDistributionDestination0"].rootParamIndex, InImage1.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_ComplexMultiply].descriptors["realDistributionDestination1"].rootParamIndex, OutReal.getUAV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_ComplexMultiply].descriptors["imaginaryDistributionDestination1"].rootParamIndex, OutImage.getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "mulCS");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_ComplexMultiply].nameAtPipeline].Get());
	mCommandList->Dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::multiply(DX12Buffer& In1, DX12Buffer& In2, DX12Buffer& Out)
{
	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_Multiply].nameAtPipeline].Get());
	mCommandList->SetComputeRootConstantBufferView(mShaderSettingComputeTbl[ShaderNameCompute_Multiply].descriptors["computeConstants"].rootParamIndex, mUtilityCB[0]->GetGPUVirtualAddress());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_Multiply].descriptors["realDistributionSource"].rootParamIndex, In1.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_Multiply].descriptors["imaginaryDistributionSource"].rootParamIndex, In2.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_Multiply].descriptors["realDistributionDestination0"].rootParamIndex, Out.getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "MulCS");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_Multiply].nameAtPipeline].Get());
	mCommandList->Dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::whitening(DX12Buffer& In, DX12Buffer& Out)
{
	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_TextureWhitening].nameAtPipeline].Get());
	mCommandList->SetComputeRootConstantBufferView(mShaderSettingComputeTbl[ShaderNameCompute_TextureWhitening].descriptors["computeConstants"].rootParamIndex, mUtilityCB[0]->GetGPUVirtualAddress());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_TextureWhitening].descriptors["realDistributionSource"].rootParamIndex, In.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_TextureWhitening].descriptors["realDistributionDestination0"].rootParamIndex, Out.getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "whitening");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_TextureWhitening].nameAtPipeline].Get());
	mCommandList->Dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::copy(TextureData& In, DX12Buffer& Out)
{
	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_Copy].nameAtPipeline].Get());
	mCommandList->SetComputeRootConstantBufferView(mShaderSettingComputeTbl[ShaderNameCompute_Copy].descriptors["computeConstants"].rootParamIndex, mUtilityCB[0]->GetGPUVirtualAddress());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_Copy].descriptors["realDistributionSource"].rootParamIndex, In.SRV);
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_Copy].descriptors["realDistributionDestination0"].rootParamIndex, Out.getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "copyCS");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_Copy].nameAtPipeline].Get());
	mCommandList->Dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::copy(DX12Buffer& In, DX12Buffer& Out)
{
	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_Copy].nameAtPipeline].Get());
	mCommandList->SetComputeRootConstantBufferView(mShaderSettingComputeTbl[ShaderNameCompute_Copy].descriptors["computeConstants"].rootParamIndex, mUtilityCB[0]->GetGPUVirtualAddress());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_Copy].descriptors["realDistributionSource"].rootParamIndex, In.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_Copy].descriptors["realDistributionDestination0"].rootParamIndex, Out.getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "copyCS");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_Copy].nameAtPipeline].Get());
	mCommandList->Dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::clear(DX12Buffer& Tex)
{
	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_Clear].nameAtPipeline].Get());
	mCommandList->SetComputeRootConstantBufferView(mShaderSettingComputeTbl[ShaderNameCompute_Clear].descriptors["computeConstants"].rootParamIndex, mUtilityCB[0]->GetGPUVirtualAddress());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_Clear].descriptors["realDistributionDestination0"].rootParamIndex, Tex.getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "clearCS");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_Clear].nameAtPipeline].Get());
	mCommandList->Dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::polygon(DX12Buffer& Tex)
{
	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_Polygon].nameAtPipeline].Get());
	mCommandList->SetComputeRootConstantBufferView(mShaderSettingComputeTbl[ShaderNameCompute_Polygon].descriptors["computeConstants"].rootParamIndex, mUtilityCB[0]->GetGPUVirtualAddress());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_Polygon].descriptors["realDistributionDestination0"].rootParamIndex, Tex.getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "drawfixpolygonCS");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_Polygon].nameAtPipeline].Get());
	mCommandList->Dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}



void PBLensFlare::amplitude(DX12Buffer& InReal, DX12Buffer& InImage, DX12Buffer& OutReal, DX12Buffer& OutImage)
{
	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_Amplitude].nameAtPipeline].Get());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_Amplitude].descriptors["realDistributionSource"].rootParamIndex, InReal.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_Amplitude].descriptors["imaginaryDistributionSource"].rootParamIndex, InImage.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_Amplitude].descriptors["realDistributionDestination"].rootParamIndex, OutReal.getUAV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_Amplitude].descriptors["imaginaryDistributionDestination"].rootParamIndex, OutImage.getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "ampCS");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_Amplitude].nameAtPipeline].Get());
	mCommandList->Dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::intensity(DX12Buffer& InReal, DX12Buffer& InImage, DX12Buffer& OutReal, DX12Buffer& OutImage)
{
	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_Intensity].nameAtPipeline].Get());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_Intensity].descriptors["realDistributionSource"].rootParamIndex, InReal.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_Intensity].descriptors["imaginaryDistributionSource"].rootParamIndex, InImage.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_Intensity].descriptors["realDistributionDestination"].rootParamIndex, OutReal.getUAV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_Intensity].descriptors["imaginaryDistributionDestination"].rootParamIndex, OutImage.getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "intenCS");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_Intensity].nameAtPipeline].Get());
	mCommandList->Dispatch(mTexwidth / mLensFlareComputeInformation.NUM_THREADS, mTexheight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::oneElem(DX12Buffer& In, DX12Buffer& Out)
{
	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[ShaderNameCompute_OneElem].nameAtPipeline].Get());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_OneElem].descriptors["src"].rootParamIndex, In.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[ShaderNameCompute_OneElem].descriptors["dst"].rootParamIndex, Out.getUAV(mCommandList));
	PIXBeginEvent(mCommandList.Get(), 0, "oneElemCS");
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[ShaderNameCompute_OneElem].nameAtPipeline].Get());
	mCommandList->Dispatch(mGhostTexWidth / mLensFlareComputeInformation.NUM_THREADS, mGhostTexHeight / mLensFlareComputeInformation.NUM_THREADS, 1);
	PIXEndEvent(mCommandList.Get());
}