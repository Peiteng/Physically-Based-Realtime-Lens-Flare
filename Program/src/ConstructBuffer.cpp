#include "../include/PBLensFlare.h"

void PBLensFlare::setupWorkingTexture()
{
	const s32 RWtexNum = 10;

	mRWFullsizeTex.resize(0);
	mRWfullsizeInnerTex.resize(0);
	mRWdisplayTex.resize(0);

	mRWFullsizeTex.shrink_to_fit();
	mRWfullsizeInnerTex.shrink_to_fit();
	mRWdisplayTex.shrink_to_fit();

	auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, mTexwidth, mTexheight);
	texDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = texDesc.Format;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	uavDesc.Texture2D.PlaneSlice = 0;
	uavDesc.Format = texDesc.Format;

	DX12Buffer tex;
	for (s32 i = 0; i < RWtexNum; i++)
	{
		wchar_t name[30];
		swprintf(name, 30, L"mRWFullsizeTex[%d]", i);
		tex.createRegularBuffer(
			mHeap, mDevice, mCommandList,
			texDesc, srvDesc, uavDesc,
			D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, name);
		mRWFullsizeTex.push_back(tex);
	}

	for (s32 i = 0; i < RWtexNum; i++)
	{
		wchar_t name[30];
		swprintf(name, 30, L"mRWfullsizeInnerTex[%d]", i);
		tex.createRegularBuffer(
			mHeap, mDevice, mCommandList,
			texDesc, srvDesc, uavDesc,
			D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, name);
		mRWfullsizeInnerTex.push_back(tex);
	}

	for (s32 i = 0; i < DisplayImage_COUNT; i++)
	{
		wchar_t name[30];
		swprintf(name, 30, L"mRWdisplayTex[%d]", i);
		tex.createRegularBuffer(
			mHeap, mDevice, mCommandList,
			texDesc, srvDesc, uavDesc,
			D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, name);
		mRWdisplayTex.push_back(tex);
	}

	{
		wchar_t name[30] = L"mRWBurstCachedTex";
		tex.createRegularBuffer(
			mHeap, mDevice, mCommandList,
			texDesc, srvDesc, uavDesc,
			D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, name);
		mBurstCachedTex = tex;
	}

	auto texDescOneElem = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8_UNORM, mGhostTexWidth, mGhostTexHeight);
	texDescOneElem.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescOneElem{};
	srvDescOneElem.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDescOneElem.Texture2D.MipLevels = 1;
	srvDescOneElem.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescOneElem.Format = texDescOneElem.Format;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDescOneElem{};
	uavDescOneElem.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDescOneElem.Texture2D.MipSlice = 0;
	uavDescOneElem.Texture2D.PlaneSlice = 0;
	uavDescOneElem.Format = texDescOneElem.Format;

	{
		wchar_t name[30] = L"mRWGhostCachedTex";
		tex.createRegularBuffer(
			mHeap, mDevice, mCommandList,
			texDescOneElem, srvDescOneElem, uavDescOneElem,
			D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, name);
		mGhostCachedTex = tex;
	}
}

void PBLensFlare::setupRayTraceLensFlare()
{
	constructConstantBufferForLensFlare();
	constructLensFlareComponents();
	constructRayBundle();
	constructQuad();
	constructBackBuffer();
	setupSimulateLensFlarePipeline();
}

void PBLensFlare::constructConstantBufferForLensFlare()
{
	mTracingCB = createConstantBuffers(CD3DX12_RESOURCE_DESC::Buffer(
		sizeof(tracingCB)
	));
	mDrawBurstCB = createConstantBuffers(CD3DX12_RESOURCE_DESC::Buffer(
		sizeof(drawBurstCB)
	));
	mDrawingCB = createConstantBuffers(CD3DX12_RESOURCE_DESC::Buffer(
		sizeof(drawingCB)
	));
	mLensFlareSceneCB = createConstantBuffers(CD3DX12_RESOURCE_DESC::Buffer(
		sizeof(SceneCB)
	));
	mFRFCB = createConstantBuffers(CD3DX12_RESOURCE_DESC::Buffer(
		sizeof(FRFCB)
	));
	mBurstCB = createConstantBuffers(CD3DX12_RESOURCE_DESC::Buffer(
		sizeof(BurstCB)
	));
	mUtilityCB = createConstantBuffers(CD3DX12_RESOURCE_DESC::Buffer(
		sizeof(UtilityCB)
	));
}

