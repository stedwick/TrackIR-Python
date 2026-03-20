//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> Prime 13 Camera <<<

#include "camerarev30.h"

namespace CameraLibrary
{
    class CameraRev31 : public CameraRev30
    {
    public:
        CameraRev31();

        int MaximumFrameRateValue() const override;
        double ImagerWidth() const override;
        double ImagerHeight() const override;
        void GetDistortionModel( Core::DistortionModel& model ) const override;

        bool IsFilterSwitchAvailable() const override;
        bool IsContinuousIRAvailable() const override;
        bool IsVideoTypeSupported( Core::eVideoMode mode ) const override;
        int StatusRingLightCount() const override;

    };
}
