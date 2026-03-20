//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

// This is the >>> eSync 2 <<<

#include "camerarev23.h"

namespace CameraLibrary
{
    class CameraRev34 : public CameraRev23
    {
    public:
        CameraRev34();

        cSyncFeatures SyncFeatures() override; // Return devices supported synchronization features
        eRinglightType RinglightType() const override;
    private:
        unsigned int ProcessAdditionalData(Frame* frame, void* additionalData, int remainingData, uint32_t &ptpInputRate);
    };
}
