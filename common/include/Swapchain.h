#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>

#include <wrl.h>
#include <vector>
#include <memory>

#include "DescriptorManager.h"
#include "AppUtil.h"

class Swapchain
{
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	Swapchain(
		ComPtr<IDXGISwapChain1> swapchain,
		std::shared_ptr<DescriptorManager>& heapRTV,
		bool useHDR = false);

	~Swapchain();

	UINT getCurrentBackBufferIndex() const {
		return mSwapchain->GetCurrentBackBufferIndex();
	}
	DescriptorHandle getCurrentRTV() const;
	ComPtr<ID3D12Resource1> getImage(UINT index) { return mImages[index]; }

	HRESULT present(UINT SyncInterval, UINT Flags);

	// wait until app can stack next command
	void waitPreviousFrame(
		ComPtr<ID3D12CommandQueue> commandQueue,
		int frameIndex, DWORD timeout);

	void resizeBuffers(UINT width, UINT height);

	CD3DX12_RESOURCE_BARRIER getBarrierPresent2RT();
	CD3DX12_RESOURCE_BARRIER getBarrierRT2Present();

	DXGI_FORMAT getFormat() const { return mDesc.Format; }

	bool isFullScreen() const;
	void setFullScreen(bool toFullScreen);
	void resizeTarget(const DXGI_MODE_DESC* pNewTargetParameters);
private:
	void setMetadata();

private:
	ComPtr<IDXGISwapChain4> mSwapchain;
	std::vector<ComPtr<ID3D12Resource1>> mImages;
	std::vector<DescriptorHandle> mImageRTV;

	std::vector<UINT64> mFenceValues;
	std::vector<ComPtr<ID3D12Fence1>> mFences;

	DXGI_SWAP_CHAIN_DESC1 mDesc;

	HANDLE mWaitEvent;

	UINT mFrame = 1;
};