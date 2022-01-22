#pragma once
#include "../common/include/Application.h"
#include "DirectXMath.h"
#include <DirectXPackedVector.h>

#include <array>
#include <utility>
#include <pix3.h>

#define MAX_DUST_IMAGE_SIZE 512
#define LAMBDA_NUM 10//if larger, quality become high, speed become slow
#define LAMBDA_MAX 700
#define LAMBDA_MIN 380

class PBLensFlare : public Application {
public:
	PBLensFlare();

	virtual void render();
	virtual void prepare();
	virtual void cleanup();

	virtual void onSizeChanged(u32 width, u32 height, bool isMinimized);
	virtual void onMouseButtonDown(u32 msg);
	virtual void onMouseButtonUp(u32 msg);
	virtual void onMouseMove(u32 msg, s32 dx, s32 dy);

private:
	using Buffer = ComPtr<ID3D12Resource1>;

	struct Vertex
	{
		FLOAT3 Position;
		FLOAT2 UV;
	};

	struct TextureData
	{
		Texture texture;
		DescriptorHandle SRV;
		DescriptorHandle UAV;
	};

	struct ImageSize
	{
		f32 w;
		f32 h;

		ImageSize() : w(0), h(0)
		{}

		bool operator ==(const ImageSize& rhs) const
		{
			return (w == rhs.w) && (h == rhs.h);
		}

		bool operator !=(const ImageSize& rhs) const
		{
			return (w != rhs.w) || (h != rhs.h);
		}
	};

	struct PSInput
	{
		FLOAT4 pos;
		FLOAT4 drawInfo;
		FLOAT4 coordinates[LAMBDA_NUM];
		FLOAT4 color[LAMBDA_NUM];
	};

	enum GridNum
	{
		GridNum_4x4,
		GridNum_8x8,
		GridNum_16x16,
		GridNum_32x32,
		GridNum_64x64
	};

	struct LensFlareAppInformationForCompute
	{
	    s32 GRID_DIV = 16;
		s32 NUM_THREADS = 16;
		s32 NUM_GROUPS = GRID_DIV / NUM_THREADS;
		s32 NUM_VERTICES_PER_BUNDLES = (GRID_DIV - 1) * (GRID_DIV - 1);
		s32 SAMPLE_LAMBDA_NUM = LAMBDA_NUM;
		f32 LAMBDA_RED = LAMBDA_MAX;
		f32 LAMBDA_BLUE = LAMBDA_MIN;
	};

	struct PatentFormat {
		f32 radius;;//radius of sphere/plane(on xy-plane)   [mm]
		f32 c_thickness; //coating thicknes = lambda / 4 / n1   [nm]
		f32 n;//refractive indices
		f32 sa_h; //nomial radius (from. opt. axis)   [mm]
		f32 d;;//distance between lens   [mm]
		bool  f;//is this interface a plane?
	};

	struct LensInterface {
		vec3 center;//center of sphere/plane on z-axis   [mm]
		f32 radius;//radius of sphere/plane(on xy-plane)   [mm]

		vec3 n;//refractive indices (n0 n1 n2) n0 and n2 are the refractive indices of the first and second media,and n1 the refractive index of an anti-reflective coating.
		f32 sa; //nomial radius (from. opt. axis)   [mm]

		f32 d1; //coating thicknes = lambda / 4 / n1   [nm]
		f32 flat;//is this interface a plane?
		FLOAT2 padding;
	};

	struct RayBundle {
		s32 suvdiv;
		Buffer indexBuffer;
		D3D12_INDEX_BUFFER_VIEW  indexBufferView;
		DX12Buffer vertexBuffer;
	};

	struct tracingCB
	{
		vec3 lightDir;
		f32 spread = 15.0f;

		f32 numInterfaces;
		u32 apertureIndex = 0;
		f32 apertureRadius = 8;
		s32 selectGhostID = 0;
	};

	struct drawBurstCB
	{
		vec3 lightDir;
		f32 apertureRadius = 5.0f;

		FLOAT2 backbufferSize;
		FLOAT2 padding;

		FLOAT3 color;
	};

	struct drawingCB
	{
		f32 ghostScale = 0.02;
		FLOAT2 backbufferSize;
		f32 intensity;
		FLOAT3 color;
	};

