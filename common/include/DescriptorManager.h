#pragma once
#include <wrl.h>
#include <list>

#include "AppUtil.h"
#include "d3dx12.h"

class DescriptorHandle
{
public:
	DescriptorHandle() : mHandleCpu(), mHandleGpu() {}

	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE hCpu, D3D12_GPU_DESCRIPTOR_HANDLE hGpu)
		: mHandleCpu(hCpu), mHandleGpu(hGpu)
	{
	}

	operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return mHandleCpu; }
	operator D3D12_GPU_DESCRIPTOR_HANDLE() const { return mHandleGpu; }

private:
	D3D12_CPU_DESCRIPTOR_HANDLE mHandleCpu;
	D3D12_GPU_DESCRIPTOR_HANDLE mHandleGpu;
};

class DescriptorManager
{
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	DescriptorManager(ComPtr<ID3D12Device> device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
		: mIndex(0), mIncrementSize(0)
	{
		HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mHeap));
		ThrowIfFailed(hr, "CreateDescriptorHeap Failed.");

		mHandleCpu = mHeap->GetCPUDescriptorHandleForHeapStart();
		mHandleGpu = mHeap->GetGPUDescriptorHandleForHeapStart();
		mIncrementSize = device->GetDescriptorHandleIncrementSize(desc.Type);
	}
	ComPtr<ID3D12DescriptorHeap> getHeap() const { return mHeap; }

	DescriptorHandle alloc()
	{
		if (!mFreeList.empty())
		{
			auto ret = mFreeList.front();
			mFreeList.pop_front();
			return ret;
		}

		mHandleCpu = mHeap->GetCPUDescriptorHandleForHeapStart();
		mHandleGpu = mHeap->GetGPUDescriptorHandleForHeapStart();

		UINT use = mIndex++;
		auto ret = DescriptorHandle(
			mHandleCpu.Offset(use, mIncrementSize),
			mHandleGpu.Offset(use, mIncrementSize)
		);

		return ret;
	}

	void free(const DescriptorHandle& handle)
	{
		mFreeList.push_back(handle);
	}


private:
	ComPtr<ID3D12DescriptorHeap> mHeap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mHandleCpu;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mHandleGpu;
	UINT mIndex;
	UINT mIncrementSize;

	std::list<DescriptorHandle> mFreeList;
};