#pragma once
#include "../common/include/Application.h"

class DX12Buffer
{
public:
	enum State {
		SRV,
		UAV
	};

	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	using Buffer = ComPtr<ID3D12Resource1>;

	DescriptorHandle getSRV(const ComPtr<ID3D12GraphicsCommandList>& commandlist);
	DescriptorHandle getUAV(const ComPtr<ID3D12GraphicsCommandList>& commandlist);
	Buffer getBuffer();
	State getState() { return mState; };

	void createRegularBuffer(
		const std::shared_ptr<DescriptorManager>& heap, const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandlist, 
		CD3DX12_RESOURCE_DESC bufferDesc,  D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc, D3D12_UNORDERED_ACCESS_VIEW_DESC& uavDesc,
		D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES firstState, LPCWSTR resourceName);

private:
	Buffer mBuffer;
	DescriptorHandle mBufferSRV;
	DescriptorHandle mBufferUAV;

	CD3DX12_RESOURCE_BARRIER mBarrierSRV2UAV;
	CD3DX12_RESOURCE_BARRIER mBarrierUAV2SRV;

	State mState;
};