void PBLensFlare::constructLensFlareComponents()
{
	mLensDescription.LensInterface.clear();
	mLensDescription.LensInterface.resize(mLensDescription.NumLensComponents);
	mLensDescription.LensInterface.shrink_to_fit();

	const FLOAT2 padding{ 0,0 };
	for (s32 i = mLensDescription.NumLensComponents - 1; i >= 0; --i)
	{
		PatentFormat& component = mLensDescription.LensComponents[i];
		mLensDescription.TotalLensDistance += component.d;

		//first midium is air
		f32 leftRefIndex = (i == 0) ? 1.f : mLensDescription.LensComponents[i - 1].n;
		f32 rightRefIndex = component.n;

		mLensDescription.MinRefIndex
			= (rightRefIndex != 1.f) ? std::min(mLensDescription.MinRefIndex, rightRefIndex) : mLensDescription.MinRefIndex;
		mLensDescription.MaxRefIndex
			= (rightRefIndex != 1.f) ? std::max(mLensDescription.MaxRefIndex, leftRefIndex) : mLensDescription.MaxRefIndex;

		LensInterface lensInterface =
		{
			vec3(0.f, 0.f, mLensDescription.TotalLensDistance - component.radius),
			component.radius,
			vec3(leftRefIndex, 1.0f, rightRefIndex),
			component.sa_h,
			component.c_thickness,
			component.f ? 1.0f : 0.0f,
			padding
		};

		mLensDescription.LensInterface[i] = lensInterface;
	}

	s32 bounceID1 = 2;//1st reflection interface ID
	s32 bounceID2 = 1;//2nd reflection interface ID
	s32 ghostID = 0;
	mLensDescription.GhostData.clear();
	mLensDescription.GhostData.resize(mLensDescription.NumGhosts);
	mLensDescription.GhostData.shrink_to_fit();

	//reflection is 2 times
	while (1)
	{
		if (bounceID1 >= (s32)(mLensDescription.LensInterface.size() - 1))
		{
			bounceID2++;
			bounceID1 = bounceID2 + 1;
		}

		//in range
		if (bounceID2 >= (s32)(mLensDescription.LensInterface.size() - 1)
			|| ghostID >= (s32)(mLensDescription.GhostData.size()))
		{
			break;
		}

		mLensDescription.GhostData[ghostID] =
			FLOAT4
			(
				(f32)bounceID1,
				(f32)bounceID2,
				padding.x, padding.y
			);
		bounceID1++;
		ghostID++;
	}

	const s32 InterfaceCount = mLensDescription.LensInterface.size();
	auto interfaceSize = sizeof(LensInterface);
	auto interfaceBufferSize = appUtility::roundupBufferSize(interfaceSize * InterfaceCount);
	auto interfaceDesc = CD3DX12_RESOURCE_DESC::Buffer(interfaceBufferSize);
	interfaceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	interfaceDesc.Format = DXGI_FORMAT_UNKNOWN;
	D3D12_SHADER_RESOURCE_VIEW_DESC interfaceSRVDesc{};
	interfaceSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	interfaceSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	interfaceSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	interfaceSRVDesc.Buffer.NumElements = InterfaceCount;
	interfaceSRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	interfaceSRVDesc.Buffer.StructureByteStride = interfaceSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC interfaceUAVDesc{};
	interfaceUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	interfaceUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	interfaceUAVDesc.Buffer.NumElements = InterfaceCount;
	interfaceUAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	interfaceUAVDesc.Buffer.StructureByteStride = interfaceSize;
	mLensInterfaceBuffer.createRegularBuffer(
		mHeap, mDevice, mCommandList,
		interfaceDesc, interfaceSRVDesc, interfaceUAVDesc,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, L"LensInterfacesBuffer"
	);
	initBufferByResource(mLensInterfaceBuffer, interfaceBufferSize, mLensDescription.LensInterface.data());

	const s32 ghostDataNum = mLensDescription.GhostData.size();
	auto ghostDataSize = sizeof(GhostData);
	auto ghostDataBufferSize = appUtility::roundupBufferSize(ghostDataSize * ghostDataNum);
	auto ghostDataDesc = CD3DX12_RESOURCE_DESC::Buffer(ghostDataBufferSize);
	ghostDataDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	ghostDataDesc.Format = DXGI_FORMAT_UNKNOWN;
	D3D12_SHADER_RESOURCE_VIEW_DESC ghostDataSRVDesc{};
	ghostDataSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	ghostDataSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	ghostDataSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	ghostDataSRVDesc.Buffer.NumElements = ghostDataNum;
	ghostDataSRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	ghostDataSRVDesc.Buffer.StructureByteStride = ghostDataSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC ghostDataUAVDesc{};
	ghostDataUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	ghostDataUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	ghostDataUAVDesc.Buffer.NumElements = ghostDataNum;
	ghostDataUAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	ghostDataUAVDesc.Buffer.StructureByteStride = ghostDataSize;
	mGhostDataBuffer.createRegularBuffer(
		mHeap, mDevice, mCommandList,
		ghostDataDesc, ghostDataSRVDesc, ghostDataUAVDesc,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, L"GhostDataBuffer"
	);
	initBufferByResource(mGhostDataBuffer, ghostDataBufferSize, mLensDescription.GhostData.data());
}

