//======================================================================================================
// Copyright 2025, NaturalPoint Inc.
//======================================================================================================
#pragma once

// This is eSync 3 PTP 
// eSync3 uses the same chipset as Active Base Station and Prime X
// don't derive from eSync2 baseclass CameraRev23 or CameraRev34 which is based on a different chipset

#include "camerarev29.h"

namespace CameraLibrary
{
    class CameraRev83 : public CameraRev29
    {
    public:
        CameraRev83(bool needBlockingController = true);

        bool IsCamera() const override { return false; }
        bool IsSyncAuthority() const override { return true; }
        bool IsESync() const override { return true; }
        void SetFrameRate(int value) override;
        cSyncFeatures SyncFeatures() override;
        eRinglightType RinglightType() const override { return eRinglightType::NO_RINGLIGHT; };
    private:
        unsigned int ProcessAdditionalData(Frame* frame, void* additionalData, int remainingData, uint32_t &ptpInputRate);
    };
}
