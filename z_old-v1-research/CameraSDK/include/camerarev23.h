//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

// This is the >>> eSync <<<

#include "camerarev11.h"

namespace CameraLibrary
{
    class CameraRev23 : public CameraRev11
    {
    public:
        CameraRev23();

        bool IsCamera() const override { return false; }  // Reports of device is a camera
        bool IsSyncAuthority() const override { return true; }
        bool IsESync() const override { return true; }
        void SetFrameRate( int value ) override;
        cSyncFeatures SyncFeatures() override; // Return devices supported synchronization features
        bool SetParameter( const char* parameterName, const char* parameterValue ) override;
        bool IsIRIlluminationAvailable() const override;
        void QueryHardwareTimeStampValue( int userData ) override;
        eRinglightType RinglightType() const override;
    };
}
