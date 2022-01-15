#define _CRT_SECURE_NO_WARNINGS

#include "../include/PBLensFlare.h"

#include "../../common/imgui/imgui.h"
#include "../../common/imgui/examples/imgui_impl_dx12.h"
#include "../../common/imgui/examples/imgui_impl_win32.h"

#include "sstream"
#include <algorithm>
using namespace std;

#define IMVEC_RED ImVec4(1,0,0,1)
#define IMVEC_GREEN ImVec4(0,1,0,1)
#define IMVEC_BLUE ImVec4(0,0,1,1)
#define IMVEC_YELLOW ImVec4(1,1,0,1)

std::string unit(u32 byte)
{
	const u32 KiB = 1024;
	const std::string unit[4] = {"B", "KB", "MB", "GB"};

	u32 count = 0;

	byte /= KiB;

	while (1)
	{
		count++;

		if ((u32)(byte / KiB) == 0 || count == 3)
		{
			break;
		}
		else
		{
			byte /= KiB;
		}
	}

	std::ostringstream oss;
	oss << byte << " " << unit[count];

	return oss.str();
}

void PBLensFlare::onMouseButtonDown(UINT msg)
{
	auto io = ImGui::GetIO();
	if (io.WantCaptureMouse)
	{
		return;
	}

	mIsDragged = true;
	mButtonType = msg;
}
void PBLensFlare::onMouseButtonUp(UINT msg) {
	mIsDragged = false;
}

void PBLensFlare::onMouseMove(UINT msg, int dx, int dy)
{
	auto io = ImGui::GetIO();
	if (io.WantCaptureMouse)
	{
		return;
	}

	if (!mIsDragged)
	{
		return;
	}

	if (mButtonType == 0)
	{
		mPosX += dx * 0.001f;
		mPosY += dy * 0.001f * mWidth / mHeight;

		f32 center = 0.5;
		f32 left = center - 1;
		f32 right = center + 1;
		f32 bottom = center - 1;
		f32 top = center + 1;

		mPosX = clamp(mPosX, left, right);
		mPosY = clamp(mPosY, bottom, top);
	}
}

void PBLensFlare::onSizeChanged(UINT width, UINT height, bool isMinimized)
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

	mViewport.Width = f32(mWidth);
	mViewport.Height = f32(mHeight);
	mScissorRect.right = mWidth;
	mScissorRect.bottom = mHeight;
}

