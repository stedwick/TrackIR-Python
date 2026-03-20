//======================================================================================================
// Copyright 2015, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> TrackIR 5.5 <<<

#include "Core/RandomNumberGenerator.h"
#include "camerarev18.h"

namespace CameraLibrary
{
    class CameraRev35 : public CameraRev18
    {
    public:
        CameraRev35();
        virtual ~CameraRev35() = default;

        double ImagerWidth() const override;
        double ImagerHeight() const override;
        double FocalLength() const override;
        void GetDistortionModel( Core::DistortionModel &Model ) const override;

    };
}
