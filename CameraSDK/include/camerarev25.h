//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once


#include "camerarev12.h"

namespace CameraLibrary
{
    class CameraRev25 : public CameraRev12
    {
    public:
        CameraRev25();
        virtual ~CameraRev25() = default;

        int HardwareFrameRate() const override { return 100; }

        bool IsFilterSwitchAvailable() const override;
        bool IsHighPowerModeAvailable() const override;
        bool IsHighPowerModeSupported() const override;
        bool IsIRIlluminationAvailable() const override;

    };
}