void PBLensFlare::renderHUD()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	auto milisec = 1000.0f / ImGui::GetIO().Framerate;
	ImGui::Begin("Information", nullptr, ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Action"))
		{
			if (ImGui::MenuItem("Shader Recompile")) {
				mRecompile = true;
				mTraceRequired = true;
			}
			if (ImGui::MenuItem("Source Image Update")) {
				mImageUpdate = true;
				mGhostKernelRegenerate = true;
				mBurstKernelRegenerate = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	ImGui::Text("ScreenSize: W %d px   H %d px", mWidth, mHeight);
	ImGui::Text("Framerate: %.3f fps (%.3f ms)", ImGui::GetIO().Framerate, milisec);

	if (ImGui::CollapsingHeader("Compute Infomation"))
	{
		ImGui::Text("GridNum: %d x %d", mLensFlareComputeInformation.GRID_DIV, mLensFlareComputeInformation.GRID_DIV);
		ImGui::Text("GhostNum: %d", mLensDescription.NumGhosts);
		ImGui::Text("VertexNum: %d", mLensDescription.NumGhosts * mLensFlareComputeInformation.GRID_DIV * mLensFlareComputeInformation.GRID_DIV);
		ImGui::Text("SampledLambdaNum: %d(From %.0f nm to %.f nm)", mLensFlareComputeInformation.SAMPLE_LAMBDA_NUM, mLensFlareComputeInformation.LAMBDA_RED, mLensFlareComputeInformation.LAMBDA_BLUE);
	}

	u32 vertexByte = mLensDescription.NumGhosts * mLensFlareComputeInformation.GRID_DIV * mLensFlareComputeInformation.GRID_DIV * sizeof(vec3);
	u32 indexByte = mLensDescription.NumGhosts * (mLensFlareComputeInformation.GRID_DIV - 1) * (mLensFlareComputeInformation.GRID_DIV - 1) * 6 * sizeof(u32);
	u32 componentByte = mLensDescription.LensComponents.size() * sizeof(PatentFormat);
	u32 interfaceByte = mLensDescription.LensInterface.size() * sizeof(LensInterface);
	u32 ghostByte = mLensDescription.GhostData.size() * sizeof(GhostData);

	if (ImGui::CollapsingHeader("Memory Usage"))
	{
		ImGui::Text("Vertex Usage: %s", unit(vertexByte).c_str());
		ImGui::Text("Index Usage: %s", unit(indexByte).c_str());
		ImGui::Text("Component Usage: %s", unit(componentByte).c_str());
		ImGui::Text("Interface Usage: %s", unit(interfaceByte).c_str());
		ImGui::Text("Ghost Bounce Usage: %s", unit(ghostByte).c_str());
	}

	ImGui::Text("Pos: X %f   Y %f", mPosX, mPosY);


	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Images [.png]"))
	{
		ImGui::InputText("Aperture Dust Image", mTextureNames[0].data(), MAX_PATH);

		if (!mFFTEnable)
		{
			ImGui::TextColored(IMVEC_RED, "Aperture Dust Image Name Is Invalid");
			ImGui::TextColored(IMVEC_YELLOW, "File Must be Exist or Image Size Must be power of 2 and 512 or less");
		}
	}
	displayParameters();
	ImGui::Spacing();

	ImGui::End();

	displayImageImGui(D3D12_GPU_DESCRIPTOR_HANDLE(mRWdisplayTex.at(DisplayImage_Aperture).getSRV(mCommandList)), "apertureImage");

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mCommandList.Get());
}

void PBLensFlare::displayParameters()
{
	if (ImGui::CollapsingHeader("PARAMETERS"))
	{
		if (ImGui::CollapsingHeader("SETTINGS"))
		{
			ImGui::Combo("Draw Mode", (s32*)&mDrawMode,
				"Fill\0Wire\0OverwriteWire\0UV\0");
		}
	
		if (ImGui::CollapsingHeader("ADJUSTMENT"))
		{
			if (ImGui::SliderFloat("Aperture Opening [mm]", &mApertureRadius, 0.0f, 10.0f))
			{
				mGhostKernelRegenerate = true;
				mBurstKernelRegenerate = true;
			}
			if (ImGui::SliderInt("Aperture Num", &mApertureBladeNum, 5, 10))
			{
				mGhostKernelRegenerate = true;
				mBurstKernelRegenerate = true;
			}
			if (ImGui::SliderFloat("RotAngle [deg]", &mRotAngle, 0.0f, 360.0f))
			{
				mGhostKernelRegenerate = true;
				mBurstKernelRegenerate = true;
			}
			if (ImGui::SliderFloat("Intensity", &mIntensity, 0.1f, 300.0f))
			{
				mBurstKernelRegenerate = true;
			}
			ImGui::ColorEdit3("BaseColor", &mColor.x);

			if (ImGui::CollapsingHeader("LENS"))
			{
				if (ImGui::Combo("Lens Type", (s32*)&mLensType,
					"Nikon\0Angenieux\0Canon\0"))
				{
					mRebuildComponent = true;
					mTraceRequired = true;
					mCBForceUpdate = true;
				}
				ImGui::Checkbox("Use AR Coating", &mUseAR);
			}
			if (ImGui::CollapsingHeader("BUNDLE"))
			{
				if (ImGui::Combo("Grid Num", (s32*)&mGridNum,
					"_4x4\0_8x8\0_16x16\0_32x32\0_64x64\0"))
				{
					mDivNunmChanged = true;
					mTraceRequired = true;
				}
				ImGui::SliderFloat("Bundle Spread", &mSpread, 0.0f, 50.0f);
			}
			if (ImGui::CollapsingHeader("GHOST"))
			{
				ImGui::SliderFloat("Scale", &mGhostScale, 0, 0.1);
				ImGui::SliderFloat("Intensity Ratio", &mGhostIntensityRatio, 0.0f, 10000.0f);
			}
			if (ImGui::CollapsingHeader("BURST"))
			{
				if (ImGui::SliderFloat("Dust Effect", &mBurgstGlitter, 0.0f, 1.0f))
				{
					mGhostKernelRegenerate = true;
					mBurstKernelRegenerate = true;
				}
			}
		}
		
	}
}

void PBLensFlare::displayImageImGui(D3D12_GPU_DESCRIPTOR_HANDLE handle, const char* tag)
{
	if (tag == nullptr)
	{
		tag = "image";
	}
	ImGui::SetNextWindowSize(ImVec2(mTexwidth / 2, mTexheight / 2), ImGuiCond_Once);
	ImGui::Begin(tag, NULL);
	{
		ImVec2 avail_size = ImGui::GetContentRegionAvail();
		ImGui::Image((void*)handle.ptr, avail_size);
	}
	ImGui::End();
}

void PBLensFlare::buttomAction()
{
	if (mRecompile)
	{
		waitForIdleGPU();

		setupWorkingTexture();
		setupComputePipeline();
		setupGraphicsPipeline();

		mBurstKernelRegenerate = true;
		mGhostKernelRegenerate = true;
		mRecompile = false;
	}
	if (mImageUpdate)
	{
		waitForIdleGPU();
		updateSourceImage();

		if (mRecompile)
		{
			setupWorkingTexture();
			setupComputePipeline();
			setupGraphicsPipeline();
			mRecompile = false;
		}

		mBurstKernelRegenerate = true;
		mGhostKernelRegenerate = true;
		mImageUpdate = false;
	}
	if (mRebuildComponent)
	{
		waitForIdleGPU();

		updateLens();
		constructLensFlareComponents();
		constructRayBundle();
		mRebuildComponent = false;
	}
	if (mDivNunmChanged)
	{
		waitForIdleGPU();

		updateComputeInfo();
		constructLensFlareComponents();
		constructRayBundle();

		setupWorkingTexture();
		setupComputePipeline();
		setupGraphicsPipeline();

		mBurstKernelRegenerate = true;
		mGhostKernelRegenerate = true;
		mDivNunmChanged = false;
	}
}