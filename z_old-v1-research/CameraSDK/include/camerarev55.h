//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> PrimeX 120 Camera / PrimeX 120W<<<
#include "camerarev49.h"

namespace CameraLibrary
{
    class CameraRev55 : public CameraRev49
    {

    public:

        CameraRev55( CameraLibrary::LensType lenstype = CameraLibrary::LensType::STANDARD_LENS, bool needBlockingController = true );

		// Camera overrides
		double ImagerWidth() const override;
		double ImagerHeight() const override;

		void GetDistortionModel(Core::DistortionModel& model) const override;
        int MaximumFrameRateValue() const override;
        bool IsFilterSwitchAvailable() const override;


        unsigned int   GetSubVersion() const;  //== returns the subversion of the camera
		FILE* GetBitStreamFile( unsigned int subVersion ) const; // returns the bitstream for the subVersion
    };
}
