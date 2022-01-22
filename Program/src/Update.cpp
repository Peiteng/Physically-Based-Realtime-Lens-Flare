#include "../include/PBLensFlare.h"

void PBLensFlare::updateBuffer()
{
	f32 nx = mPosX * 2.f - 1.f;
	f32 ny = mPosY * 2.f - 1.f;

	vec3 lightDir = normalize(vec3(-nx, ny, -1.f));
	FLOAT2 backBufferSize = FLOAT2(mWidth, mHeight);

	tracingCB tracingCB;
	tracingCB.lightDir = lightDir;
	tracingCB.numInterfaces = mLensDescription.LensInterface.size();
	tracingCB.spread = mSpread;
	tracingCB.apertureIndex = mLensDescription.APERTURE_IDX;
	tracingCB.apertureRadius = mApertureRadius;
	tracingCB.selectGhostID = mSelectGhostID;
	tracingCB.invisibleReflectance = mInvisibleReflectance;

	drawBurstCB drawBurstCB;
	drawBurstCB.lightDir = lightDir;
	drawBurstCB.backbufferSize = backBufferSize;
	drawBurstCB.apertureRadius = (100 - mApertureRadius * mApertureRadius + 0.1) * 0.05;
	drawBurstCB.color = mColor;

	drawingCB drawingCB;
	drawingCB.backbufferSize = backBufferSize;
	drawingCB.ghostScale = mGhostScale;
	drawingCB.intensity = mIntensity * mGhostIntensityRatio;
	drawingCB.color = mColor;

	FRFCB frfCB;
	frfCB.distance = mPropdistance / 1000;
	frfCB.interval = FLOAT2(mIntervalX * 1e-6, mIntervalX * 1e-6 * mTexwidth / mTexheight);
	frfCB.interval.x *= 512.0f / mTexwidth;
	frfCB.interval.y *= 512.0f / mTexheight;

	BurstCB burstCB;
	burstCB.glareIntensity = mIntensity;
	burstCB.glareLambdaSamplenum = mGlarelambdasamplenum;

	UtilityCB utilityCB;
	utilityCB.minColOfDustTex = 1 - mBurgstGlitter;
	utilityCB.N = mApertureBladeNum;
	utilityCB.apertureRadius = mApertureRadius / 10.f;
	utilityCB.rotAngle = mRotAngle;

	SceneCB sceneCB;
	auto mtxProj = DirectX::XMMatrixOrthographicLH(backBufferSize.x, backBufferSize.y, 0.0f, 10.0f);
	XMStoreFloat4x4(&sceneCB.proj, XMMatrixTranspose(mtxProj));
	sceneCB.backbufferSize = backBufferSize;

	if (mCBForceUpdate)
	{
		writeToUploadHeapMemory(mTracingCB[0].Get(), sizeof(tracingCB), &tracingCB);
		writeToUploadHeapMemory(mDrawBurstCB[0].Get(), sizeof(drawBurstCB), &drawBurstCB);
		writeToUploadHeapMemory(mDrawingCB[0].Get(), sizeof(drawingCB), &drawingCB);
		writeToUploadHeapMemory(mFRFCB[0].Get(), sizeof(frfCB), &frfCB);
		writeToUploadHeapMemory(mBurstCB[0].Get(), sizeof(burstCB), &burstCB);
		writeToUploadHeapMemory(mUtilityCB[0].Get(), sizeof(utilityCB), &utilityCB);
		writeToUploadHeapMemory(mLensFlareSceneCB[0].Get(), sizeof(sceneCB), &sceneCB);
		mCBForceUpdate = false;
	}
	else
	{
		if (memcmp(&tracingCB, &mCacheTracingCBStruct, sizeof(tracingCB)))
		{
			writeToUploadHeapMemory(mTracingCB[0].Get(), sizeof(tracingCB), &tracingCB);
			mCacheTracingCBStruct = tracingCB;
		}
		if (memcmp(&drawBurstCB, &mCacheDrawBurstCBStruct, sizeof(drawBurstCB)))
		{
			writeToUploadHeapMemory(mDrawBurstCB[0].Get(), sizeof(drawBurstCB), &drawBurstCB);
			mCacheDrawBurstCBStruct = drawBurstCB;
		}
		if (memcmp(&drawingCB, &mCacheDrawingCBStruct, sizeof(drawingCB)))
		{
			writeToUploadHeapMemory(mDrawingCB[0].Get(), sizeof(drawingCB), &drawingCB);
			mCacheDrawingCBStruct = drawingCB;
		}
		if (memcmp(&frfCB, &mCacheFRFCBStruct, sizeof(frfCB)))
		{
			writeToUploadHeapMemory(mFRFCB[0].Get(), sizeof(frfCB), &frfCB);
			mCacheFRFCBStruct = frfCB;
		}
		if (memcmp(&burstCB, &mCacheBurstCBStruct, sizeof(burstCB)))
		{
			writeToUploadHeapMemory(mBurstCB[0].Get(), sizeof(burstCB), &burstCB);
			mCacheBurstCBStruct = burstCB;
		}
		if (memcmp(&utilityCB, &mCacheUtilityCBStruct, sizeof(utilityCB)))
		{
			writeToUploadHeapMemory(mUtilityCB[0].Get(), sizeof(utilityCB), &utilityCB);
			mCacheUtilityCBStruct = utilityCB;
		}
		if (memcmp(&sceneCB, &mCacheSceneCBStruct, sizeof(sceneCB)))
		{
			writeToUploadHeapMemory(mLensFlareSceneCB[0].Get(), sizeof(sceneCB), &sceneCB);
			mCacheSceneCBStruct = sceneCB;
		}
	}


	if (mPosXSave != mPosX || mPosYSave != mPosY)
	{
		mTraceRequired = true;
	}
}

