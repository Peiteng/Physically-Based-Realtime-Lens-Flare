#include "../common/include/Application.h"
#include <exception>
#include <fstream>

#include "../common//include/AppUtil.h"

//IMGUI
#include "../imgui/imgui.h"
#include "../imgui/examples/imgui_impl_dx12.h"
#include "../imgui/examples/imgui_impl_win32.h"

using namespace std;
using namespace Microsoft::WRL;

Application::Application()
{
	mFrameIndex = 0;
	mWaitFence = CreateEvent(NULL, FALSE, FALSE, NULL);
}


Application::~Application()
{
	CloseHandle(mWaitFence);
}

void Application::setTitle(const std::string& title)
{
	SetWindowTextA(mHwnd, title.c_str());
}

void Application::initializeApp(HWND hwnd, DXGI_FORMAT format, bool isFullscreen)
{
	mHwnd = hwnd;
	HRESULT hr;
	UINT dxgiFlags = 0;
#if defined(_DEBUG)
	ComPtr<ID3D12Debug> debug;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
	{
		debug->EnableDebugLayer();
		dxgiFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif
	ComPtr<IDXGIFactory5> factory;
	hr = CreateDXGIFactory2(dxgiFlags, IID_PPV_ARGS(&factory));
	ThrowIfFailed(hr, "CreateDXGIFactory2 失敗");

	// search hardware adapter
	ComPtr<IDXGIAdapter1> useAdapter;
	{
		UINT adapterIndex = 0;
		ComPtr<IDXGIAdapter1> adapter;
		while (DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter))
		{
			DXGI_ADAPTER_DESC1 desc1{};
			adapter->GetDesc1(&desc1);
			++adapterIndex;
			if (desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			// is D3D12 enable?
			hr = D3D12CreateDevice(
				adapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				__uuidof(ID3D12Device), nullptr);
			if (SUCCEEDED(hr))
				break;
		}
		adapter.As(&useAdapter);
	}

	hr = D3D12CreateDevice(useAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice));
	ThrowIfFailed(hr, "D3D12CreateDevice 失敗");

	//construct command queue
	D3D12_COMMAND_QUEUE_DESC queueDesc{
	  D3D12_COMMAND_LIST_TYPE_DIRECT,
	  0,
	  D3D12_COMMAND_QUEUE_FLAG_NONE,
	  0
	};
	hr = mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue));
	ThrowIfFailed(hr, "CreateCommandQueue 失敗");

	prepareDescriptorHeaps();

	//determine client region size fro HWND
	RECT rect;
	GetClientRect(hwnd, &rect);
	mWidth = rect.right - rect.left;
	mHeight = rect.bottom - rect.top;

	bool useHDR = format == DXGI_FORMAT_R16G16B16A16_FLOAT || format == DXGI_FORMAT_R10G10B10A2_UNORM;
	if (useHDR)
	{
		bool isDisplayHDR10 = false;
		UINT index = 0;
		ComPtr<IDXGIOutput> current;
		while (useAdapter->EnumOutputs(index, &current) != DXGI_ERROR_NOT_FOUND)
		{
			ComPtr<IDXGIOutput6> output6;
			current.As(&output6);

			DXGI_OUTPUT_DESC1 desc;
			output6->GetDesc1(&desc);
			isDisplayHDR10 |= desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
			++index;
		}

		if (!isDisplayHDR10)
		{
			format = DXGI_FORMAT_R8G8B8A8_UNORM;
			useHDR = false;
		}
	}

	BOOL allowTearing = FALSE;
	hr = factory->CheckFeatureSupport(
		DXGI_FEATURE_PRESENT_ALLOW_TEARING,
		&allowTearing,
		sizeof(allowTearing)
	);
	mIsAllowTearing = SUCCEEDED(hr) && allowTearing;

	// construct swapchain
	{
		DXGI_SWAP_CHAIN_DESC1 scDesc{};
		scDesc.BufferCount = FrameBufferCount;
		scDesc.Width = mWidth;
		scDesc.Height = mHeight;
		scDesc.Format = format;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		// scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		scDesc.SampleDesc.Count = 1;
		//scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;// if u wanna change display resolution, do uncomment
		scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;// if u wanna change display resolution, do uncomment

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsDesc{};
		fsDesc.Windowed = isFullscreen ? FALSE : TRUE;
		fsDesc.RefreshRate.Denominator = 1000;
		fsDesc.RefreshRate.Numerator = 60317;
		fsDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		fsDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;

		ComPtr<IDXGISwapChain1> swapchain;
		hr = factory->CreateSwapChainForHwnd(
			mCommandQueue.Get(),
			hwnd,
			&scDesc,
			&fsDesc,
			nullptr,
			&swapchain);
		ThrowIfFailed(hr, "CreateSwapChainForHwnd 失敗");
		mSwapchain = std::make_shared<Swapchain>(swapchain, mHeapRTV);
	}

	factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
	mSurfaceFormat = mSwapchain->getFormat();

	createDefaultDepthBuffer(mWidth, mHeight);

	createCommandAllocators();

	hr = mDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		mCommandAllocators[0].Get(),
		nullptr,
		IID_PPV_ARGS(&mCommandList)
	);
	ThrowIfFailed(hr, "CreateCommandList 失敗");
	mCommandList->Close();

	mViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, float(mWidth), float(mHeight));
	mScissorRect = CD3DX12_RECT(0, 0, LONG(mWidth), LONG(mHeight));

	prepare();

	prepareImGui();
}