	struct FRFCB
	{
		vec3 lambda = vec3(633e-9, 532e-9, 466e-9);
		f32 distance;

		FLOAT2 interval;
		FLOAT2 padding;
	};

	struct BurstCB
	{
		f32 glareIntensity;
		f32 glareLambdaSamplenum;
		FLOAT2 padding;
	};

	struct UtilityCB
	{
		f32 minColOfDustTex;
		f32 rotAngle;
		f32 N;
		f32 apertureRadius;
	};

	struct SceneCB
	{
		MAT4 proj;
		FLOAT2 backbufferSize;
	};

	struct LensDescription {
		const f32 AirN = 1.f;
		// Nikon Lens
		const f32 Nikon_d6 = 53.142f;
		const f32 Nikon_d10 = 7.063f;
		const f32 Nikon_d14 = 1.532f;
		const f32 Nikon_dAp = 2.800f;
		const f32 Nikon_d20 = 16.889f;
		const f32 Nikon_Bf = 39.683f;
		const s32 NikonApertureID = 14;//AP_IDX
		const s32 NikonNumGhosts = 352; // 27!/2*(27-2)!
		const std::vector<PatentFormat> Nikon28_75mm = {//29
			//curvature radius  | c_thickness |  n | sa_h | d | flat
			{    72.747f,  530, 1.60300f, 29.0f, 2.300f, false },
			{    37.000f, 600, AirN, 29.0f, 13.000f, false},

			{  -172.809f,  570, 1.58913f,  26.2f,  2.100f, false},
			{    39.894f,  660, AirN,  26.2f, 1.000f ,false},

			{    49.820f,  330, 1.86074f, 20.0f, 4.400f , false},
			{    74.750f,      544, AirN, 20.0f, Nikon_d6 , false},

			{    63.402f,  740, 1.86074f, 16.1f, 1.600f , false},
			{    37.530f,  411, 1.51680f, 16.1f, 8.600f , false},

			{   -75.887f,  580, 1.80458f, 16.0f, 1.600f , false},
			{   -97.792f,     730, AirN, 16.5f, Nikon_d10 , false},

			{    96.034f,  700, 1.62041f, 18.0f, 3.600f , false},
			{   261.743f,  440, AirN, 18.0f, 0.100f , false},

			{    54.262f,  800, 1.69680f, 18.0f, 6.000f , false},
			{ -5995.277f,     300, AirN, 18.0f, Nikon_d14 , false},

			{       0.0f,     440, AirN,  7.f, Nikon_dAp , true},//aperture
			{   -74.414f,  500, 1.90265f, 13.0f, 2.200f , false},

			{   -62.929f,  770, 1.51680f, 13.0f, 1.450f , false},
			{   121.380f,  820, AirN, 13.1f, 2.500f , false},

			{   -85.723f,  200, 1.49782f, 13.0f, 1.400f , false},
			{    31.093f,  540, 1.80458f, 13.1f, 2.600f , false},

			{    84.758f,     580, AirN, 13.0f, Nikon_d20 , false},
			{   459.690f,  533, 1.86074f, 15.0f, 1.400f , false},

			{    40.240f,  666, 1.49782f, 15.0f, 7.300f , false},
			{   -49.771f,  500, AirN, 15.2f, 0.100f , false},

			{    62.369f,  487, 1.67025f, 16.0f, 7.000f , false},
			{   -76.454f,  671, AirN, 16.0f, 5.200f , false},

			{   -32.524f,  487, 1.80454f, 17.0f, 2.000f , false},
			{   -50.194f,      732, AirN, 17.0f, Nikon_Bf , false},

			{        0.f,     500, AirN, 10.f, 5.f ,  true}
		};