void PBLensFlare::updateLens()
{
	switch (mLensType)
	{
	case LensType_NIKON:
		mLensDescription.LensComponents = mLensDescription.Nikon28_75mm;
		mLensDescription.APERTURE_IDX = mLensDescription.NikonApertureID;
		mLensDescription.NumGhosts = mLensDescription.NikonNumGhosts;
		break;
	case LensType_ANGENIEUX:
		mLensDescription.LensComponents = mLensDescription.Angenieux;
		mLensDescription.APERTURE_IDX = mLensDescription.AngenieuxApertureID;
		mLensDescription.NumGhosts = mLensDescription.AngenieuxNumGhosts;
		break;
	case LensType_CANON:
		mLensDescription.LensComponents = mLensDescription.Canon70_200mm;
		mLensDescription.APERTURE_IDX = mLensDescription.CanonApertureID;
		mLensDescription.NumGhosts = mLensDescription.CanonNumGhosts;
		break;
	default:
		mLensDescription.LensComponents = mLensDescription.Nikon28_75mm;
		mLensDescription.APERTURE_IDX = mLensDescription.NikonApertureID;
		mLensDescription.NumGhosts = mLensDescription.NikonNumGhosts;
		break;
	}
	mLensDescription.NumLensComponents = (s32)mLensDescription.LensComponents.size();
}

void PBLensFlare::updateComputeInfo()
{
	switch (mGridNum)
	{
	case GridNum_4x4:
		mLensFlareComputeInformation.GRID_DIV = 4;
		mLensFlareComputeInformation.NUM_THREADS = 4;
		break;
	case GridNum_8x8:
		mLensFlareComputeInformation.GRID_DIV = 8;
		mLensFlareComputeInformation.NUM_THREADS = 8;
		break;
	case GridNum_16x16:
		mLensFlareComputeInformation.GRID_DIV = 16;
		mLensFlareComputeInformation.NUM_THREADS = 16;
		break;
	case GridNum_32x32:
		mLensFlareComputeInformation.GRID_DIV = 32;
		mLensFlareComputeInformation.NUM_THREADS = 16;
		break;
	case GridNum_64x64:
		mLensFlareComputeInformation.GRID_DIV = 64;
		mLensFlareComputeInformation.NUM_THREADS = 16;
		break;
	default:
		mLensFlareComputeInformation.GRID_DIV = 16;
		mLensFlareComputeInformation.NUM_THREADS = 16;
		break;
	}

	mLensFlareComputeInformation.NUM_GROUPS = mLensFlareComputeInformation.GRID_DIV / mLensFlareComputeInformation.NUM_THREADS;
	mLensFlareComputeInformation.NUM_VERTICES_PER_BUNDLES = (mLensFlareComputeInformation.GRID_DIV - 1) * (mLensFlareComputeInformation.GRID_DIV - 1);
}