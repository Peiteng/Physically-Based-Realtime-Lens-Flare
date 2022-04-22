#include "../include/PBLensFlare.h"

f32 PBLensFlare::computeFlareMappingSize(const u32 resolution, const f32 standardLambda, const f32 distApToSencor, const f32 apRadius)
{
	return resolution * standardLambda * distApToSencor / apRadius;
}

void PBLensFlare::updateBuffers()
{
	vec3 lightDir = normalize(vec3(-(mPosX * 2.f - 1.f), mPosY * 2.f - 1.f, -1.f));

	const f32 aspect = (f32)mScreenSize.w / (f32)mScreenSize.h;

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
	drawBurstCB.aspect = aspect;
	drawBurstCB.mappingScale = computeFlareMappingSize(MAX_DUST_IMAGE_SIZE, 633e-9, mLensDescription.distApToSencor * 1e-3, mApertureRadius * 2 * 1e-3) / (mLensDescription.sensorSize * 1e-3);
	drawBurstCB.color = mColor;

	drawingCB drawingCB;
	drawingCB.aspect = aspect;
	drawingCB.ghostScale = mGhostScale;
	drawingCB.intensity = mIntensity * mGhostIntensityRatio;
	drawingCB.color = mColor;

	FRFCB frfCB;
	frfCB.distance = mPropdistance * 1e-3;
	//frfCB.distance = 0;
	frfCB.interval = FLOAT2(mIntervalX * 1e-6, mIntervalX * 1e-6 * mTexwidth / mTexheight);
	frfCB.interval.x *= MAX_DUST_IMAGE_SIZE / mTexwidth;
	frfCB.interval.y *= MAX_DUST_IMAGE_SIZE / mTexheight;

	BurstCB burstCB;
	burstCB.glareIntensity = mIntensity;

	UtilityCB utilityCB;
	utilityCB.minColOfDustTex = 1 - mBurgstGlitter;
	utilityCB.N = mApertureBladeNum;
	utilityCB.apertureRatio = mApertureRadius / mLensDescription.maxApertureRadius;
	utilityCB.rotAngle = mRotAngle;

	SceneCB sceneCB;
	auto mtxProj = DirectX::XMMatrixOrthographicLH(mScreenSize.w, mScreenSize.h, 0.0f, 10.0f);
	XMStoreFloat4x4(&sceneCB.proj, XMMatrixTranspose(mtxProj));
	sceneCB.backbufferSize = FLOAT2(mScreenSize.w, mScreenSize.h);

	if (mCBForceUpdate)
	{
		updateBuffer(mTracingCB[0].Get(), sizeof(tracingCB), &tracingCB);
		updateBuffer(mDrawBurstCB[0].Get(), sizeof(drawBurstCB), &drawBurstCB);
		updateBuffer(mDrawingCB[0].Get(), sizeof(drawingCB), &drawingCB);
		updateBuffer(mFRFCB[0].Get(), sizeof(frfCB), &frfCB);
		updateBuffer(mBurstCB[0].Get(), sizeof(burstCB), &burstCB);
		updateBuffer(mUtilityCB[0].Get(), sizeof(utilityCB), &utilityCB);
		updateBuffer(mLensFlareSceneCB[0].Get(), sizeof(sceneCB), &sceneCB);
		mCBForceUpdate = false;

		mCacheTracingCBStruct = tracingCB;
		mCacheDrawBurstCBStruct = drawBurstCB;
		mCacheDrawingCBStruct = drawingCB;
		mCacheFRFCBStruct = frfCB;
		mCacheBurstCBStruct = burstCB;
		mCacheUtilityCBStruct = utilityCB;
		mCacheSceneCBStruct = sceneCB;
	}
	else
	{
		if (memcmp(&tracingCB, &mCacheTracingCBStruct, sizeof(tracingCB)))
		{
			updateBuffer(mTracingCB[0].Get(), sizeof(tracingCB), &tracingCB);
			mCacheTracingCBStruct = tracingCB;
			mTraceRequired = true;
		}
		if (memcmp(&drawBurstCB, &mCacheDrawBurstCBStruct, sizeof(drawBurstCB)))
		{
			updateBuffer(mDrawBurstCB[0].Get(), sizeof(drawBurstCB), &drawBurstCB);
			mCacheDrawBurstCBStruct = drawBurstCB;
		}
		if (memcmp(&drawingCB, &mCacheDrawingCBStruct, sizeof(drawingCB)))
		{
			updateBuffer(mDrawingCB[0].Get(), sizeof(drawingCB), &drawingCB);
			mCacheDrawingCBStruct = drawingCB;
		}
		if (memcmp(&frfCB, &mCacheFRFCBStruct, sizeof(frfCB)))
		{
			updateBuffer(mFRFCB[0].Get(), sizeof(frfCB), &frfCB);
			mCacheFRFCBStruct = frfCB;
		}
		if (memcmp(&burstCB, &mCacheBurstCBStruct, sizeof(burstCB)))
		{
			updateBuffer(mBurstCB[0].Get(), sizeof(burstCB), &burstCB);
			mCacheBurstCBStruct = burstCB;
		}
		if (memcmp(&utilityCB, &mCacheUtilityCBStruct, sizeof(utilityCB)))
		{
			updateBuffer(mUtilityCB[0].Get(), sizeof(utilityCB), &utilityCB);
			mCacheUtilityCBStruct = utilityCB;
		}
		if (memcmp(&sceneCB, &mCacheSceneCBStruct, sizeof(sceneCB)))
		{
			updateBuffer(mLensFlareSceneCB[0].Get(), sizeof(sceneCB), &sceneCB);
			mCacheSceneCBStruct = sceneCB;
		}
	}
}

void PBLensFlare::updateLens()
{
	switch (mLensType)
	{
	case LensType_NIKON:
		mLensDescription.LensComponents = mLensDescription.Nikon28_75mm;
		mLensDescription.APERTURE_IDX = mLensDescription.Nikon28_75mm_ApertureID;
		mLensDescription.NumGhosts = mLensDescription.Nikon28_75mm_NumGhosts;
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
		mLensDescription.APERTURE_IDX = mLensDescription.Nikon28_75mm_ApertureID;
		mLensDescription.NumGhosts = mLensDescription.Nikon28_75mm_NumGhosts;
		break;
	}
}

void PBLensFlare::updateComputeInfo()
{
	switch (mGridNum)
	{
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