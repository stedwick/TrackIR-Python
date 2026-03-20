//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> OptiHub <<<

#include "camera.h"
#include "cameramanager.h"

namespace CameraLibrary
{
    class CameraRev13 : public Camera
    {
    public:
        CameraRev13();
        virtual ~CameraRev13() = default;

        bool IsHub() const override { return true; }
        bool IsCamera() const override { return false; }
        bool IsSyncAuthority() const override { return true; }

        void SetFrameRate( int Value ) override;

        cSyncFeatures SyncFeatures() override; //== Return device's supported synchronization features

        bool IsIRIlluminationAvailable() const override;

    };
}
