//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> TrackIR 4 <<<

#include "camera.h"
#include "camerarevisions.h"

namespace CameraLibrary
{
    class Frame;
    class cInputBase;

    class CameraRev5 : public Camera
    {
    public:
        CameraRev5();
        virtual ~CameraRev5() = default;

        //== Camera Physical Constants ======================================================----

        double ImagerWidth() const override { return TRACKIR4_IMAGERWIDTH; }
        double ImagerHeight() const override { return TRACKIR4_IMAGERHEIGHT; }
        double FocalLength() const override { return TRACKIR4_FOCALLENGTH; }
        int HardwareFrameRate() const override { return 120; }

        void GetDistortionModel( Core::DistortionModel &Model ) const override;

    };
}