		// Angenieux Lens
		const s32 AngenieuxApertureID = 7;//AP_IDX
		const s32 AngenieuxNumGhosts = 92; // 14!/2*(14-2)!
		const std::vector<PatentFormat> Angenieux = {//16
			//radius       | c_thickness |  n | sa_h | d | flat
			{ 164.13f,     432, 1.67510f, 52.0f, 10.99f , false},
			{ 559.20f,      532, AirN, 52.0f, 0.23f , false},

			{ 100.12f,     382, 1.66890f, 48.0f, 11.45f , false},
			{ 213.54f,      422, AirN, 48.0f, 0.23f , false},

			{ 58.04f,      572, 1.69131f, 36.0f, 22.95f , false},
			{ 2551.10f,     612, 1.67510f,42.0f, 2.58f , false},

			{ 32.39f,      732, AirN, 36.0f, 30.66f , false},
			{ 0.0f,        440, AirN,  7.f, 10.00f , true},//aperture

			{ -40.42f,      602, 1.69920f, 13.0f, 2.74f , false},
			{ 192.98f,     482, 1.62040f, 36.0f, 27.92f , false},

			{ -55.53f,      662, AirN, 36.0f, 0.23f , false},
			{ 192.98f,      332, 1.69131f, 35.0f, 7.98f , false},

			{ -225.30f,     412, AirN, 35.0f, 0.23f , false},
			{ 175.09f,      532, 1.69130f, 35.0f, 8.48f , false},

			{ -203.55f,      632, AirN, 35.0f, 40.f , false},
			{ 0.f,            500, AirN,  5.f, 5.f ,  true}
		};

		//https://j-platpat.inpit.go.jp/p0200 ex 2

		//telescope
		const f32 Canon_d4 = 3.97;
		const f32 Canon_d9 = 14.55;
		const f32 Canon_d11 = 0.99;
		const f32 Canon_d19 = 4.65;
		const f32 Canon_d21 = 6;
		const f32 Canon_d25 = 39.84;
		const f32 Canon_BF = 39.84;

		//middle
		//const f32 Canon_d4 = 31.74;
		//const f32 Canon_d9 = 12.43;
		//const f32 Canon_d11 = 3.11;
		//const f32 Canon_d19 = 1.29;
		//const f32 Canon_d21 = 6.26;
		//const f32 Canon_d25 = 26.79;
		//const f32 Canon_BF = 26.79;

		//wide angle
		//const f32 Canon_d4 = 47.04;
		//const f32 Canon_d9 = 11.59;
		//const f32 Canon_d11 = 3.95;
		//const f32 Canon_d19 = 1.08;
		//const f32 Canon_d21 = 5.86;
		//const f32 Canon_d25 = 21.92;
		//const f32 Canon_BF = 21.92;

		const s32 CanonApertureID = 13;//AP_IDX
		const s32 CanonNumGhosts = 276; // 24!/2*(24-2)!

		const std::vector<PatentFormat> Canon70_200mm = {//26
			//radius | c_thickness |  n | sa_h | d | flat
			{-202,     100, 1.72916f, 30.f, 1.75f ,  false},
			{32.491,     200, AirN, 30.f, 2.31f ,  false},

			{37.259,     100, 1.98612, 30.f, 2.74f ,  false},
			{47.070,     350, AirN, 30.f, Canon_d4 ,  false},

			{40.262,     270, 1.7552, 20, 1 ,  false},
			{35.621,     400, 1.59522, 20, 5.95 ,  false},

			{-80.627,     300, AirN, 20, 0.14 ,  false},
			{26.554,     250, 1.49700, 20, 2.03 ,  false},

			{31.495,     250, AirN, 20, Canon_d9 ,  false},
			{-36.403,     200, 1.80100, 20, 1 ,  false},

			{-1001.069,     120, AirN, 20, Canon_d11 ,  false},
			{38.426,     450, 2.00069, 20, 3.31 ,  false},

			{240.409,     320, AirN, 20, 1.49 ,  false},
			{0,     200, AirN, 20, 1.18 ,  true},//aperture

			{249.387,     130, 1.85478, 20, 1.15 ,  false},
			{21.119,     120, 1.49700, 20, 7.31 ,  false},

			{-65.298,     410, AirN, 20, 0.15 ,  false},
			{29.334,     310, 1.77250, 20, 5.03 ,  false},

			{-163.635,     290, AirN, 20, Canon_d19 ,  false},
			{-325.599,     300, 1.72825, 20, 0.95 ,  false},

			{33.614,     250, AirN, 20, Canon_d21 ,  false},
			{-27.658,     300, 1.48749, 20, 1.30 ,  false},

			{125.276,     100, AirN, 20, 0.13 ,  false},
			{59.799,     150, 1.98612, 20, 2.24 ,  false},

			{-690.239,     100, AirN, 20, Canon_d25 ,  false},
			{ 0.f,            500, AirN,  5.f, 5.f ,  true}
		};

