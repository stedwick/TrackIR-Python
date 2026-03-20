//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> Slim 13E Camera <<<

#include "camerarev31.h"

namespace CameraLibrary
{
    class CameraRev33 : public CameraRev31
    {
    public:
        CameraRev33() = default;

        int StatusRingLightCount() const override;
        void SetStatusRingLights( int Count, const sStatusLightColor* LightColors ) override;
        bool IsIRIlluminationAvailable() const override;
        eRinglightType RinglightType() const override;
    };
}
