# include "../include/PBLensFlare.h"
using namespace std;

#define PROJECT_PI 3.14149265

void PBLensFlare::setupComputePipeline()
{
	mShaderSettingComputeTbl.clear();
	mShaderSettingComputeTbl.resize(ShaderNameCompute_COUNT);
	s32 setting = ShaderNameCompute_TraceRay;

	{
		setting = ShaderNameCompute_TraceRay;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareRayTracing.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"rayTraceCS";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "traceRay";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_GROUPS", to_wstring(mLensFlareComputeInformation.NUM_GROUPS) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_THREADS", to_wstring(mLensFlareComputeInformation.NUM_THREADS) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_GHOSTS", to_wstring(mLensDescription.NumGhosts) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"GRID_DIV", to_wstring(mLensFlareComputeInformation.GRID_DIV) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"SAMPLE_LAMBDA_NUM", to_wstring(mLensFlareComputeInformation.SAMPLE_LAMBDA_NUM) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LAMBDA_NM_RED", to_wstring(mLensFlareComputeInformation.LAMBDA_NM_RED) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LAMBDA_NM_BLUE", to_wstring(mLensFlareComputeInformation.LAMBDA_NM_BLUE) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LENS_NAME_MAX", to_wstring(LENS_NAME_MAX) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("lensBank");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("traceResult");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("lensInterface");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("ghostData");
		mShaderSettingComputeTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["lensBank"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["traceResult"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["lensInterface"] = DescriptorInfo{ 3, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["ghostData"] = DescriptorInfo{ 4, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };	
		setting = ShaderNameCompute_TraceRayAR;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareRayTracing.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"rayTraceCS";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "traceRayAR";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_GROUPS", to_wstring(mLensFlareComputeInformation.NUM_GROUPS) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_THREADS", to_wstring(mLensFlareComputeInformation.NUM_THREADS) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_GHOSTS", to_wstring(mLensDescription.NumGhosts) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"GRID_DIV", to_wstring(mLensFlareComputeInformation.GRID_DIV) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"SAMPLE_LAMBDA_NUM", to_wstring(mLensFlareComputeInformation.SAMPLE_LAMBDA_NUM) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LAMBDA_NM_RED", to_wstring(mLensFlareComputeInformation.LAMBDA_NM_RED) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LAMBDA_NM_BLUE", to_wstring(mLensFlareComputeInformation.LAMBDA_NM_BLUE) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"AR_CORTING", to_wstring(1) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LENS_NAME_MAX", to_wstring(LENS_NAME_MAX) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("lensBank");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("traceResult");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("lensInterface");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("ghostData");
		mShaderSettingComputeTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["lensBank"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["traceResult"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["lensInterface"] = DescriptorInfo{ 3, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["ghostData"] = DescriptorInfo{ 4, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		setting = ShaderNameCompute_TraceRay_DEBUG;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareRayTracing.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"rayTraceCS";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "traceRay_DEBUG";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_GROUPS", to_wstring(mLensFlareComputeInformation.NUM_GROUPS) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_THREADS", to_wstring(mLensFlareComputeInformation.NUM_THREADS) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_GHOSTS", to_wstring(mLensDescription.NumGhosts) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"GRID_DIV", to_wstring(mLensFlareComputeInformation.GRID_DIV) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"SAMPLE_LAMBDA_NUM", to_wstring(mLensFlareComputeInformation.SAMPLE_LAMBDA_NUM) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LAMBDA_NM_RED", to_wstring(mLensFlareComputeInformation.LAMBDA_NM_RED) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LAMBDA_NM_BLUE", to_wstring(mLensFlareComputeInformation.LAMBDA_NM_BLUE) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LENS_NAME_MAX", to_wstring(LENS_NAME_MAX) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"DEBUG", to_wstring(1) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("lensBank");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("traceResult");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("lensInterface");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("ghostData");
		mShaderSettingComputeTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["lensBank"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["traceResult"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["lensInterface"] = DescriptorInfo{ 3, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["ghostData"] = DescriptorInfo{ 4, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		setting = ShaderNameCompute_TraceRayAR_DEBUG;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareRayTracing.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"rayTraceCS";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "traceRayAR_DEBUG";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_GROUPS", to_wstring(mLensFlareComputeInformation.NUM_GROUPS) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_THREADS", to_wstring(mLensFlareComputeInformation.NUM_THREADS) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_GHOSTS", to_wstring(mLensDescription.NumGhosts) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"GRID_DIV", to_wstring(mLensFlareComputeInformation.GRID_DIV) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"SAMPLE_LAMBDA_NUM", to_wstring(mLensFlareComputeInformation.SAMPLE_LAMBDA_NUM) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LAMBDA_NM_RED", to_wstring(mLensFlareComputeInformation.LAMBDA_NM_RED) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LAMBDA_NM_BLUE", to_wstring(mLensFlareComputeInformation.LAMBDA_NM_BLUE) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"AR_CORTING", to_wstring(1) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LENS_NAME_MAX", to_wstring(LENS_NAME_MAX) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"DEBUG", to_wstring(1) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("lensBank");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("traceResult");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("lensInterface");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("ghostData");
		mShaderSettingComputeTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["lensBank"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["traceResult"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["lensInterface"] = DescriptorInfo{ 3, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["ghostData"] = DescriptorInfo{ 4, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
	}

	{
		setting = ShaderNameCompute_FRF;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareFRF.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"drawFRF";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "drawFRF";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"WIDTH", to_wstring(mTexwidth) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"HEIGHT", to_wstring(mTexheight) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_THREADS", to_wstring(mLensFlareComputeInformation.NUM_THREADS) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistribution");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistribution");
		mShaderSettingComputeTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["realDistribution"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistribution"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1} };
	}

	{
		setting = ShaderNameCompute_FFTcol;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareFFT.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"FFT1D";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "FFTcol";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"WIDTH", to_wstring(mTexwidth) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"HEIGHT", to_wstring(mTexheight) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"ROW", to_wstring(0) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LENGTH", L"HEIGHT" });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"BUTTERFLY_COUNT", to_wstring(inv2pow(mTexheight)) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"INVERSE", to_wstring(0) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"DOUBLE", to_wstring(1) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination");
		mShaderSettingComputeTbl[setting].descriptors["realDistributionSource"] = DescriptorInfo{ 0, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionSource"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination"] = DescriptorInfo{ 3, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1} };
		setting = ShaderNameCompute_FFTrow;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareFFT.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"FFT1D";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "FFTrow";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"WIDTH", to_wstring(mTexwidth) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"HEIGHT", to_wstring(mTexheight) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"ROW", to_wstring(1) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LENGTH", L"WIDTH" });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"BUTTERFLY_COUNT", to_wstring(inv2pow(mTexwidth)) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"INVERSE", to_wstring(0) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"DOUBLE", to_wstring(1) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination");
		mShaderSettingComputeTbl[setting].descriptors["realDistributionSource"] = DescriptorInfo{ 0, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionSource"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination"] = DescriptorInfo{ 3, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1} };
		setting = ShaderNameCompute_InvFFTcol;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareFFT.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"FFT1D";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "InvFFTcol";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"WIDTH", to_wstring(mTexwidth) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"HEIGHT", to_wstring(mTexheight) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"ROW", to_wstring(0) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LENGTH", L"HEIGHT" });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"BUTTERFLY_COUNT", to_wstring(inv2pow(mTexheight)) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"INVERSE", to_wstring(1) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"DOUBLE", to_wstring(1) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination");
		mShaderSettingComputeTbl[setting].descriptors["realDistributionSource"] = DescriptorInfo{ 0, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionSource"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination"] = DescriptorInfo{ 3, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1} };
		setting = ShaderNameCompute_InvFFTrow;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareFFT.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"FFT1D";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "InvFFTrow";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"WIDTH", to_wstring(mTexwidth) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"HEIGHT", to_wstring(mTexheight) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"ROW", to_wstring(1) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LENGTH", L"WIDTH" });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"BUTTERFLY_COUNT", to_wstring(inv2pow(mTexwidth)) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"INVERSE", to_wstring(1) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"DOUBLE", to_wstring(1) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination");
		mShaderSettingComputeTbl[setting].descriptors["realDistributionSource"] = DescriptorInfo{ 0, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionSource"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination"] = DescriptorInfo{ 3, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1} };
	}

	{
		setting = ShaderNameCompute_RaiseValue;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareGenerateBurst.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"raiseValue";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "raiseValue";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"WIDTH", to_wstring(mTexwidth) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"HEIGHT", to_wstring(mTexheight) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_THREADS", to_wstring(mLensFlareComputeInformation.NUM_THREADS) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LAMBDA_NM_RED", to_wstring(mLensFlareComputeInformation.LAMBDA_NM_RED) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LAMBDA_NM_BLUE", to_wstring(mLensFlareComputeInformation.LAMBDA_NM_BLUE) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("sourceDistribution");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination");
		mShaderSettingComputeTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0 , D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["sourceDistribution"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionSource"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionSource"] = DescriptorInfo{ 3, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination"] = DescriptorInfo{ 4, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination"] = DescriptorInfo{ 5, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1} };
		setting = ShaderNameCompute_LambdaIntegral;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareGenerateBurst.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"lambdaIntegral";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "lambdaIntegral";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"WIDTH", to_wstring(mTexwidth) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"HEIGHT", to_wstring(mTexheight) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_THREADS", to_wstring(mLensFlareComputeInformation.NUM_THREADS) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LAMBDA_NM_RED", to_wstring(mLensFlareComputeInformation.LAMBDA_NM_RED) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LAMBDA_NM_BLUE", to_wstring(mLensFlareComputeInformation.LAMBDA_NM_BLUE) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("sourceDistribution");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination");
		mShaderSettingComputeTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0 , D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["sourceDistribution"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionSource"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionSource"] = DescriptorInfo{ 3, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination"] = DescriptorInfo{ 4, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination"] = DescriptorInfo{ 5, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1} };
		setting = ShaderNameCompute_BurstFilter;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareGenerateBurst.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"burstFilter";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "burstFilter";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"WIDTH", to_wstring(mTexwidth) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"HEIGHT", to_wstring(mTexheight) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_THREADS", to_wstring(mLensFlareComputeInformation.NUM_THREADS) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LAMBDA_NM_RED", to_wstring(mLensFlareComputeInformation.LAMBDA_NM_RED) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"LAMBDA_NM_BLUE", to_wstring(mLensFlareComputeInformation.LAMBDA_NM_BLUE) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("sourceDistribution");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination");
		mShaderSettingComputeTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0 , D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["sourceDistribution"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionSource"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionSource"] = DescriptorInfo{ 3, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination"] = DescriptorInfo{ 4, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination"] = DescriptorInfo{ 5, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1} };
	}

	{
		setting = ShaderNameCompute_ComplexMultiply;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareUtility.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"multiplyComplex";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "multiplyComplex";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"WIDTH", to_wstring(mTexwidth) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"HEIGHT", to_wstring(mTexheight) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_THREADS", to_wstring(mLensFlareComputeInformation.NUM_THREADS) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination0");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination0");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination1");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination1");
		mShaderSettingComputeTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0 , D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionSource"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionSource"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination0"] = DescriptorInfo{ 3, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination0"] = DescriptorInfo{ 4, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination1"] = DescriptorInfo{ 5, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination1"] = DescriptorInfo{ 6, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3} };
		setting = ShaderNameCompute_Multiply;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareUtility.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"multiplyReal";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "multiplyReal";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"WIDTH", to_wstring(mTexwidth) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"HEIGHT", to_wstring(mTexheight) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_THREADS", to_wstring(mLensFlareComputeInformation.NUM_THREADS) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination0");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination0");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination1");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination1");
		mShaderSettingComputeTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0 , D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionSource"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionSource"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination0"] = DescriptorInfo{ 3, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination0"] = DescriptorInfo{ 4, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination1"] = DescriptorInfo{ 5, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination1"] = DescriptorInfo{ 6, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3} };
		setting = ShaderNameCompute_TextureWhitening;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareUtility.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"whitening";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "whitening";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"WIDTH", to_wstring(mTexwidth) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"HEIGHT", to_wstring(mTexheight) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_THREADS", to_wstring(mLensFlareComputeInformation.NUM_THREADS) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination0");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination0");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination1");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination1");
		mShaderSettingComputeTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0 , D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionSource"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionSource"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination0"] = DescriptorInfo{ 3, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination0"] = DescriptorInfo{ 4, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination1"] = DescriptorInfo{ 5, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination1"] = DescriptorInfo{ 6, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3} };
		setting = ShaderNameCompute_Copy;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareUtility.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"copy";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "copy";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"WIDTH", to_wstring(mTexwidth) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"HEIGHT", to_wstring(mTexheight) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_THREADS", to_wstring(mLensFlareComputeInformation.NUM_THREADS) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination0");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination0");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination1");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination1");
		mShaderSettingComputeTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0 , D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionSource"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionSource"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination0"] = DescriptorInfo{ 3, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination0"] = DescriptorInfo{ 4, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination1"] = DescriptorInfo{ 5, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination1"] = DescriptorInfo{ 6, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3} };
		setting = ShaderNameCompute_Clear;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareUtility.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"clear";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "clear";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"WIDTH", to_wstring(mTexwidth) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"HEIGHT", to_wstring(mTexheight) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_THREADS", to_wstring(mLensFlareComputeInformation.NUM_THREADS) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination0");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination0");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination1");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination1");
		mShaderSettingComputeTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0 , D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionSource"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionSource"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination0"] = DescriptorInfo{ 3, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination0"] = DescriptorInfo{ 4, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination1"] = DescriptorInfo{ 5, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination1"] = DescriptorInfo{ 6, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3} };
		setting = ShaderNameCompute_Polygon;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareUtility.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"polygon";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "polygon";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"PI", to_wstring(PROJECT_PI) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"WIDTH", to_wstring(mTexwidth) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"HEIGHT", to_wstring(mTexheight) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_THREADS", to_wstring(mLensFlareComputeInformation.NUM_THREADS) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination0");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination0");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionDestination1");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionDestination1");
		mShaderSettingComputeTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0 , D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionSource"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionSource"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination0"] = DescriptorInfo{ 3, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination0"] = DescriptorInfo{ 4, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["realDistributionDestination1"] = DescriptorInfo{ 5, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionDestination1"] = DescriptorInfo{ 6, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3} };
	}

	{
	    setting = ShaderNameCompute_Amplitude;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareComplexDistribution.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"amplitude";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "amplitude";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_THREADS", to_wstring(mLensFlareComputeInformation.NUM_THREADS) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("destDestination");
		mShaderSettingComputeTbl[setting].descriptors["realDistributionSource"] = DescriptorInfo{ 0, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionSource"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["destDestination"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
		setting = ShaderNameCompute_Intensity;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareComplexDistribution.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"intensity";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "intensity";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_THREADS", to_wstring(mLensFlareComputeInformation.NUM_THREADS) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("realDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("imaginaryDistributionSource");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("destDestination");
		mShaderSettingComputeTbl[setting].descriptors["realDistributionSource"] = DescriptorInfo{ 0, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["imaginaryDistributionSource"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingComputeTbl[setting].descriptors["destDestination"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
	}

	{
		setting = ShaderNameCompute_OneElem;
		mShaderSettingComputeTbl[setting].shaderFileName = L"simulateLensFlareCopyToOneElem.hlsl";
		mShaderSettingComputeTbl[setting].shaderEntryPoint = L"oneElem";
		mShaderSettingComputeTbl[setting].nameAtPipeline = "oneElem";
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"NUM_THREADS", to_wstring(mLensFlareComputeInformation.NUM_THREADS) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"WIDTH", to_wstring(mGhostTexWidth) });
		mShaderSettingComputeTbl[setting].shaderMacro.push_back(Shader::DefineMacro{ L"HEIGHT", to_wstring(mGhostTexHeight) });
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("src");
		mShaderSettingComputeTbl[setting].descriptorKeys.push_back("dst");
		mShaderSettingComputeTbl[setting].descriptors["src"] = DescriptorInfo{ 0, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingComputeTbl[setting].descriptors["dst"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0} };
	}

	vector<CD3DX12_STATIC_SAMPLER_DESC> samplerDesc(1);
	samplerDesc[0].Init(
		0,
		D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		0,
		16,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
		0.f,
		D3D12_FLOAT32_MAX,
		D3D12_SHADER_VISIBILITY_ALL,
		0);

	setupComputePipelineAndSignature(samplerDesc);
}

void PBLensFlare::setupGraphicsPipeline()
{
	mShaderSettingGraphicsTbl.clear();
	mShaderSettingGraphicsTbl.resize(ShaderNameGraphics_COUNT);
	s32 setting = 0;
	{
		setting = ShaderNameGraphics_AddGhosts;
		mShaderSettingGraphicsTbl[setting].shaderFileNameVS = L"simulateLensFlareDrawing.hlsl";
		mShaderSettingGraphicsTbl[setting].shaderEntryPointVS = L"rayTraceVS";
		mShaderSettingGraphicsTbl[setting].shaderFileNamePS = L"simulateLensFlareDrawing.hlsl";
		mShaderSettingGraphicsTbl[setting].shaderEntryPointPS = L"rayTracePS";
		mShaderSettingGraphicsTbl[setting].nameAtPipeline = "addGhosts";
		mShaderSettingGraphicsTbl[setting].shaderMacroVS.push_back(Shader::DefineMacro{ L"GRID_DIV", to_wstring(mLensFlareComputeInformation.GRID_DIV) });
		mShaderSettingGraphicsTbl[setting].shaderMacroVS.push_back(Shader::DefineMacro{ L"SAMPLE_LAMBDA_NUM", to_wstring(mLensFlareComputeInformation.SAMPLE_LAMBDA_NUM) });
		mShaderSettingGraphicsTbl[setting].shaderMacroVS.push_back(Shader::DefineMacro{ L"NUM_GHOSTS", to_wstring(mLensDescription.NumGhosts) });
		mShaderSettingGraphicsTbl[setting].shaderMacroPS.push_back(Shader::DefineMacro{ L"GRID_DIV", to_wstring(mLensFlareComputeInformation.GRID_DIV) });
		mShaderSettingGraphicsTbl[setting].shaderMacroPS.push_back(Shader::DefineMacro{ L"SAMPLE_LAMBDA_NUM", to_wstring(mLensFlareComputeInformation.SAMPLE_LAMBDA_NUM) });
		mShaderSettingGraphicsTbl[setting].shaderMacroPS.push_back(Shader::DefineMacro{ L"NUM_GHOSTS", to_wstring(mLensDescription.NumGhosts) });
		mShaderSettingGraphicsTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingGraphicsTbl[setting].descriptorKeys.push_back("traceResult");
		mShaderSettingGraphicsTbl[setting].descriptorKeys.push_back("texture");
		mShaderSettingGraphicsTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0 , D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingGraphicsTbl[setting].descriptors["traceResult"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingGraphicsTbl[setting].descriptors["texture"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingGraphicsTbl[setting].rasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		mShaderSettingGraphicsTbl[setting].rasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		mShaderSettingGraphicsTbl[setting].rasterizerState.DepthClipEnable = true;
		setting = ShaderNameGraphics_AddGhostsWireFrame;
		mShaderSettingGraphicsTbl[setting].shaderFileNameVS = L"simulateLensFlareDrawing.hlsl";
		mShaderSettingGraphicsTbl[setting].shaderEntryPointVS = L"rayTraceVS";
		mShaderSettingGraphicsTbl[setting].shaderFileNamePS = L"simulateLensFlareDrawing.hlsl";
		mShaderSettingGraphicsTbl[setting].shaderEntryPointPS = L"rayTracePS";
		mShaderSettingGraphicsTbl[setting].nameAtPipeline = "addGhostsWire";
		mShaderSettingGraphicsTbl[setting].shaderMacroVS.push_back(Shader::DefineMacro{ L"GRID_DIV", to_wstring(mLensFlareComputeInformation.GRID_DIV) });
		mShaderSettingGraphicsTbl[setting].shaderMacroVS.push_back(Shader::DefineMacro{ L"SAMPLE_LAMBDA_NUM", to_wstring(mLensFlareComputeInformation.SAMPLE_LAMBDA_NUM) });
		mShaderSettingGraphicsTbl[setting].shaderMacroVS.push_back(Shader::DefineMacro{ L"NUM_GHOSTS", to_wstring(mLensDescription.NumGhosts) });
		mShaderSettingGraphicsTbl[setting].shaderMacroVS.push_back(Shader::DefineMacro{ L"WIRE_FRAME", to_wstring(1) });
		mShaderSettingGraphicsTbl[setting].shaderMacroPS.push_back(Shader::DefineMacro{ L"GRID_DIV", to_wstring(mLensFlareComputeInformation.GRID_DIV) });
		mShaderSettingGraphicsTbl[setting].shaderMacroPS.push_back(Shader::DefineMacro{ L"SAMPLE_LAMBDA_NUM", to_wstring(mLensFlareComputeInformation.SAMPLE_LAMBDA_NUM) });
		mShaderSettingGraphicsTbl[setting].shaderMacroPS.push_back(Shader::DefineMacro{ L"NUM_GHOSTS", to_wstring(mLensDescription.NumGhosts) });
		mShaderSettingGraphicsTbl[setting].shaderMacroPS.push_back(Shader::DefineMacro{ L"WIRE_FRAME", to_wstring(1) });
		mShaderSettingGraphicsTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingGraphicsTbl[setting].descriptorKeys.push_back("traceResult");
		mShaderSettingGraphicsTbl[setting].descriptorKeys.push_back("texture");
		mShaderSettingGraphicsTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0 , D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingGraphicsTbl[setting].descriptors["traceResult"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingGraphicsTbl[setting].descriptors["texture"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingGraphicsTbl[setting].rasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		mShaderSettingGraphicsTbl[setting].rasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		mShaderSettingGraphicsTbl[setting].rasterizerState.DepthClipEnable = true;
		mShaderSettingGraphicsTbl[setting].rasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		setting = ShaderNameGraphics_AddGhostsUV;
		mShaderSettingGraphicsTbl[setting].shaderFileNameVS = L"simulateLensFlareDrawing.hlsl";
		mShaderSettingGraphicsTbl[setting].shaderEntryPointVS = L"rayTraceVS";
		mShaderSettingGraphicsTbl[setting].shaderFileNamePS = L"simulateLensFlareDrawing.hlsl";
		mShaderSettingGraphicsTbl[setting].shaderEntryPointPS = L"rayTracePS";
		mShaderSettingGraphicsTbl[setting].nameAtPipeline = "addGhostsUV";
		mShaderSettingGraphicsTbl[setting].shaderMacroVS.push_back(Shader::DefineMacro{ L"GRID_DIV", to_wstring(mLensFlareComputeInformation.GRID_DIV) });
		mShaderSettingGraphicsTbl[setting].shaderMacroVS.push_back(Shader::DefineMacro{ L"SAMPLE_LAMBDA_NUM", to_wstring(mLensFlareComputeInformation.SAMPLE_LAMBDA_NUM) });
		mShaderSettingGraphicsTbl[setting].shaderMacroVS.push_back(Shader::DefineMacro{ L"NUM_GHOSTS", to_wstring(mLensDescription.NumGhosts) });
		mShaderSettingGraphicsTbl[setting].shaderMacroVS.push_back(Shader::DefineMacro{ L"UV", to_wstring(1) });
		mShaderSettingGraphicsTbl[setting].shaderMacroPS.push_back(Shader::DefineMacro{ L"GRID_DIV", to_wstring(mLensFlareComputeInformation.GRID_DIV) });
		mShaderSettingGraphicsTbl[setting].shaderMacroPS.push_back(Shader::DefineMacro{ L"SAMPLE_LAMBDA_NUM", to_wstring(mLensFlareComputeInformation.SAMPLE_LAMBDA_NUM) });
		mShaderSettingGraphicsTbl[setting].shaderMacroPS.push_back(Shader::DefineMacro{ L"NUM_GHOSTS", to_wstring(mLensDescription.NumGhosts) });
		mShaderSettingGraphicsTbl[setting].shaderMacroPS.push_back(Shader::DefineMacro{ L"UV", to_wstring(1) });
		mShaderSettingGraphicsTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingGraphicsTbl[setting].descriptorKeys.push_back("traceResult");
		mShaderSettingGraphicsTbl[setting].descriptorKeys.push_back("texture");
		mShaderSettingGraphicsTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0 , D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingGraphicsTbl[setting].descriptors["traceResult"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingGraphicsTbl[setting].descriptors["texture"] = DescriptorInfo{ 2, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1} };
		mShaderSettingGraphicsTbl[setting].rasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		mShaderSettingGraphicsTbl[setting].rasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		mShaderSettingGraphicsTbl[setting].rasterizerState.DepthClipEnable = true;
		mShaderSettingGraphicsTbl[setting].rasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	}

	{
		setting = SahderNameGraphics_AddStarBurst;
		mShaderSettingGraphicsTbl[setting].shaderFileNameVS = L"simulateLensFlareDrawBurst.hlsl";
		mShaderSettingGraphicsTbl[setting].shaderEntryPointVS = L"starburstVS";
		mShaderSettingGraphicsTbl[setting].shaderFileNamePS = L"simulateLensFlareDrawBurst.hlsl";
		mShaderSettingGraphicsTbl[setting].shaderEntryPointPS = L"starburstPS";
		mShaderSettingGraphicsTbl[setting].nameAtPipeline = "addStarBurst";
		mShaderSettingGraphicsTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingGraphicsTbl[setting].descriptorKeys.push_back("texture");
		mShaderSettingGraphicsTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0 , D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingGraphicsTbl[setting].descriptors["texture"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingGraphicsTbl[setting].rasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		mShaderSettingGraphicsTbl[setting].rasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		mShaderSettingGraphicsTbl[setting].rasterizerState.DepthClipEnable = false;
	}

	{
		setting = ShaderNameGraphics_ToneMapper;
		mShaderSettingGraphicsTbl[setting].shaderFileNameVS = L"simulateLensFlareToneMapper.hlsl";
		mShaderSettingGraphicsTbl[setting].shaderEntryPointVS = L"VS";
		mShaderSettingGraphicsTbl[setting].shaderFileNamePS = L"simulateLensFlareToneMapper.hlsl";
		mShaderSettingGraphicsTbl[setting].shaderEntryPointPS = L"PS";
		mShaderSettingGraphicsTbl[setting].nameAtPipeline = "toneMapper";
		mShaderSettingGraphicsTbl[setting].descriptorKeys.push_back("computeConstants");
		mShaderSettingGraphicsTbl[setting].descriptorKeys.push_back("texture");
		mShaderSettingGraphicsTbl[setting].descriptors["computeConstants"] = DescriptorInfo{ 0 , D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0} };
		mShaderSettingGraphicsTbl[setting].descriptors["texture"] = DescriptorInfo{ 1, D3D12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0} };
		mShaderSettingGraphicsTbl[setting].imputElements.push_back(D3D12_INPUT_ELEMENT_DESC{ "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		mShaderSettingGraphicsTbl[setting].imputElements.push_back(D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		mShaderSettingGraphicsTbl[setting].rasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		mShaderSettingGraphicsTbl[setting].rasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		mShaderSettingGraphicsTbl[setting].rasterizerState.DepthClipEnable = false;
	}

	vector<CD3DX12_STATIC_SAMPLER_DESC> samplerDesc(1);
	samplerDesc[0].Init(
		0,
		D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		0,
		16,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
		0.f,
		D3D12_FLOAT32_MAX,
		D3D12_SHADER_VISIBILITY_ALL,
		0);

	setupGraphicsPipelineAndSignature(samplerDesc, false, true);
}