		std::vector<LensInterface> LensInterface;
		std::vector<GhostData> GhostData;

		std::vector<PatentFormat> LensComponents = Nikon28_75mm;
		s32 APERTURE_IDX = NikonApertureID;
		s32 NumGhosts = NikonNumGhosts;

		s32 NumLensComponents = (s32)LensComponents.size();

		f32 TotalLensDistance = 0.f;
		f32 MaxRefIndex = -1000.f;
		f32 MinRefIndex = 1000.f;
	};

	enum ShaderNameCompute
	{
		ShaderNameCompute_TraceRay,
		ShaderNameCompute_TraceRayAR,

		ShaderNameCompute_FRF,

		ShaderNameCompute_FFTcol,
		ShaderNameCompute_FFTrow,
		ShaderNameCompute_InvFFTcol,
		ShaderNameCompute_InvFFTrow,

		ShaderNameCompute_RaiseValue,
		ShaderNameCompute_LambdaIntegral,
		ShaderNameCompute_BurstFilter,

		ShaderNameCompute_ComplexMultiply,
		ShaderNameCompute_Multiply,
		ShaderNameCompute_TextureWhitening,
		ShaderNameCompute_Copy,
		ShaderNameCompute_Clear,
		ShaderNameCompute_Polygon,

		ShaderNameCompute_Amplitude,
		ShaderNameCompute_Intensity,

		ShaderNameCompute_OneElem,

		ShaderNameCompute_COUNT
	};

	enum ShaderNameGraphics
	{
		ShaderNameGraphics_AddGhosts,
		ShaderNameGraphics_AddGhostsWireFrame,
		ShaderNameGraphics_AddGhostsUV,
		SahderNameGraphics_AddStarBurst,
		ShaderNameGraphics_ToneMapper,
		ShaderNameGraphics_COUNT
	};

	enum ReadOmlyImage
	{
		ReadOmlyImage_ApertureDust,
		ReadOnlyImage_COUNT
	};

	enum DisplayImage
	{
		DisplayImage_Aperture,
		DisplayImage_COUNT
	};

	enum LensType
	{
		LensType_NIKON,
		LensType_ANGENIEUX,
		LensType_CANON,
		LensType_COUNT
	};

	enum DrawMOde
	{
		DrawMode_Mesh,
		DrawMode_Wire,
		DrawMode_OverrideWire,
		DrawMode_UV,
		DrawMode_COUNT
	};

	bool mRecompile = false;
	bool mImageUpdate = false;

	std::vector<std::string> mTextureNames;

	bool mGhostKernelRegenerate = true;
	bool mBurstKernelRegenerate = true;
	bool mRebuildComponent = true;
	bool mDivNunmChanged = false;
	bool mTraceRequired = true;
	bool mCBForceUpdate = false;

	//For Flare
	LensDescription mLensDescription;
	LensFlareAppInformationForCompute mLensFlareComputeInformation;
	RayBundle mRayBundle;

	//Constant Buffer
	std::vector<Buffer> mTracingCB;
	std::vector<Buffer> mDrawBurstCB;
	std::vector<Buffer> mDrawingCB;
	std::vector<Buffer> mFRFCB;
	std::vector<Buffer> mBurstCB;
	std::vector<Buffer> mUtilityCB;
	std::vector<Buffer> mLensFlareSceneCB;

	//CB Cache
	tracingCB mCacheTracingCBStruct;
	drawBurstCB mCacheDrawBurstCBStruct;
	drawingCB mCacheDrawingCBStruct;
	FRFCB mCacheFRFCBStruct;
	BurstCB mCacheBurstCBStruct;
	UtilityCB mCacheUtilityCBStruct;
	SceneCB mCacheSceneCBStruct;

	//For RayTracing
	DX12Buffer mLensInterfaceBuffer;
	DX12Buffer mGhostDataBuffer;

	//Target Polygon
	ModelData mStarBurstQuad;
	ModelData mFullScreenQuad;

