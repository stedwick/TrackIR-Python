//======================================================================================================
// Copyright 2024, NaturalPoint Inc.
//======================================================================================================
#pragma once
//== This is the >>> VersaX 22 Camera <<<

#include "camerarev29.h"

using namespace CameraLibrary;

namespace CameraLibrary
{
	class CameraRev80 : public CameraRev29
	{
	public:
		CameraRev80( CameraBodyStyle bodyStyle = CameraBodyStyle::PRIMEX,
			LensType lensType = LensType::STANDARD_LENS,
			bool needBlockingController = true );

		int MaximumFrameRateValue() const override;
		bool IsFilterSwitchAvailable() const override;
		double ImagerWidth() const override;
		double ImagerHeight() const override;
		void GetDistortionModel( Core::DistortionModel& model ) const override;

		eRinglightType RinglightType() const override;
		int RinglightWavelength() const override;
		eFilterSwitchType FilterSwitchType() const override;
		bool IsIRIlluminationAvailable() const override;

		std::string RinglightSerial() const override;

		int   GetSubVersion() const;  //== returns the subversion of the camera
		FILE* GetBitStreamFile( int subVersion ) const; // returns the bitstream for the subVersion

		
	};
}
