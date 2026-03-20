//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once


#include "camerarev12.h"
#include "camerarevisions.h"

namespace CameraLibrary
{
    class CameraRev15 : public CameraRev12
    {
    public:
        CameraRev15();
        ~CameraRev15() = default;

        int HardwareFrameRate()  const override { return 120; }

    };
}