	BackBuffer mBackBuffer;

	//Wave Distribution Texture Cache
	DX12Buffer mGhostCachedTex;
	DX12Buffer mBurstCachedTex;

	//Texture Resource
	std::vector<TextureData> mFullsizeTex;
	std::vector<DX12Buffer> mRWFullsizeTex;
	std::vector<DX12Buffer> mRWdisplayTex;
	std::vector<DX12Buffer> mRWfullsizeInnerTex;

	//preparation
	void setupWorkingTexture();
	void setupSimulateLensFlarePipeline();
	void setupComputePipeline();
	void setupGraphicsPipeline();
	void constructConstantBufferForLensFlare();
	void constructLensFlareComponents();
	void constructRayBundle();
	void constructQuad();
	void constructBackBuffer();
	void setupRayTraceLensFlare();
	void executeRayTracingLensFlareCommand();

	//computation
	void drawFRF(DX12Buffer& Real, DX12Buffer& Image);
	void FFT2D(DX12Buffer& Real, DX12Buffer& Image);
	void invFFT2D(DX12Buffer& Real, DX12Buffer& Image);
	void raiseValue(DX12Buffer& InReal, DX12Buffer& InImage, DX12Buffer& OutReal, DX12Buffer& OutImage);
	void lambdaIntegral(DX12Buffer& InReal, DX12Buffer& InImage, DX12Buffer& OutReal, DX12Buffer& OutImage);
	void burstFiltering(DX12Buffer& In, DX12Buffer& Out);
	void multiply(DX12Buffer& In1, DX12Buffer& In2, DX12Buffer& Out);
	void complexMultiply(DX12Buffer& InReal0, DX12Buffer& InImage0, DX12Buffer& InReal1, DX12Buffer& InImage1, DX12Buffer& OutReal, DX12Buffer& OutImage);
	void whitening(DX12Buffer& In, DX12Buffer& Out);
	void copy(TextureData& In, DX12Buffer& Out);
	void copy(DX12Buffer& In, DX12Buffer& Out);
	void clear(DX12Buffer& Tex);
	void polygon(DX12Buffer& Tex);
	void amplitude(DX12Buffer& InReal, DX12Buffer& InImage, DX12Buffer& OutReal, DX12Buffer& OutImage);
	void intensity(DX12Buffer& InReal, DX12Buffer& InImage, DX12Buffer& OutReal, DX12Buffer& OutImage);
	void traceRay();
	void addGhosts(bool wireFrame = false);
	void addGhostsUV();
	void addBurst();
	void setupDrawing(DescriptorHandle rtv, DescriptorHandle dsv);
	void oneElem(DX12Buffer& In, DX12Buffer& Out);
	void drawLensFlare();

	//generation
	void generateBurst();
	void generateGhost();

	//helper
	s32 inv2pow(s32 n);
	TextureData LoadTextureFromFile(const std::wstring& name, ImageSize& size, bool& isValid);

	//update
	void updateBuffer();
	void updateLens();
	void updateComputeInfo();
	void updateSourceImage();

	void draw();

	//UI
	void renderHUD();
	void displayParameters();
	void displayImageImGui(D3D12_GPU_DESCRIPTOR_HANDLE handle, const char* tag = nullptr);
	void buttomAction();

	bool mIsDragged;
	s32 mButtonType;

	u32 mTexwidth;
	u32 mTexheight;
	u32 mGhostTexWidth;
	u32 mGhostTexHeight;

	s32 mDrawMode;
	s32 mLensType;

	//param
	f32 mSpread;
	f32 mGhostScale;
	f32 mApertureRadius;
	f32 mGhostIntensityRatio;
	FLOAT3 mColor;
	s32 mApertureBladeNum;
	f32 mRotAngle;
	f32 mIntensity;
	f32 mBurgstGlitter;
	s32 mSelectGhostID;

	//internal param
	f32 mPropdistance;
	f32 mIntervalX;
	f32 mPosX;
	f32 mPosY;
	f32 mPosXSave;
	f32 mPosYSave;
	s32 mGlarelambdasamplenum;
	
	bool mUseAR;

	f32 mExecuteTimeMS;

	GridNum mGridNum;

	bool mFFTEnable;
};