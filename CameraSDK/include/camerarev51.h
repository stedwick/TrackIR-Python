//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> PrimeX 13 Camera <<<

#include "camerarev31.h"

namespace CameraLibrary
{
    class CameraRev51 : public CameraRev31
    {
    public:
        CameraRev51() = default;

        int MaximumFrameRateValue() const override;
        int MaximumFullImageFrameRateValue() const override;

        bool IsVideoTypeSupported( Core::eVideoMode mode ) const override;

    };
}
