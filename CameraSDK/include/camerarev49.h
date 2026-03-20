//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> PrimeX 41/41W Camera <<<

#include "camerarev29.h"

namespace CameraLibrary
{
    class CameraRev49 : public CameraRev29
    {
    public:

        CameraRev49( CameraLibrary::LensType lenstype = CameraLibrary::LensType::STANDARD_LENS, bool needBlockingController = true );

        int MaximumFrameRateValue() const override;
        bool IsFilterSwitchAvailable() const override;

        bool IsVideoTypeSupported( Core::eVideoMode mode ) const override;

		int   GetSubVersion() const;  //== returns the subversion of the camera
		FILE* GetBitStreamFile( int subVersion ) const; // returns the bitstream for the subVersion
    };
}
