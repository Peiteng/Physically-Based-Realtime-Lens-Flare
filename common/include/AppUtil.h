#pragma once
#include <stdexcept>
#include <d3d12.h>
#include <DirectXMath.h>
#include "d3dx12.h"

#define STRINGFY(s)  #s
#define TO_STRING(x) STRINGFY(x)
#define FILE_PREFIX __FILE__ "(" TO_STRING(__LINE__) "): " 
#define ThrowIfFailed(hr, msg) appUtility::checkResultCodeD3D12( hr, FILE_PREFIX msg)

namespace appUtility
{
    class DX12Exception : public std::runtime_error
    {
    public:
        DX12Exception(const std::string& msg) : std::runtime_error(msg.c_str()) {
        }
    };

    inline void checkResultCodeD3D12(HRESULT hr, const std::string& errorMsg)
    {
        if (FAILED(hr))
        {
            throw DX12Exception(errorMsg);
        }
    }

    inline UINT roundupBufferSize(UINT size)
    {
        size = (size + 0xff) & ~0xff;
        return size;
    }

    inline D3D12_GRAPHICS_PIPELINE_STATE_DESC createDefaultPsoDesc(
        DXGI_FORMAT targetFormat,
        CD3DX12_RASTERIZER_DESC rasterizerDesc,
        const D3D12_INPUT_ELEMENT_DESC* inputElementDesc, UINT inputElementDescCount,
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSig,
        Microsoft::WRL::ComPtr<ID3DBlob> vs,
        Microsoft::WRL::ComPtr<ID3DBlob> ps,
        Microsoft::WRL::ComPtr<ID3DBlob> gs = nullptr,
        Microsoft::WRL::ComPtr<ID3DBlob> hs = nullptr,
        Microsoft::WRL::ComPtr<ID3DBlob> ds = nullptr,
        bool isDepthEnable = false,
        bool isAlphaEnable = false
    )
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps.Get());
        if (gs)
        {
            psoDesc.GS = CD3DX12_SHADER_BYTECODE(gs.Get());
        }
        if (hs)
        {
            psoDesc.HS = CD3DX12_SHADER_BYTECODE(hs.Get());
        }
        if (ds)
        {
            psoDesc.DS = CD3DX12_SHADER_BYTECODE(ds.Get());
        }

        if (isAlphaEnable)
        {
            D3D12_BLEND_DESC blendDesc;
            blendDesc.AlphaToCoverageEnable = false;
            blendDesc.IndependentBlendEnable = false;
            const D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc =
            {
                TRUE,FALSE,
                D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ONE, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP,
                D3D12_COLOR_WRITE_ENABLE_ALL,
            };
            for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
                blendDesc.RenderTarget[i] = renderTargetBlendDesc;
            psoDesc.BlendState = CD3DX12_BLEND_DESC(blendDesc);
        }
        else
        {
            psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        }

        psoDesc.RasterizerState = rasterizerDesc;

        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = targetFormat;
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = isDepthEnable;
        psoDesc.InputLayout = { inputElementDesc, inputElementDescCount };

        psoDesc.pRootSignature = rootSig.Get();
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.SampleDesc = { 1,0 };
        psoDesc.SampleMask = UINT_MAX; // MUST USE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        return psoDesc;
    }

    inline Microsoft::WRL::ComPtr<ID3D12Resource1> createBufferOnUploadHeap(
        Microsoft::WRL::ComPtr<ID3D12Device> device, UINT bufferSize, const void* data = nullptr)
    {
        Microsoft::WRL::ComPtr<ID3D12Resource1> ret;
        HRESULT hr;

        auto cd3dx12_heap_propertises = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto cd3dx12_resource_desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

        hr = device->CreateCommittedResource(
            &cd3dx12_heap_propertises,
            D3D12_HEAP_FLAG_NONE,
            &cd3dx12_resource_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&ret)
        );
        ThrowIfFailed(hr, "CreateCommittedResource failed.");
        if (data != nullptr)
        {
            void* mapped;
            hr = ret->Map(0, nullptr, &mapped);
            ThrowIfFailed(hr, "Map failed.");
            if (mapped) {
                memcpy(mapped, data, bufferSize);
                ret->Unmap(0, nullptr);
            }
        }
        return ret;
    }
}