void  PBLensFlare::constructRayBundle()
{
	s32 subdiv = mLensFlareComputeInformation.GRID_DIV;
	s32 ghostNum = mLensDescription.NumGhosts;

	const f32 left = -1.0f;
	const f32 right = 1.0f;
	const f32 bottom = -1.0f;
	const f32 top = 1.0f;

	std::vector<vec3> vertices;
	std::vector<s32> indices;
	const u32 numVerticesPerGhost = subdiv * subdiv;
	vertices.resize(subdiv * subdiv * ghostNum);
	for (u32 n = 0; n < ghostNum; ++n)
		for (u32 y = 0; y < subdiv; ++y)
			for (u32 x = 0; x < subdiv; ++x)
			{
				const u32 ghostOffset = n * numVerticesPerGhost;
				const u32 vertexIDPerGhost = x + y * subdiv;
				const f32 u = std::lerp(left, right, (f32)x / (f32)(subdiv - 1));
				const f32 v = std::lerp(top, bottom, (f32)y / (f32)(subdiv - 1));
				vertices[ghostOffset + vertexIDPerGhost] = vec3(u, v, 0);
			}

	u32 indexOffset = 0;
	const u32 vertNum = 6;
	const u32 numIndicesPerGhost = (subdiv - 1) * (subdiv - 1) * vertNum;
	indices.resize(numIndicesPerGhost * ghostNum);
	for (u32 n = 0; n < ghostNum; ++n)
		for (u32 y = 0; y < subdiv - 1; ++y)
		{
			for (u32 x = 0; x < subdiv - 1; ++x)
			{
				u32 index = (y * (subdiv - 1) + x) * vertNum + numIndicesPerGhost * n;

				const u32 id0 = indexOffset;
				const u32 id1 = id0 + 1;
				const u32 id2 = id0 + subdiv;
				const u32 id3 = id2 + 1;

				indices[index++] = id2;
				indices[index++] = id0;
				indices[index++] = id1;

				indices[index++] = id1;
				indices[index++] = id3;
				indices[index++] = id2;

				indexOffset++;
			}
			indexOffset++;
		}

	mRayBundle.suvdiv = subdiv;

	auto indexBufferSize = UINT(sizeof(UINT) * indices.size());
	createIndexBuffer(mRayBundle.indexBuffer, indexBufferSize, indices.data());
	mRayBundle.indexBufferView.BufferLocation = mRayBundle.indexBuffer->GetGPUVirtualAddress();
	mRayBundle.indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	mRayBundle.indexBufferView.SizeInBytes = indexBufferSize;

	const s32 verticesNum = vertices.size();
	auto vertexSize = sizeof(PSInput);
	auto vertexBufferSize = appUtility::roundupBufferSize(vertexSize * verticesNum);
	auto vertexDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	vertexDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	vertexDesc.Format = DXGI_FORMAT_UNKNOWN;
	D3D12_SHADER_RESOURCE_VIEW_DESC vertexSRVDesc{};
	vertexSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	vertexSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	vertexSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	vertexSRVDesc.Buffer.NumElements = verticesNum;
	vertexSRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	vertexSRVDesc.Buffer.StructureByteStride = vertexSize;
	D3D12_UNORDERED_ACCESS_VIEW_DESC vertexUAVDesc{};
	vertexUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	vertexUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	vertexUAVDesc.Buffer.NumElements = verticesNum;
	vertexUAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	vertexUAVDesc.Buffer.StructureByteStride = vertexSize;
	mRayBundle.vertexBuffer.createRegularBuffer(
		mHeap, mDevice, mCommandList,
		vertexDesc, vertexSRVDesc, vertexUAVDesc,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"RayBundleVertexBuffer"
	);
}

void PBLensFlare::constructQuad()
{
	using VertexData = std::vector<Vertex>;
	using IndexData = std::vector<UINT>;

	f32 w = mWidth;
	f32 h = mHeight;

	VertexData quadVertices = {
	  { DirectX::XMFLOAT3(-w + 0,  h, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
	  { DirectX::XMFLOAT3(w + 0,  h, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
	  { DirectX::XMFLOAT3(-w + 0, -h, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
	  { DirectX::XMFLOAT3(w  + 0, -h, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
	};
	IndexData  quadIndices = { 0, 1, 2, 3, };

	mStarBurstQuad = createSimpleModel(quadVertices, quadIndices);
	mFullScreenQuad = createSimpleModel(quadVertices, quadIndices);
}

void PBLensFlare::constructBackBuffer()
{
	s32 backBufferWidth = mWidth;
	s32 backBufferHeight = mHeight;

	auto bufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_UNORM, mWidth, mHeight);
	bufferDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = bufferDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = bufferDesc.Format;

	auto cd3dx12HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	HRESULT hr = mDevice->CreateCommittedResource(
		&cd3dx12HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		nullptr,
		IID_PPV_ARGS(&mBackBuffer.texture)
	);
	ThrowIfFailed(hr, "CreateCommittedResource failed.");

	mBackBuffer.srv = mHeap->alloc();

	mDevice->CreateShaderResourceView(
		mBackBuffer.texture.Get(),
		&srvDesc,
		mBackBuffer.srv
	);

	mBackBuffer.rtv = mHeapRTV->alloc();

	mDevice->CreateRenderTargetView(
		mBackBuffer.texture.Get(),
		&rtvDesc,
		mBackBuffer.rtv
	);
}