void Application::terminate()
{
	waitForIdleGPU();
	cleanup();

	cleanupImGui();
}


void Application::render()
{
	mFrameIndex = mSwapchain->getCurrentBackBufferIndex();

	mCommandAllocators[mFrameIndex]->Reset();
	mCommandList->Reset(
		mCommandAllocators[mFrameIndex].Get(),
		nullptr
	);

	// status : swapchain rendering ok 2 render target
	auto barrierToRT = mSwapchain->getBarrierPresent2RT();
	mCommandList->ResourceBarrier(1, &barrierToRT);

	auto rtv = mSwapchain->getCurrentRTV();
	auto dsv = mDefaultDepthDSV;

	const float clearColor[] = { 0.5f,0.75f,1.0f,0.0f };
	mCommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);

	mCommandList->ClearDepthStencilView(
		dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	auto d3d12_cpu_desc_handle_rtv = (D3D12_CPU_DESCRIPTOR_HANDLE)rtv;
	auto d3d12_cpu_desc_handle_dsv = (D3D12_CPU_DESCRIPTOR_HANDLE)dsv;

	mCommandList->OMSetRenderTargets(1, &d3d12_cpu_desc_handle_rtv,
		FALSE, &d3d12_cpu_desc_handle_dsv);

	ID3D12DescriptorHeap* heaps[] = { mHeap->getHeap().Get() };
	mCommandList->SetDescriptorHeaps(_countof(heaps), heaps);

	// status :  render target 2  swapchain rendering ok
	auto barrierToPresent = mSwapchain->getBarrierRT2Present();
	mCommandList->ResourceBarrier(1, &barrierToPresent);

	mCommandList->Close();

	ID3D12CommandList* lists[] = { mCommandList.Get() };

	mCommandQueue->ExecuteCommandLists(1, lists);

	mSwapchain->present(0, DXGI_PRESENT_ALLOW_TEARING);
	mSwapchain->waitPreviousFrame(mCommandQueue, mFrameIndex, GpuWaitTimeout);

}

Application::ComPtr<ID3D12Resource1> Application::createResource(
	const CD3DX12_RESOURCE_DESC& desc,
	D3D12_RESOURCE_STATES resourceStates,
	const D3D12_CLEAR_VALUE* clearValue,
	D3D12_HEAP_TYPE heapType)
{
	HRESULT hr;
	ComPtr<ID3D12Resource1> ret;

	auto cd3dx12_heap_properties = CD3DX12_HEAP_PROPERTIES(heapType);

	hr = mDevice->CreateCommittedResource(
		&cd3dx12_heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		resourceStates,
		clearValue,
		IID_PPV_ARGS(&ret)
	);
	ThrowIfFailed(hr, "CreateCommittedResource Failed.");
	return ret;
}

std::vector<ComPtr<ID3D12Resource1>> Application::createConstantBuffers(const CD3DX12_RESOURCE_DESC& desc, int count)
{
	vector<ComPtr<ID3D12Resource1>> buffers;
	for (int i = 0; i < count; ++i)
	{
		buffers.emplace_back(
			createResource(desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, D3D12_HEAP_TYPE_UPLOAD)
		);
	}
	return buffers;
}


