//======================================================================================================
// Copyright 2018, NaturalPoint Inc.
//======================================================================================================
#pragma once

// This is the >>> Active Base Station <<<

#include <vector>

#include "activetag.h"
#include "camerarev11.h"

namespace CameraLibrary
{

    class CameraRev38 : public CameraRev11
    {
    public:
        CameraRev38();

        bool IsCamera() const override { return false; }
        bool IsBaseStation() const override { return true; }
        bool IsIRIlluminationAvailable() const override { return false; }
        void QueryHardwareTimeStampValue( int userData ) override;

        bool IsCameraIDAssigned() override;
        bool IsVideoTypeSupported( Core::eVideoMode mode ) const override;
        bool ExpectFrameID( int frameID ) const override;

        void QueryHardwareTimeInfo( const sHardwareTimeInfo& timeInfo ) const override;
        bool IsHardwareTimeInfoSupported() const override;

        void PacketJunction( unsigned char* buffer, long bufferSize, unsigned long long startTimestamp,
            unsigned long long endTimestamp ) override;
        eRinglightType RinglightType() const override;
    };
}
