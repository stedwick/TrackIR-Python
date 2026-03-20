//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

// This is the >>> Prime 41 Camera <<<

#include "camerarev26.h"

namespace CameraLibrary
{
    class CameraRev29 : public CameraRev26
    {
    public:
        CameraRev29( bool needBlockingController = true );

        int MaximumMJPEGRateValue() const override;    // Returns the maximum MJPEG rate
        bool IsMJPEGAvailable() const override { return true; }

    };
}