Application::ComPtr<ID3D12GraphicsCommandList> Application::createCommandList()
{
	HRESULT hr;
	ComPtr<ID3D12GraphicsCommandList> command;
	hr = mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mOneshotCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&command));
	ThrowIfFailed(hr, "CreateCommandList(OneShot) Failed.");
	command->SetName(L"OneShotCommand");

	return command;
}

void Application::finishCommandList(ComPtr<ID3D12GraphicsCommandList>& command)
{
	ID3D12CommandList* commandList[] = {
	  command.Get()
	};
	command->Close();
	mCommandQueue->ExecuteCommandLists(1, commandList);
	HRESULT hr;
	ComPtr<ID3D12Fence1> fence;
	hr = mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	ThrowIfFailed(hr, "CreateFence Failed.");
	const UINT64 expectValue = 1;
	mCommandQueue->Signal(fence.Get(), expectValue);
	do
	{
	} while (fence->GetCompletedValue() != expectValue);
	mOneshotCommandAllocator->Reset();
}

ComPtr<ID3D12GraphicsCommandList> Application::createBundleCommandList()
{
	ComPtr<ID3D12GraphicsCommandList> command;
	mDevice->CreateCommandList(
		0, D3D12_COMMAND_LIST_TYPE_BUNDLE,
		mBundleCommandAllocator.Get(),
		nullptr, IID_PPV_ARGS(&command)
	);
	return command;
}

void Application::writeToUploadHeapMemory(ID3D12Resource1* resource, uint32_t size, const void* data)
{
	void* mapped;
	HRESULT hr = resource->Map(0, nullptr, &mapped);
	if (SUCCEEDED(hr))
	{
		memcpy(mapped, data, size);
		resource->Unmap(0, nullptr);
	}
	else
	{
		hr = mDevice->GetDeviceRemovedReason();
		MessageBox(NULL, TEXT("Map Failed."), TEXT("ERROR"), MB_OK | MB_ICONINFORMATION);

		ThrowIfFailed(hr, "Map Failed.");
	}
}

void Application::prepareDescriptorHeaps()
{
	const int MaxDescriptorCount = 2048; // SRV,CBV,UAV etc.
	const int MaxDescriptorCountRTV = 100;
	const int MaxDescriptorCountDSV = 100;

	D3D12_DESCRIPTOR_HEAP_DESC heapDescRTV{
	  D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
	  MaxDescriptorCountRTV,
	  D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
	  0
	};
	mHeapRTV = std::make_shared<DescriptorManager>(mDevice, heapDescRTV);

	D3D12_DESCRIPTOR_HEAP_DESC heapDescDSV{
	  D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
	  MaxDescriptorCountDSV,
	  D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
	  0
	};
	mHeapDSV = std::make_shared<DescriptorManager>(mDevice, heapDescDSV);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{
	  D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	  MaxDescriptorCount,
	  D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	  0
	};
	mHeap = std::make_shared<DescriptorManager>(mDevice, heapDesc);
}

void Application::createDefaultDepthBuffer(int width, int height)
{
	// construct depth buffer
	auto depthBufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		width,
		height,
		1, 0,
		1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.Format = depthBufferDesc.Format;
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.DepthStencil.Stencil = 0;

	HRESULT hr;

	auto cd3dx12_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	hr = mDevice->CreateCommittedResource(
		&cd3dx12_heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&depthBufferDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&mDepthBuffer)
	);
	ThrowIfFailed(hr, "CreateCommittedResource 失敗");

	// construct depth stencil view
	mDefaultDepthDSV = mHeapDSV->alloc();
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc
	{
	  DXGI_FORMAT_D32_FLOAT,
	  D3D12_DSV_DIMENSION_TEXTURE2D,
	  D3D12_DSV_FLAG_NONE,
	  {       // D3D12_TEX2D_DSV
		0     // MipSlice
	  }
	};
	mDevice->CreateDepthStencilView(mDepthBuffer.Get(), &dsvDesc, mDefaultDepthDSV);
}

