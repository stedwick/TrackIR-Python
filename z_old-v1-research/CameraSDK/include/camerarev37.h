//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

// This is the >>> Prime Color Camera <<<

#include "camerarev36.h"

namespace CameraLibrary
{
    class CameraRev37 : public CameraRev36
    {
    public:
        CameraRev37();

        int MaximumFrameRateValue() const override;
        int MaximumFullImageFrameRateValue() const override;
        int CameraResolutionCount() const override;
        int CameraResolutionID() const override;
        sCameraResolution CameraResolution( int index ) const override;
        void SetCameraResolution( int ResolutionID ) override;
        bool IsFilterSwitchAvailable() const override;
        bool IsVideoTypeSupported( Core::eVideoMode mode ) const override;
        bool IsIRIlluminationAvailable() const override;
        void PacketJunction( unsigned char* buffer, long bufferSize, unsigned long long startTimestamp,
            unsigned long long endTimestamp ) override;
        eRinglightType RinglightType() const override;
    };
}
