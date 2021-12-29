#include "../common/include/Swapchain.h"

Swapchain::Swapchain(
	ComPtr<IDXGISwapChain1> swapchain,
	std::shared_ptr<DescriptorManager>& heapRTV,
	bool useHDR)
{
	swapchain.As(&mSwapchain); // IDXGISwapChain4
	mSwapchain->GetDesc1(&mDesc);

	ComPtr<ID3D12Device> device;
	swapchain->GetDevice(IID_PPV_ARGS(&device));

	mImages.resize(mDesc.BufferCount);
	mImageRTV.resize(mDesc.BufferCount);
	mFences.resize(mDesc.BufferCount);
	mFenceValues.resize(mDesc.BufferCount);
	mWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	HRESULT hr;
	for (UINT i = 0; i < mDesc.BufferCount; ++i)
	{
		hr = device->CreateFence(
			0, D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&mFences[i]));
		ThrowIfFailed(hr, "CreateFence Failed");

		mImageRTV[i] = heapRTV->alloc();

		// generate image rtv
		hr = mSwapchain->GetBuffer(i, IID_PPV_ARGS(&mImages[i]));
		ThrowIfFailed(hr, "GetBuffer() Failed");
		device->CreateRenderTargetView(mImages[i].Get(), nullptr, mImageRTV[i]);
	}

	// set color space
	DXGI_COLOR_SPACE_TYPE colorSpace;
	switch (mDesc.Format)
	{
	default:
		colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
		break;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
		break;
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
		break;
	}
	mSwapchain->SetColorSpace1(colorSpace);

	if (useHDR)
	{
		setMetadata();
	}
}

Swapchain::~Swapchain() {
	BOOL isFullScreen;
	mSwapchain->GetFullscreenState(&isFullScreen, nullptr);
	if (isFullScreen)
	{
		mSwapchain->SetFullscreenState(FALSE, nullptr);
	}
	CloseHandle(mWaitEvent);
}

DescriptorHandle Swapchain::getCurrentRTV() const
{
	return mImageRTV[getCurrentBackBufferIndex()];
}

HRESULT Swapchain::present(UINT SyncInterval, UINT Flags)
{
	return mSwapchain->Present(SyncInterval, Flags);
}


void Swapchain::waitPreviousFrame(ComPtr<ID3D12CommandQueue> commandQueue, int frameIndex, DWORD timeout)
{
	frameIndex = 0;

	auto fence = mFences[frameIndex];
	auto value = ++mFenceValues[frameIndex];
	commandQueue->Signal(fence.Get(), value);

	// wait.
	auto nextIndex = getCurrentBackBufferIndex();
	if (value - mFrame >= 0)
	{
		fence->SetEventOnCompletion(mFrame, mWaitEvent);
		WaitForSingleObject(mWaitEvent, timeout);
		mFrame++;
	}
}

void Swapchain::resizeBuffers(UINT width, UINT height)
{
	for (auto& v : mImages) {
		v.Reset();
	}
	HRESULT hr = mSwapchain->ResizeBuffers(
		mDesc.BufferCount,
		width, height, mDesc.Format, mDesc.Flags
	);
	ThrowIfFailed(hr, "ResizeBuffers Failed");

	ComPtr<ID3D12Device> device;
	mSwapchain->GetDevice(IID_PPV_ARGS(&device));
	for (UINT i = 0; i < mDesc.BufferCount; ++i) {
		mSwapchain->GetBuffer(i, IID_PPV_ARGS(&mImages[i]));
		device->CreateRenderTargetView(
			mImages[i].Get(),
			nullptr,
			mImageRTV[i]);
	}
}

CD3DX12_RESOURCE_BARRIER Swapchain::getBarrierPresent2RT()
{
	return CD3DX12_RESOURCE_BARRIER::Transition(
		mImages[getCurrentBackBufferIndex()].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
}
CD3DX12_RESOURCE_BARRIER Swapchain::getBarrierRT2Present()
{
	return CD3DX12_RESOURCE_BARRIER::Transition(
		mImages[getCurrentBackBufferIndex()].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
}


void Swapchain::setMetadata()
{
	struct DisplayChromacities
	{
		float RedX;
		float RedY;
		float GreenX;
		float GreenY;
		float BlueX;
		float BlueY;
		float WhiteX;
		float WhiteY;
	} DisplayChromacityList[] = {
	  { 0.64000f, 0.33000f, 0.30000f, 0.60000f, 0.15000f, 0.06000f, 0.31270f, 0.32900f }, // Rec709 
	  { 0.70800f, 0.29200f, 0.17000f, 0.79700f, 0.13100f, 0.04600f, 0.31270f, 0.32900f }, // Rec2020
	};
	int useIndex = 0;
	if (mDesc.Format == DXGI_FORMAT_R16G16B16A16_FLOAT)
	{
		useIndex = 1;
	}
	const auto& chroma = DisplayChromacityList[useIndex];
	DXGI_HDR_METADATA_HDR10 HDR10MetaData{};
	HDR10MetaData.RedPrimary[0] = UINT16(chroma.RedX * 50000.0f);
	HDR10MetaData.RedPrimary[1] = UINT16(chroma.RedY * 50000.0f);
	HDR10MetaData.GreenPrimary[0] = UINT16(chroma.GreenX * 50000.0f);
	HDR10MetaData.GreenPrimary[1] = UINT16(chroma.GreenY * 50000.0f);
	HDR10MetaData.BluePrimary[0] = UINT16(chroma.BlueX * 50000.0f);
	HDR10MetaData.BluePrimary[1] = UINT16(chroma.BlueY * 50000.0f);
	HDR10MetaData.WhitePoint[0] = UINT16(chroma.WhiteX * 50000.0f);
	HDR10MetaData.WhitePoint[1] = UINT16(chroma.WhiteY * 50000.0f);
	HDR10MetaData.MaxMasteringLuminance = UINT(1000.0f * 10000.0f);
	HDR10MetaData.MinMasteringLuminance = UINT(0.001f * 10000.0f);
	HDR10MetaData.MaxContentLightLevel = UINT16(2000.0f);
	HDR10MetaData.MaxFrameAverageLightLevel = UINT16(500.0f);
	mSwapchain->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(HDR10MetaData), &HDR10MetaData);
}

bool Swapchain::isFullScreen() const
{
	BOOL fullscreen;
	if (FAILED(mSwapchain->GetFullscreenState(&fullscreen, nullptr)))
	{
		fullscreen = FALSE;
	}
	return fullscreen == TRUE;
}
void Swapchain::resizeTarget(const DXGI_MODE_DESC* pNewTargetParameters)
{
	mSwapchain->ResizeTarget(pNewTargetParameters);
}
void Swapchain::setFullScreen(bool toFullScreen)
{
	if (toFullScreen)
	{
		ComPtr<IDXGIOutput> output;
		mSwapchain->GetContainingOutput(&output);
		if (output)
		{
			DXGI_OUTPUT_DESC desc{};
			output->GetDesc(&desc);
		}

		mSwapchain->SetFullscreenState(TRUE, /*output.Get()*/nullptr);
	}
	else
	{
		mSwapchain->SetFullscreenState(FALSE, nullptr);
	}
}