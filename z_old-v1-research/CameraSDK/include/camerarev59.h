//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> SlimX 41 / SlimX 41W Camera <<<

#include "camerarev49.h"

namespace CameraLibrary
{
    class CameraRev54 : public CameraRev49
    {
    public:
        CameraRev54(CameraLibrary::LensType lenstype = CameraLibrary::LensType::STANDARD_LENS, bool needBlockingController = true );

		int MaximumFrameRateValue() const override;
		bool IsFilterSwitchAvailable() const override;
		eRinglightType RinglightType() const override;
		int   GetSubVersion() const;  //== returns the subversion of the camera
		FILE* GetBitStreamFile(int subVersion) const; // returns the bitstream for the subVersion
    };
}
