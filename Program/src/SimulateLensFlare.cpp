#include "../include/PBLensFlare.h"
using namespace std;

void PBLensFlare::executeRayTracingLensFlareCommand()
{
	auto rtv = mSwapchain->getCurrentRTV();
	auto dsv = mDefaultDepthDSV;
	setupDrawing(rtv, dsv);
	updateBuffer();

	if (mErrorShaderTbl.size() != 0)
	{
		return;
	}

	if (mBurstKernelRegenerate)
	{
		generateBurst();
	}
	if (mGhostKernelRegenerate)
	{
		generateGhost();
	}

	auto judge = [this]() -> bool { return mGhostIntensityRatio && mSpread > 0 && (mColor.x + mColor.y + mColor.z > 1e-4); } ();

	if (mTraceRequired && judge)
	{
		traceRay();
		mTraceRequired = false;
	}

	if (judge)
	{
		drawLensFlare();
	}

	addBurst();
}

void PBLensFlare::drawLensFlare()
{
	switch (mDrawMode)
	{
	case DrawMode_Mesh:
	{
		addGhosts(false);
	}
	break;
	case DrawMode_Wire:
	{
		addGhosts(true);
	}
	break;
	case DrawMode_OverrideWire:
	{
		addGhosts();
		addGhosts(true);
	}
	break;
	case DrawMode_UV:
	{
		addGhostsUV();
	}
	break;
	default:
		break;
	}
}

void PBLensFlare::setupDrawing(DescriptorHandle rtv, DescriptorHandle dsv)
{
	f32 clear[4] = { 0,0,0,0 };
	mCommandList->ClearRenderTargetView(rtv, clear, 0, nullptr);
	mCommandList->ClearDepthStencilView(
		dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	auto d3d12CpuDescHandleRtv = (D3D12_CPU_DESCRIPTOR_HANDLE)rtv;
	auto d3d12CpuDescHandleDsv = (D3D12_CPU_DESCRIPTOR_HANDLE)dsv;
	mCommandList->OMSetRenderTargets(1, &d3d12CpuDescHandleRtv,
		FALSE, &d3d12CpuDescHandleDsv);
	auto viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, float(mWidth), float(mHeight));
	auto scissorRect = CD3DX12_RECT(0, 0, LONG(mWidth), LONG(mHeight));
	mCommandList->RSSetViewports(1, &viewport);
	mCommandList->RSSetScissorRects(1, &scissorRect);
	const FLOAT blendFactor = 1.0f;
	mCommandList->OMSetBlendFactor(&blendFactor);
}

