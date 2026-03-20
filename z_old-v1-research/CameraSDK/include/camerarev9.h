//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> TrackIR 5 <<<

#include "camera.h"
#include "camerarevisions.h"
#include "coremath.h"

namespace CameraLibrary
{
    class CameraRev9 : public Camera
    {
    public:
        CameraRev9();
        ~CameraRev9() = default;

        //== Camera Physical Constants ======================================================----

        double ImagerWidth() const override { return TRACKIR5_IMAGERWIDTH; }
        double ImagerHeight() const override { return TRACKIR5_IMAGERHEIGHT; }
        double FocalLength() const override { return TRACKIR5_FOCALLENGTH; }
        int HardwareFrameRate() const override { return 120; }

        void GetDistortionModel( Core::DistortionModel& Model ) const override;

        bool IsVideoTypeSupported( Core::eVideoMode mode ) const override;

        int  MinimumExposureValue() const override; //== Returns the minimum camera exposure ========---
        int  MaximumExposureValue() const override; //== Returns the maximum camera exposure ========---

    };

}
