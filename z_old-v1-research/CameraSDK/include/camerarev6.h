//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> OptiTrack C120 <<<

#include "camera.h"
#include "camerarevisions.h"

namespace CameraLibrary
{
    class CameraRev6 : public Camera
    {
    public:
        CameraRev6();
        ~CameraRev6() = default;

        //== Camera Physical Constants ======================================================----

        double ImagerWidth() const override { return C120_IMAGERWIDTH; }
        double ImagerHeight() const override { return C120_IMAGERHEIGHT; }
        double FocalLength() const override { return C120_FOCALLENGTH; }
        int HardwareFrameRate() const override { return 120; }

    };
}