void PBLensFlare::traceRay()
{
	PIXBeginEvent(mCommandList.Get(), 0, "traceRay");
	const u32 pass = mUseAR ? ShaderNameCompute_TraceRayAR : ShaderNameCompute_TraceRay;
	mCommandList->SetComputeRootSignature(mSignatureTbl[mShaderSettingComputeTbl[pass].nameAtPipeline].Get());
	mCommandList->SetComputeRootConstantBufferView(mShaderSettingComputeTbl[pass].descriptors["computeConstants"].rootParamIndex, mTracingCB[0]->GetGPUVirtualAddress());
	mCommandList->SetComputeRootConstantBufferView(mShaderSettingComputeTbl[pass].descriptors["lensBank"].rootParamIndex, mLensBank[0]->GetGPUVirtualAddress());
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[pass].descriptors["traceResult"].rootParamIndex, mRayBundle.vertexBuffer.getUAV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[pass].descriptors["lensInterface"].rootParamIndex, mLensInterfaceBuffer.getSRV(mCommandList));
	mCommandList->SetComputeRootDescriptorTable(mShaderSettingComputeTbl[pass].descriptors["ghostData"].rootParamIndex, mGhostDataBuffer.getSRV(mCommandList));
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingComputeTbl[pass].nameAtPipeline].Get());
	//0 <= groupID.x < NumGhosts * NUM_GROUPS
	//0 <= groupID.y < NUM_GROUPS
	//0 <= groupID.z < SAMPLE_LAMBDA_NUM
	mCommandList->Dispatch((unsigned)mLensDescription.NumGhosts * mLensFlareComputeInformation.NUM_GROUPS, (unsigned)mLensFlareComputeInformation.NUM_GROUPS, mLensFlareComputeInformation.SAMPLE_LAMBDA_NUM);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::addGhosts(bool wireFrame)
{
	PIXBeginEvent(mCommandList.Get(), 0, wireFrame ? "addGhostsWire" : "addGhosts");
	u32 pass = wireFrame ? ShaderNameGraphics_AddGhostsWireFrame : ShaderNameGraphics_AddGhosts;
	mCommandList->SetGraphicsRootSignature(mSignatureTbl[mShaderSettingGraphicsTbl[pass].nameAtPipeline].Get());
	mCommandList->SetGraphicsRootConstantBufferView(mShaderSettingGraphicsTbl[pass].descriptors["computeConstants"].rootParamIndex, mDrawingCB[0]->GetGPUVirtualAddress());
	mCommandList->SetGraphicsRootDescriptorTable(mShaderSettingGraphicsTbl[pass].descriptors["texture"].rootParamIndex, mGhostCachedTex.getSRV(mCommandList));
	mCommandList->SetGraphicsRootDescriptorTable(mShaderSettingGraphicsTbl[pass].descriptors["traceResult"].rootParamIndex, mRayBundle.vertexBuffer.getSRV(mCommandList));
	mCommandList->IASetIndexBuffer(&mRayBundle.indexBufferView);
	mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingGraphicsTbl[pass].nameAtPipeline].Get());
	mCommandList->DrawIndexedInstanced((unsigned)mLensFlareComputeInformation.NUM_VERTICES_PER_BUNDLES * 3 * 2, mLensDescription.NumGhosts, 0, 0, 0);
	//mCommandList->DrawIndexedInstanced(1, 1, 0, 0, 0);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::addGhostsUV()
{
	PIXBeginEvent(mCommandList.Get(), 0, "addGhostsUV");
	const u32 pass = ShaderNameGraphics_AddGhostsUV;
	mCommandList->SetGraphicsRootSignature(mSignatureTbl[mShaderSettingGraphicsTbl[pass].nameAtPipeline].Get());
	mCommandList->SetGraphicsRootConstantBufferView(mShaderSettingGraphicsTbl[pass].descriptors["computeConstants"].rootParamIndex, mDrawingCB[0]->GetGPUVirtualAddress());
	mCommandList->SetGraphicsRootDescriptorTable(mShaderSettingGraphicsTbl[pass].descriptors["texture"].rootParamIndex, mGhostCachedTex.getSRV(mCommandList));
	mCommandList->SetGraphicsRootDescriptorTable(mShaderSettingGraphicsTbl[pass].descriptors["traceResult"].rootParamIndex, mRayBundle.vertexBuffer.getSRV(mCommandList));
	mCommandList->IASetIndexBuffer(&mRayBundle.indexBufferView);
	mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingGraphicsTbl[pass].nameAtPipeline].Get());
	mCommandList->DrawIndexedInstanced((unsigned)mLensFlareComputeInformation.NUM_VERTICES_PER_BUNDLES * 3 * 2, mLensDescription.NumGhosts, 0, 0, 0);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::addBurst()
{
	PIXBeginEvent(mCommandList.Get(), 0, "addStarBurst");
	const u32 pass = SahderNameGraphics_AddStarBurst;
	mCommandList->SetGraphicsRootSignature(mSignatureTbl[mShaderSettingGraphicsTbl[pass].nameAtPipeline].Get());
	mCommandList->SetGraphicsRootConstantBufferView(mShaderSettingGraphicsTbl[pass].descriptors["computeConstants"].rootParamIndex, mDrawBurstCB[0]->GetGPUVirtualAddress());
	mCommandList->SetGraphicsRootDescriptorTable(mShaderSettingGraphicsTbl[pass].descriptors["texture"].rootParamIndex, mBurstCachedTex.getSRV(mCommandList));
	mCommandList->IASetIndexBuffer(&mStarBurstQuad.ibView);
	mCommandList->IASetVertexBuffers(0, 1, &mStarBurstQuad.vbView);
	mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	mCommandList->SetPipelineState(mPipelineStateTbl[mShaderSettingGraphicsTbl[pass].nameAtPipeline].Get());
	mCommandList->DrawIndexedInstanced(4, 1, 0, 0, 0);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::setupSimulateLensFlarePipeline()
{
	mErrorShaderTbl.resize(0);
	setupComputePipeline();
	setupGraphicsPipeline();
}