void Application::createCommandAllocators()
{
	HRESULT hr;
	mCommandAllocators.resize(FrameBufferCount);
	for (UINT i = 0; i < FrameBufferCount; ++i)
	{
		hr = mDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&mCommandAllocators[i])
		);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed CreateCommandAllocator");
		}
	}
	hr = mDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&mOneshotCommandAllocator)
	);
	ThrowIfFailed(hr, "CreateCommandAllocator Failed(oneShot)");

	hr = mDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_BUNDLE,
		IID_PPV_ARGS(&mBundleCommandAllocator)
	);
	ThrowIfFailed(hr, "CreateCommandAllocator Failed(bundle)");
}

void Application::waitForIdleGPU()
{
	// wait for all command
	ComPtr<ID3D12Fence1> fence;
	const UINT64 expectValue = 1;
	HRESULT hr = mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	ThrowIfFailed(hr, "CreateFence 失敗");

	mCommandQueue->Signal(fence.Get(), expectValue);
	if (fence->GetCompletedValue() != expectValue)
	{
		fence->SetEventOnCompletion(expectValue, mWaitFence);
		WaitForSingleObject(mWaitFence, INFINITE);
	}
}
void Application::onSizeChanged(UINT width, UINT height, bool isMinimized)
{
	mWidth = width;
	mHeight = height;
	if (!mSwapchain || isMinimized)
		return;

	waitForIdleGPU();
	mSwapchain->resizeBuffers(width, height);

	mDepthBuffer.Reset();
	mHeapDSV->free(mDefaultDepthDSV);
	createDefaultDepthBuffer(mWidth, mHeight);

	mFrameIndex = mSwapchain->getCurrentBackBufferIndex();

	mViewport.Width = float(mWidth);
	mViewport.Height = float(mHeight);
	mScissorRect.right = mWidth;
	mScissorRect.bottom = mHeight;
}
void Application::toggleFullscreen()
{
	if (mSwapchain->isFullScreen())
	{
		// FullScreen -> Windowed
		mSwapchain->setFullScreen(false);
		SetWindowLong(mHwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		ShowWindow(mHwnd, SW_NORMAL);
	}
	else
	{
		// Windowed -> FullScreen
		DXGI_MODE_DESC desc;
		desc.Format = mSurfaceFormat;
		desc.Width = mWidth;
		desc.Height = mHeight;
		desc.RefreshRate.Denominator = 1;
		desc.RefreshRate.Numerator = 60;
		desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		mSwapchain->resizeTarget(&desc);
		mSwapchain->setFullScreen(true);
	}
	onSizeChanged(mWidth, mHeight, false);
}

void Application::prepareImGui()
{
	auto descriptorImGui = mHeap->alloc();
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpu(descriptorImGui);
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGpu(descriptorImGui);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(mHwnd);

	ID3D12DescriptorHeap* cbv_srv_heap = nullptr;

	ImGui_ImplDX12_Init(
		mDevice.Get(),
		FrameBufferCount,
		mSurfaceFormat,
		hCpu,
		hGpu);
}

void Application::cleanupImGui()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Application::setupComputePipelineAndSignature(vector<CD3DX12_STATIC_SAMPLER_DESC>& samplerDescTbl)
{
	for (auto& settings : mShaderSettingComputeTbl)
	{
		vector<CD3DX12_ROOT_PARAMETER> rootParams(settings.descriptors.size());
		for (s32 i = 0; i < settings.descriptorKeys.size(); i++)
		{
			const string key = settings.descriptorKeys[i];
			const s32 index = settings.descriptors[key].rootParamIndex;

			if (settings.descriptors[key].isConstantBuffer)
			{
				rootParams[index].InitAsConstantBufferView(settings.descriptors[key].descriptorRange.BaseShaderRegister);
			}
			else
			{
				rootParams[index].InitAsDescriptorTable(1, &settings.descriptors[key].descriptorRange);
			}
		}

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
		rootSignatureDesc.Init(
			UINT(rootParams.size()), rootParams.data(),
			UINT(samplerDescTbl.size()), samplerDescTbl.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature, errBlob;
		HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc,
			D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &errBlob);
		if (FAILED(hr))
		{
			auto error = reinterpret_cast<const char*>(errBlob->GetBufferPointer());
			OutputDebugStringA(error);
		}

		hr = mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mSignatureTbl[settings.nameAtPipeline]));
		ThrowIfFailed(hr, "CreateRootSignature failed.");

		Shader shaderCS;

		bool isCSValid = true;
		if (!settings.shaderFileName.empty())
		{
			isCSValid = shaderCS.load(settings.shaderFileName, Shader::Compute, settings.shaderEntryPoint, settings.flags, settings.shaderMacro);
		}

		PipelineState pipeline;
		if (isCSValid)
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC computeDesc{};
			computeDesc.CS = CD3DX12_SHADER_BYTECODE(shaderCS.getCode().Get());
			computeDesc.pRootSignature = mSignatureTbl[settings.nameAtPipeline].Get();
			
			hr = mDevice->CreateComputePipelineState(&computeDesc, IID_PPV_ARGS(&pipeline));
			ThrowIfFailed(hr, "CreateComputePipelineState failed.");

			mPipelineStateTbl[settings.nameAtPipeline] = pipeline;
		}
		else
		{
			ErrorShader errShader = { settings.shaderFileName, settings.shaderEntryPoint, settings.nameAtPipeline };
			mErrorShaderTbl.emplace_back(errShader);
			//if (mPipelineStateTbl[settings.nameAtPipeline].Get() == nullptr)
			//{
			//	throw runtime_error("First time shader compile failed");
			//}
		}
	}
}

