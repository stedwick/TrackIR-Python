//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> Prime 17W Camera <<<

#include "camerarev29.h"

namespace CameraLibrary
{
    class CameraRev30 : public CameraRev29
    {
    public:
        CameraRev30( bool needBlockingController = true );

        int MaximumFrameRateValue() const override;
        int MaximumFullImageFrameRateValue() const override;
        double ImagerWidth() const override;
        double ImagerHeight() const override;
        void GetDistortionModel( Core::DistortionModel& model ) const override;
        bool IsMJPEGAvailable() const override { return true; }

    };
}
