#include "../include/PBLensFlare.h"

using namespace std;
using namespace DirectX;

void PBLensFlare::render()
{
	mFrameIndex = mSwapchain->getCurrentBackBufferIndex();
	mCommandAllocators[mFrameIndex]->Reset();
	mCommandList->Reset(
		mCommandAllocators[mFrameIndex].Get(), nullptr
	);

	// status : swapchain rendering ok 2 render target
	{
		auto barrierToRT = mSwapchain->getBarrierPresent2RT();
		mCommandList->ResourceBarrier(1, &barrierToRT);
	}

	ID3D12DescriptorHeap* heaps[] = { mHeap->getHeap().Get() };
	mCommandList->SetDescriptorHeaps(_countof(heaps), heaps);

	draw();

	renderHUD();

	// status :  render target 2  swapchain rendering ok
	{
		auto barrierToPresent = mSwapchain->getBarrierRT2Present();
		CD3DX12_RESOURCE_BARRIER barriers[] = {
		  barrierToPresent,
		};

		mCommandList->ResourceBarrier(_countof(barriers), barriers);
	}

	mCommandList->Close();

	LARGE_INTEGER freq;
	LARGE_INTEGER start;
	LARGE_INTEGER end;

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);

	ID3D12CommandList* lists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(1, lists);

	buttomAction();

	// mSwapchain->Present(1, 0);
	mSwapchain->present(0, DXGI_PRESENT_ALLOW_TEARING);
	mSwapchain->waitPreviousFrame(mCommandQueue, mFrameIndex, GpuWaitTimeout);

	QueryPerformanceCounter(&end);

	mExecuteTimeMS = 1000.0 * ((f64)end.QuadPart - start.QuadPart) / freq.QuadPart;
}

void PBLensFlare::draw()
{
	executeRayTracingLensFlareCommand();
}