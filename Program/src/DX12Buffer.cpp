#include "../include/DX12Buffer.h"
template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
using Buffer = ComPtr<ID3D12Resource1>;
DescriptorHandle DX12Buffer::getSRV(const ComPtr<ID3D12GraphicsCommandList>& commandlist)
{
	if (mState == UAV)
	{
		mState = SRV;
        commandlist->ResourceBarrier(1, &mBarrierUAV2SRV);
	}
	return mBufferSRV;
}

DescriptorHandle DX12Buffer::getUAV(const ComPtr<ID3D12GraphicsCommandList>& commandlist)
{
	if (mState == SRV)
	{
		mState = UAV;
        commandlist->ResourceBarrier(1, &mBarrierSRV2UAV);
	}
	return mBufferUAV;
}

Buffer DX12Buffer::getBuffer()
{
    return mBuffer;
}

void DX12Buffer::createRegularBuffer(
	const std::shared_ptr<DescriptorManager>& heap, const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandlist,
	CD3DX12_RESOURCE_DESC bufferDesc, D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc, D3D12_UNORDERED_ACCESS_VIEW_DESC& uavDesc,
	D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES firstState, LPCWSTR resourceName)
{
    auto cd3dx12HeapProperties = CD3DX12_HEAP_PROPERTIES(heapType);

    HRESULT hr = device->CreateCommittedResource(
        &cd3dx12HeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        firstState,
        nullptr,
        IID_PPV_ARGS(&mBuffer)
    );
    ThrowIfFailed(hr, "CreateCommittedResource failed.");

    if (firstState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        mState = UAV;
    else if (firstState == D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
        mState = SRV;

    mBufferSRV = heap->alloc();
    device->CreateShaderResourceView(
        mBuffer.Get(),
        &srvDesc,
        mBufferSRV
    );

    mBufferUAV = heap->alloc();
    device->CreateUnorderedAccessView(
        mBuffer.Get(),
        nullptr,
        &uavDesc,
        mBufferUAV
    );

    auto barrierWtoR = CD3DX12_RESOURCE_BARRIER::Transition(
        mBuffer.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    mBarrierUAV2SRV = barrierWtoR;

    auto barrierRtoW = CD3DX12_RESOURCE_BARRIER::Transition(
        mBuffer.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS
    );
    mBarrierSRV2UAV = barrierRtoW;

    mBuffer->SetName(resourceName);
}