void Application::setupGraphicsPipelineAndSignature(vector<CD3DX12_STATIC_SAMPLER_DESC>& samplerDescTbl, bool isDepthEnable, bool isAlphaEnable)
{
	for (auto& settings : mShaderSettingGraphicsTbl)
	{
		vector<CD3DX12_ROOT_PARAMETER> rootParams(settings.descriptors.size());
		for (s32 i = 0; i < settings.descriptorKeys.size(); i++)
		{
			const string key = settings.descriptorKeys[i];
			const s32 index = settings.descriptors[key].rootParamIndex;

			if (settings.descriptors[key].isConstantBuffer)
			{
				rootParams[index].InitAsConstantBufferView(settings.descriptors[key].descriptorRange.BaseShaderRegister);
			}
			else
			{
				rootParams[index].InitAsDescriptorTable(1, &settings.descriptors[key].descriptorRange);
			}
		}

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
		rootSignatureDesc.Init(
			UINT(rootParams.size()), rootParams.data(),
			UINT(samplerDescTbl.size()), samplerDescTbl.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature, errBlob;
		HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc,
			D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &errBlob);
		if (FAILED(hr))
		{
			auto error = reinterpret_cast<const char*>(errBlob->GetBufferPointer());
			OutputDebugStringA(error);
		}

		hr = mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mSignatureTbl[settings.nameAtPipeline]));
		ThrowIfFailed(hr, "CreateRootSignature failed.");

		Shader shaderVS, shaderPS, shaderGS, shaderHS, shaderDS;

		bool isVSValid = true, isPSValid = true, isGSValid = true, isHSValid = true, isDSValid = true;
		ErrorShader errShader;
		if (!settings.shaderFileNameVS.empty())
		{
			isVSValid = shaderVS.load(settings.shaderFileNameVS, Shader::Vertex, settings.shaderEntryPointVS, settings.flags, settings.shaderMacroVS);
			if (!isVSValid)
			{
				errShader.shaderFileName = settings.shaderFileNameVS;
				errShader.shaderEntryPoint = settings.shaderEntryPointVS;
				errShader.nameAtPipeline = settings.nameAtPipeline;
				mErrorShaderTbl.emplace_back(errShader);
			}
		}
		if (!settings.shaderFileNamePS.empty() && isVSValid)
		{
			isPSValid = shaderPS.load(settings.shaderFileNamePS, Shader::Pixel, settings.shaderEntryPointPS, settings.flags, settings.shaderMacroPS);
			if (!isVSValid)
			{
				errShader.shaderFileName = settings.shaderFileNamePS;
				errShader.shaderEntryPoint = settings.shaderEntryPointPS;
				errShader.nameAtPipeline = settings.nameAtPipeline;
				mErrorShaderTbl.emplace_back(errShader);
			}
		}
		if (!settings.shaderFileNameGS.empty() && isPSValid)
		{
			isGSValid = shaderGS.load(settings.shaderFileNameGS, Shader::Geometry, settings.shaderEntryPointGS, settings.flags, settings.shaderMacroGS);
			if (!isVSValid)
			{
				errShader.shaderFileName = settings.shaderFileNameGS;
				errShader.shaderEntryPoint = settings.shaderEntryPointGS;
				errShader.nameAtPipeline = settings.nameAtPipeline;
				mErrorShaderTbl.emplace_back(errShader);
			}
		}
		if (!settings.shaderFileNameHS.empty() && isGSValid)
		{
			isHSValid = shaderHS.load(settings.shaderFileNameHS, Shader::Hull, settings.shaderEntryPointHS, settings.flags, settings.shaderMacroHS);
			if (!isVSValid)
			{
				errShader.shaderFileName = settings.shaderFileNameHS;
				errShader.shaderEntryPoint = settings.shaderEntryPointHS;
				errShader.nameAtPipeline = settings.nameAtPipeline;
				mErrorShaderTbl.emplace_back(errShader);
			}
		}
		if (!settings.shaderFileNameDS.empty() && isHSValid)
		{
			isDSValid = shaderDS.load(settings.shaderFileNameDS, Shader::Domain, settings.shaderEntryPointDS, settings.flags, settings.shaderMacroDS);
			if (!isVSValid)
			{
				errShader.shaderFileName = settings.shaderFileNameDS;
				errShader.shaderEntryPoint = settings.shaderEntryPointDS;
				errShader.nameAtPipeline = settings.nameAtPipeline;
				mErrorShaderTbl.emplace_back(errShader);
			}
		}

		PipelineState pipeline;

		if (isVSValid && isPSValid && isGSValid && isHSValid && isDSValid)
		{
			auto psoDesc = appUtility::createDefaultPsoDesc(
				DXGI_FORMAT_R8G8B8A8_UNORM,
				settings.rasterizerState,
				settings.imputElements.data(), settings.imputElements.size(),
				mSignatureTbl[settings.nameAtPipeline],
				shaderVS.getCode(),
				shaderPS.getCode(),
				shaderGS.getCode(),
				shaderHS.getCode(),
				shaderDS.getCode(),
				isDepthEnable,
				isAlphaEnable
			);

			hr = mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipeline));
			ThrowIfFailed(hr, "CreateGraphicsPipelineState Failed.");

			mPipelineStateTbl[settings.nameAtPipeline] = pipeline;
		}
		else
		{
		/*	if (mPipelineStateTbl[settings.nameAtPipeline].Get() == nullptr)
			{
				throw runtime_error("First time shader compile failed");
			}*/
		}
	}
}

