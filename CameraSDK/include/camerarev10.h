//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> OptiTrack Hardware Key <<<

#include "camera.h"
#include "cameramanager.h"
#include "camerarevisions.h"

namespace CameraLibrary
{

    class CameraRev10 : public HardwareKey
    {
    public:
        CameraRev10();
        virtual ~CameraRev10() = default;

        bool IsHardwareKey() const override { return true; } //== For separation of cameras & keys--
        bool IsCamera() const override { return false; } //== Reports of device is a camera ==--

    };
}

