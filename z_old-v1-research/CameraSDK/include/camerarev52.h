//======================================================================================================
// Copyright 2010, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> PrimeX 13W Camera <<<

#include "camerarev51.h"

namespace CameraLibrary
{
    class CameraRev52 : public CameraRev51
    {
    public:
        CameraRev52() = default;

        void GetDistortionModel( Core::DistortionModel& model ) const override;

    };
}