void Application::initBufferByResource(DX12Buffer& targetBuffer, const u32 bufferSize, const void* initResource)
{
	auto command = createCommandList();

	D3D12_RESOURCE_STATES firstState = (targetBuffer.getState() == DX12Buffer::SRV) ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE : D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	D3D12_RESOURCE_BARRIER barrier[2] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
		targetBuffer.getBuffer().Get(),
		firstState,
		D3D12_RESOURCE_STATE_COPY_DEST),
		CD3DX12_RESOURCE_BARRIER::Transition(
		targetBuffer.getBuffer().Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		firstState)
	};

	command->ResourceBarrier(1, &barrier[0]);

	auto desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	auto upload = createResource(desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, D3D12_HEAP_TYPE_UPLOAD);
	writeToUploadHeapMemory(upload.Get(), bufferSize, initResource);

	command->CopyResource(targetBuffer.getBuffer().Get(), upload.Get());

	D3D12_RESOURCE_BARRIER barrierFromDEST = CD3DX12_RESOURCE_BARRIER::Transition(
		targetBuffer.getBuffer().Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		firstState);

	command->ResourceBarrier(1, &barrier[1]);
	finishCommandList(command);
}

void Application::createIndexBuffer(Buffer& indexBuffer, const u32 bufferSize, const void* initResource)
{
	auto indexDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	indexBuffer = createResource(indexDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, D3D12_HEAP_TYPE_DEFAULT);
	auto uploadIB = createResource(indexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, D3D12_HEAP_TYPE_UPLOAD);
	writeToUploadHeapMemory(uploadIB.Get(), bufferSize, initResource);

	auto command = createCommandList();
	command->CopyResource(indexBuffer.Get(), uploadIB.Get());

	D3D12_RESOURCE_BARRIER barrierFromDEST = CD3DX12_RESOURCE_BARRIER::Transition(
		indexBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_INDEX_BUFFER);

	command->ResourceBarrier(1, &barrierFromDEST);
	finishCommandList(command);
}