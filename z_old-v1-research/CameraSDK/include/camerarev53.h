//======================================================================================================
// Copyright 2010, NaturalPoint Inc.
//======================================================================================================
#pragma once

// This is the >>> SlimX 13E Camera <<<

#include "camerarev51.h"

namespace CameraLibrary
{
    class CameraRev53 : public CameraRev51
    {
    public:
        CameraRev53() = default;

        int StatusRingLightCount() const override;
        void SetStatusRingLights( int count, const sStatusLightColor* lightColors ) override;
        bool IsIRIlluminationAvailable() const override;
        eRinglightType RinglightType() const override;
    };
}
