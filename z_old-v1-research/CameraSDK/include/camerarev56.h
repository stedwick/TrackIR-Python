//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> slimX 120 ( rev56 )/ slimX 120W Camera ( rev64 )<<<
#include "camerarev49.h"

namespace CameraLibrary
{
	class CameraRev56 : public CameraRev49
	{
	public:
		CameraRev56(CameraLibrary::LensType lenstype = CameraLibrary::LensType::STANDARD_LENS, bool needBlockingController = true);

		// Camera overrides
		double ImagerWidth() const override;
		double ImagerHeight() const override;

		void GetDistortionModel(Core::DistortionModel& model) const override;
		int MaximumFrameRateValue() const override;
		bool IsFilterSwitchAvailable() const override;
		eRinglightType RinglightType() const override;

		unsigned int   GetSubVersion() const;  //== returns the subversion of the camera
		FILE* GetBitStreamFile(unsigned int subVersion) const; // returns the bitstream for the subVersion
	};
}
