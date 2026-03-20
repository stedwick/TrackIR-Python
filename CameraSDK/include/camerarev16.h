//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

// This is the >>> TBar Trio 3 <<<

#include "camerarev12.h"
#include "camerarevisions.h"
#include "devicedatastoragebase.h"
#include "Core/ThreadLock.h"

namespace CameraLibrary
{
    class cInputBase;
    class CameraRev16Child;
    class CameraRev39;

    class CameraRev16 : public CameraRev12
    {

    private:
        CameraRev16() {}; // hide default constructor

    public:
        CameraRev16(bool storageImplementation, bool initSerialPrefix = true);
        ~CameraRev16();

        void AttachInput(cInputBase* Input) override;

        bool IsFilterSwitchAvailable() const override;

        bool IsHighPowerModeSupported() const override { return false; }  // This camera is always in high-power mode

        bool IsAGCAvailable() const override { return true; }
        bool IsAECAvailable() const override { return false; }
        void SetAGC(bool Enable);
        void SetAEC(bool Enable);

        bool IsTBar() const override { return true; }
        bool IsSyncAuthority() const override { return false; }

        cSyncFeatures SyncFeatures() override; // Return devices supported synchronization features

        int HardwareFrameRate() const override { return 120; }

        bool IsVideoTypeSupported(Core::eVideoMode mode) const override;

        // Device Non-Volatile Data Storage

        int StorageMaxSize() override;

        int LoadFile( const char* Filename, unsigned char* buffer, int bufferSize ) const override;
        bool SaveFile( const char* Filename, unsigned char* buffer, int bufferSize ) const override;

    };

    class CameraRev16Child : public CameraRev12
    {
    public:
        CameraRev16Child();
        ~CameraRev16Child();

        int MJPEGQuality() const override;
        bool IsHighPowerModeSupported() const override { return false; }  // This camera is always in
        // high-power mode

        bool IsFilterSwitchAvailable() const override;

        bool IsTBar() const override { return true; }

        bool IsVideoTypeSupported(Core::eVideoMode mode) const override;

        bool IsAGCAvailable() const override { return true; }
        bool IsAECAvailable() const override { return false; }
        void SetAGC(bool Enable);
        void SetAEC(bool Enable);

        int Intensity() override;

        int HardwareFrameRate()  const override { return 120; }

        void SetContinuousIR(bool Enable) override;
        bool ContinuousIR();

        bool IsVirtual() const override;

        eCameraState State() const override;

    };
}

// Device Storage ==> 0x3000 -> 0x3FFF :  4096  calibration storage
//
