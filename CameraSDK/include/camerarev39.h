//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> OptiTrack I/O X Duo and Trio Device Break-out Connector <<<

#include "camera.h"
#include "camerarev16.h"

namespace CameraLibrary
{
    class CameraRev39 : public Camera
    {
    public:
        CameraRev39();
        virtual ~CameraRev39();

        bool IsSyncAuthority() const override { return true; }
        bool IsCamera() const override { return false; }
        bool IsTBar() const override { return true; }
        bool IsUSB() const override { return true; }
        bool IsVirtual() const override;
        eRinglightType RinglightType() const override;
    };
}
