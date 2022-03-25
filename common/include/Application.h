#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d12.h>
#include <dxgi1_6.h>

#include "d3dx12.h"
#include <wrl.h>

#include "DescriptorManager.h"
#include "Swapchain.h"
#include <memory>
#include <unordered_map>

#include <profileapi.h>

#include "../Program/include/DX12Buffer.h"
#include "../Program/include/LinearAlgebra.h"

#include "../Program/include/Shader.h"

typedef LAVector Float3;
typedef LAVector Color;
typedef LAVector Lambda;
typedef LAVector vec3;
typedef DirectX::XMFLOAT4 GhostData;
typedef DirectX::XMFLOAT2 FLOAT2;
typedef DirectX::XMFLOAT3 FLOAT3;
typedef DirectX::XMFLOAT4 FLOAT4;
typedef DirectX::XMFLOAT4X4 MAT4;
typedef float f32;
typedef int s32;
typedef unsigned int u32;
typedef double f64;

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class DX12Buffer;

class Application
{
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	using Buffer = ComPtr<ID3D12Resource1>;

	using Buffer = ComPtr<ID3D12Resource1>;
	using Texture = ComPtr<ID3D12Resource1>;
	struct ModelData
	{
		UINT idxCount;
		UINT vtxCount;
		D3D12_VERTEX_BUFFER_VIEW vbView;
		D3D12_INDEX_BUFFER_VIEW  ibView;
		Buffer vbBuffer;
		Buffer ibBuffer;
	};

	struct DescriptorInfo
	{
		s32 rootParamIndex;
		D3D12_DESCRIPTOR_RANGE descriptorRange;
	};

	enum PipelineType
	{
		PipelineType_Graphics,
		PipelineType_Compute
	};

	struct ShaderSettingGraphics
	{
		const PipelineType pipelineType = PipelineType_Graphics;
		std::wstring shaderFileNameVS;
		std::wstring shaderEntryPointVS;
		std::wstring shaderFileNamePS;
		std::wstring shaderEntryPointPS;
		std::wstring shaderFileNameGS;
		std::wstring shaderEntryPointGS;
		std::wstring shaderFileNameHS;
		std::wstring shaderEntryPointHS;
		std::wstring shaderFileNameDS;
		std::wstring shaderEntryPointDS;
		std::string nameAtPipeline;

		std::vector<Shader::DefineMacro> shaderMacroVS;
		std::vector<Shader::DefineMacro> shaderMacroPS;
		std::vector<Shader::DefineMacro> shaderMacroGS;
		std::vector<Shader::DefineMacro> shaderMacroHS;
		std::vector<Shader::DefineMacro> shaderMacroDS;
		std::vector<std::wstring> flags;

		std::vector<std::string> descriptorKeys;
		std::unordered_map<std::string, DescriptorInfo> descriptors;
		std::vector<D3D12_INPUT_ELEMENT_DESC> imputElements;
		CD3DX12_RASTERIZER_DESC rasterizerState;
	};

	struct ShaderSettingCompute
	{
		const PipelineType pipelineType = PipelineType_Compute;
		std::wstring shaderFileName;
		std::wstring shaderEntryPoint;
		std::string nameAtPipeline;

		std::vector<Shader::DefineMacro> shaderMacro;
		std::vector<std::wstring> flags;

		std::vector<std::string> descriptorKeys;
		std::unordered_map<std::string, DescriptorInfo> descriptors;
		std::vector<D3D12_INPUT_ELEMENT_DESC> imputElements;
		CD3DX12_RASTERIZER_DESC rasterizerState;
	};

	struct BackBuffer
	{
		Buffer texture;
		DescriptorHandle srv;
		DescriptorHandle rtv;
	};

	struct ErrorShader
	{
		std::wstring shaderFileName;
		std::wstring shaderEntryPoint;
		std::string nameAtPipeline;
	};

	struct U32Size
	{
		u32 w;
		u32 h;
	};

	using PipelineState = ComPtr<ID3D12PipelineState>;

	Application();
	virtual ~Application();

	void initializeApp(HWND hWnd, DXGI_FORMAT format, bool isFullScreen);
	void terminate();

	virtual void render();// = 0;
	virtual void prepare() { }
	virtual void cleanup() { }

	const UINT GpuWaitTimeout = (10 * 1000);  // 10s
	//static const UINT FrameBufferCount = 2;
	static const UINT FrameBufferCount = 3;

	virtual void onSizeChanged(UINT width, UINT height, bool isMinimized);
	virtual void onMouseButtonDown(UINT msg) { }
	virtual void onMouseButtonUp(UINT msg) { }
	virtual void onMouseMove(UINT msg, int dx, int dy) { }

	void setupComputePipelineAndSignature(std::vector<CD3DX12_STATIC_SAMPLER_DESC>& samplerDescTbl);
	void setupGraphicsPipelineAndSignature(std::vector<CD3DX12_STATIC_SAMPLER_DESC>& samplerDescTbl, bool isDepthEnable = false, bool isAlphaEnable = false);
	void initBufferByResource(DX12Buffer& targetBuffer, const u32 bufferSize, const void* initResource);
	void createIndexBuffer(Buffer& indexBuffer, const u32 bufferSize, const void* initResource);

	void setTitle(const std::string& title);
	void toggleFullscreen();

	ComPtr<ID3D12Device> getDevice() { return mDevice; }
	std::shared_ptr<Swapchain> getSwapchain() { return mSwapchain; }

	ComPtr<ID3D12Resource1> createResource(const CD3DX12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES resourceStates, const D3D12_CLEAR_VALUE* clearValue, D3D12_HEAP_TYPE heapType);

