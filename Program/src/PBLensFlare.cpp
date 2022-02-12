#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include "../include/PBLensFlare.h"

#include <DirectXTex.h>
#include <codecvt>
#include <algorithm>

using namespace std;
using namespace DirectX;

PBLensFlare::PBLensFlare() :
	mIsDragged(false),
	mButtonType(0),

	mTexwidth(0),
	mTexheight(0),
	mGhostTexWidth(0),
	mGhostTexHeight(0),

	mDrawMode(0),
	mLensType(LensType_NIKON),

	mSpread(4.228),
	mGhostScale(0.014),
	mApertureRadius(0.4),
	mGhostIntensityRatio(1000),
	mColor(FLOAT3(1.f, 1.f, 1.f)),
	mApertureBladeNum(8),
	mRotAngle(15),
	mIntensity(20),
	mBurgstGlitter(0.23),
	mSelectGhostID(-1),
	mInvisibleReflectance(0),

	mPropdistance(20),
	mIntervalX(10),
	mPosX(0.485),
	mPosY(0.485),
	mPosXSave(0.51),//force update
	mPosYSave(0.51),
	mGlarelambdasamplenum(50),

	mUseAR(false),

	mExecuteTimeMS(0),

	mGridNum(GridNum_16x16)
{
	mTextureNames.resize(ReadOnlyImage_COUNT);

	string ext = ".png";

	mTextureNames.at(0) = "dust" + ext;
	mTextureNames.at(0).resize(MAX_PATH);

	mFullsizeTex.resize(ReadOnlyImage_COUNT);
}

int PBLensFlare::inv2pow(int n) {
	int m = 0;
	m = static_cast<int>(log(static_cast<double>(n)) / log(2.0));
	return m;
}

void PBLensFlare::cleanup()
{

}



void PBLensFlare::prepare()
{
	setTitle("Physically Based Lens Flare");

	mCommandList->Reset(mCommandAllocators[0].Get(), nullptr);
	ID3D12DescriptorHeap* heaps[] = { mHeap->getHeap().Get() };
	mCommandList->SetDescriptorHeaps(1, heaps);
	mCommandList->Close();
	updateSourceImage();
	setupWorkingTexture();

	setupRayTraceLensFlare();
}

void PBLensFlare::updateSourceImage()
{
	vector<ImageSize> size;

	vector<wstring> names;

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	const string imageslash = "image/";

	ImageSize ss;
	ImageSize dust;

	u32 count = 0;
	bool isFFTEnable = true;
	vector<TextureData> texTbl;

	auto isPow2 = [](u32 x)
	{
		if (x == 0) {
			return false;
		}
		return (x & (x - 1)) == 0;
	};

	for (auto& name : mTextureNames)
	{
		string s(imageslash + name);
		std::wstring wides = converter.from_bytes(s);
		bool isValid;
		TextureData tex = LoadTextureFromFile(wides, ss, isValid);
		if (!isValid)
		{
			isFFTEnable = false;
			break;
		}

		if (count == ReadOmlyImage_ApertureDust)
		{
			if ((ss.w <= MAX_DUST_IMAGE_SIZE)
				&& (ss.h <= MAX_DUST_IMAGE_SIZE)
				&& isPow2(ss.w)
				&& isPow2(ss.h))
			{
				dust = ss;
			}
			else
			{
				isFFTEnable = false;
			}
		}

		texTbl.push_back(tex);
		count++;
	}

	mFFTEnable = isFFTEnable;

	if (!isFFTEnable)
	{
		return;
	}

	mTexwidth = dust.w;
	mTexheight = dust.h;

	mGhostTexWidth = mTexwidth * 2;
	mGhostTexHeight = mTexheight * 2;

	mFullsizeTex.clear();
	mFullsizeTex.resize(ReadOnlyImage_COUNT);

	for (u32 i = 0; i < mTextureNames.size(); ++i)
	{
		mFullsizeTex[i] = texTbl[i];
	}

	mRecompile = true;
}



PBLensFlare::TextureData PBLensFlare::LoadTextureFromFile(const std::wstring& name, ImageSize& size, bool& isValid)
{
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage image;

	HRESULT hr;
	if (name.find(L".png") != std::wstring::npos)
	{
		hr = DirectX::LoadFromWICFile(name.c_str(), 0, &metadata, image);
	}
	if (name.find(L".dds") != std::wstring::npos)
	{
		hr = DirectX::LoadFromDDSFile(name.c_str(), 0, &metadata, image);
	}
	if (name.find(L".tga") != std::wstring::npos)
	{
		hr = DirectX::LoadFromTGAFile(name.c_str(), &metadata, image);
	}

	if (hr != 0)
	{
		isValid = false;
		TextureData texData;
		return texData;
	}

	size.w = metadata.width;
	size.h = metadata.height;

	D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;
	resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	ComPtr<ID3D12Resource> texture;
	CreateTextureEx(mDevice.Get(), metadata, resFlags, false, &texture);

	Buffer srcBuffer;
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	PrepareUpload(mDevice.Get(), image.GetImages(), image.GetImageCount(), metadata, subresources);
	const auto totalBytes = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresources.size()));

	auto staging = createResource(CD3DX12_RESOURCE_DESC::Buffer(totalBytes), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, D3D12_HEAP_TYPE_UPLOAD);
	auto command = createCommandList();
	UpdateSubresources(command.Get(),
		texture.Get(), staging.Get(), 0, 0, UINT(subresources.size()), subresources.data());
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		texture.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	command->ResourceBarrier(1, &barrier);
	finishCommandList(command);

	TextureData texData;
	texture.As(&texData.texture);
	texData.SRV = mHeap->alloc();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.TextureCube.MipLevels = UINT(metadata.mipLevels);
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.ResourceMinLODClamp = 0;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	mDevice->CreateShaderResourceView(texture.Get(), &srvDesc, texData.SRV);

	isValid = true;
	return texData;
}