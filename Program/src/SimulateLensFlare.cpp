#include "../include/PBLensFlare.h"
using namespace std;

void PBLensFlare::executeRayTracingLensFlareCommand()
{
	auto rtv = mSwapchain->getCurrentRTV();
	auto dsv = mDefaultDepthDSV;
	setupDrawing(rtv, dsv);
	updateBuffers();//

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

	if (//mTraceRequired && judge
		 true
		)
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
	auto viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, mScreenSize.w, mScreenSize.h);
	auto scissorRect = CD3DX12_RECT(0, 0, mScreenSize.w, mScreenSize.h);
	mCommandList->RSSetViewports(1, &viewport);
	mCommandList->RSSetScissorRects(1, &scissorRect);
	const FLOAT blendFactor = 1.0f;
	mCommandList->OMSetBlendFactor(&blendFactor);
}

void PBLensFlare::traceRay()
{
	PIXBeginEvent(mCommandList.Get(), 0, "traceRay");
	u32 state = 0;
	if (mUseAR)
	{
		if (mDebug)
		{
			state = ShaderNameCompute_TraceRayAR_DEBUG;
		}
		else
		{
			state = ShaderNameCompute_TraceRayAR;
		}
	}
	else
	{
		if (mDebug)
		{
			state = ShaderNameCompute_TraceRay_DEBUG;
		}
		else
		{
			state = ShaderNameCompute_TraceRay;
		}
	}
	setPipelineState(PipelineType_Compute, state);
	setPipelineConstantResource("computeConstants", mTracingCB[0]->GetGPUVirtualAddress());
	setPipelineConstantResource("lensBank", mLensBank[0]->GetGPUVirtualAddress());
	setPipelineResource("traceResult", mRayBundle.vertexBuffer.getUAV(mCommandList));
	setPipelineResource("lensInterface", mLensInterfaceBuffer.getSRV(mCommandList));
	setPipelineResource("ghostData", mGhostDataBuffer.getSRV(mCommandList));
	dispatch((unsigned)mLensDescription.NumGhosts * mLensFlareComputeInformation.NUM_GROUPS, (unsigned)mLensFlareComputeInformation.NUM_GROUPS, mLensFlareComputeInformation.SAMPLE_LAMBDA_NUM);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::addGhosts(bool wireFrame)
{
	PIXBeginEvent(mCommandList.Get(), 0, wireFrame ? "addGhostsWire" : "addGhosts");
	setPipelineState(PipelineType_Graphics, wireFrame ? ShaderNameGraphics_AddGhostsWireFrame : ShaderNameGraphics_AddGhosts);
	setPipelineConstantResource("computeConstants", mDrawingCB[0]->GetGPUVirtualAddress());
	setPipelineResource("texture", mGhostCachedTex.getSRV(mCommandList));
	setPipelineResource("traceResult", mRayBundle.vertexBuffer.getSRV(mCommandList));
	setIndexBuffer(&mRayBundle.indexBufferView);
	setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	drawIndexedInstanced((unsigned)mLensFlareComputeInformation.NUM_VERTICES_PER_BUNDLES * 3 * 2, mLensDescription.NumGhosts, 0, 0, 0);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::addGhostsUV()
{
	PIXBeginEvent(mCommandList.Get(), 0, "addGhostsUV");
	setPipelineState(PipelineType_Graphics, ShaderNameGraphics_AddGhostsUV);
	setPipelineConstantResource("computeConstants", mDrawingCB[0]->GetGPUVirtualAddress());
	setPipelineResource("texture", mGhostCachedTex.getSRV(mCommandList));
	setPipelineResource("traceResult", mRayBundle.vertexBuffer.getSRV(mCommandList));
	setIndexBuffer(&mRayBundle.indexBufferView);
	setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	drawIndexedInstanced((unsigned)mLensFlareComputeInformation.NUM_VERTICES_PER_BUNDLES * 3 * 2, mLensDescription.NumGhosts, 0, 0, 0);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::addBurst()
{
	PIXBeginEvent(mCommandList.Get(), 0, "addStarBurst");
	setPipelineState(PipelineType_Graphics, SahderNameGraphics_AddStarBurst);
	setPipelineConstantResource("computeConstants", mDrawBurstCB[0]->GetGPUVirtualAddress());
	setPipelineResource("texture", mBurstCachedTex.getSRV(mCommandList));
	setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	drawInstanced(3, 1, 0, 0);
	PIXEndEvent(mCommandList.Get());
}

void PBLensFlare::setupSimulateLensFlarePipeline()
{
	mErrorShaderTbl.resize(0);
	setupComputePipeline();
	setupGraphicsPipeline();
}