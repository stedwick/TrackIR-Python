//======================================================================================================
// Copyright 2018, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> PrimeX 22 , PrimeX 22W Camera <<<

#include "camerarev49.h"

namespace CameraLibrary
{
    class CameraRev50 : public CameraRev49
    {
    public:
        CameraRev50(CameraLibrary::LensType lenstype = CameraLibrary::LensType::STANDARD_LENS, bool needBlockingController = true);

        int MaximumFrameRateValue() const override;
        int MaximumFullImageFrameRateValue() const override;
        double ImagerWidth() const override;
        double ImagerHeight() const override;
        void GetDistortionModel( Core::DistortionModel& model ) const override;

    };
}