	std::vector<ComPtr<ID3D12Resource1>> createConstantBuffers(const CD3DX12_RESOURCE_DESC& desc, int count = FrameBufferCount);

	//commandBuffer
	ComPtr<ID3D12GraphicsCommandList>  createCommandList();
	void finishCommandList(ComPtr<ID3D12GraphicsCommandList>& command);
	ComPtr<ID3D12GraphicsCommandList> createBundleCommandList();

	void updateBuffer(ID3D12Resource1* resource, uint32_t size, const void* pData);

	std::shared_ptr<DescriptorManager> getDescriptorManager() { return mHeap; }

	template<class T>
	ModelData createSimpleModel(const std::vector<T>& vertices, const std::vector<uint32_t>& indices)
	{
		ModelData model;
		auto bufferSize = uint32_t(sizeof(T) * vertices.size());
		auto vbDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
		auto srcHeapType = D3D12_HEAP_TYPE_UPLOAD;
		auto dstHeapType = D3D12_HEAP_TYPE_DEFAULT;

		model.vbBuffer = createResource(vbDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, dstHeapType);
		auto uploadVB = createResource(vbDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, srcHeapType);
		updateBuffer(uploadVB.Get(), bufferSize, vertices.data());

		bufferSize = UINT(sizeof(UINT) * indices.size());
		auto ibDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
		model.ibBuffer = createResource(ibDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, dstHeapType);
		auto uploadIB = createResource(ibDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, srcHeapType);
		updateBuffer(uploadIB.Get(), bufferSize, indices.data());

		auto command = createCommandList();
		command->CopyResource(model.vbBuffer.Get(), uploadVB.Get());
		command->CopyResource(model.ibBuffer.Get(), uploadIB.Get());

		D3D12_RESOURCE_BARRIER barriers[] = {
		  CD3DX12_RESOURCE_BARRIER::Transition(
			model.vbBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
		  CD3DX12_RESOURCE_BARRIER::Transition(
			model.ibBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_INDEX_BUFFER)
		};
		command->ResourceBarrier(_countof(barriers), barriers);
		finishCommandList(command);

		model.idxCount = UINT(indices.size());
		model.vtxCount = UINT(vertices.size());

		model.vbView.BufferLocation = model.vbBuffer->GetGPUVirtualAddress();
		model.vbView.StrideInBytes = sizeof(T);
		model.vbView.SizeInBytes = UINT(model.vbView.StrideInBytes * vertices.size());
		model.ibView.BufferLocation = model.ibBuffer->GetGPUVirtualAddress();
		model.ibView.Format = DXGI_FORMAT_R32_UINT;
		model.ibView.SizeInBytes = UINT(sizeof(UINT) * indices.size());

		return model;
	}

protected:

	void prepareDescriptorHeaps();

	void createDefaultDepthBuffer(int width, int height);
	void createCommandAllocators();
	void waitForIdleGPU();

	// ImGui
	void prepareImGui();
	void cleanupImGui();

	void setPipelineState(const PipelineType type, const s32 pass);
	void setPipelineConstantResource(const std::string resourceName, const D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
	void setPipelineResource(const std::string resourceName, const D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor);
	void dispatch(const u32 x, const u32 y, const u32 z);
	void setIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* ivViewPtr);
	void setPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY topology);
	void drawInstanced(const u32 vertexCountPerInstance, const u32 instanceCount, const u32 startIVertexLocation, const u32 startInstanceLocation);
	void drawIndexedInstanced(const u32 indexCountPerInstance, const u32 instanceCount, const u32 startIndexLocation, const s32 baseVertexLocation, const u32 startInstanceLocation);
	void setVertexBuffers(const u32 startSlot, const u32 numViews, const D3D12_VERTEX_BUFFER_VIEW* vbViewPtr);

	std::unordered_map<std::string, ComPtr<ID3D12RootSignature>> mSignatureTbl;
	std::unordered_map<std::string, PipelineState> mPipelineStateTbl;
	std::vector<ShaderSettingCompute> mShaderSettingComputeTbl;
	std::vector<ShaderSettingGraphics> mShaderSettingGraphicsTbl;
	std::vector<ErrorShader> mErrorShaderTbl;

	ComPtr<ID3D12Device> mDevice;
	ComPtr<ID3D12CommandQueue> mCommandQueue;

	std::shared_ptr<Swapchain> mSwapchain;

	std::vector<ComPtr<ID3D12Resource1>> mDenderTargets;
	ComPtr<ID3D12Resource1> mDepthBuffer;

	CD3DX12_VIEWPORT  mViewport;
	CD3DX12_RECT mScissorRect;
	DXGI_FORMAT  mSurfaceFormat;


	std::vector<ComPtr<ID3D12CommandAllocator>> mCommandAllocators;
	ComPtr<ID3D12CommandAllocator> mOneshotCommandAllocator;
	ComPtr<ID3D12CommandAllocator> mBundleCommandAllocator;

	std::shared_ptr<DescriptorManager> mHeapRTV;
	std::shared_ptr<DescriptorManager> mHeapDSV;
	std::shared_ptr<DescriptorManager> mHeap;

	DescriptorHandle mDefaultDepthDSV;
	ComPtr<ID3D12GraphicsCommandList> mCommandList;
	HANDLE mWaitFence;

	UINT mFrameIndex;

	PipelineType mPipelineType;
	s32 mPipelinePass;

	U32Size mScreenSize;
	bool mIsAllowTearing;
	HWND mHwnd;
};
