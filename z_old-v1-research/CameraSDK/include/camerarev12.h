//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

// This is the >>> Flex 3 ( formerly known as V100:R2 ) <<<

#include "camerarev7.h"

namespace CameraLibrary
{
    class cModuleIJGDecompressor;

    class CLAPI CameraRev12 : public CameraRev7
    {
    public:
        CameraRev12();
        ~CameraRev12();

        // camera model specific features

        void SetHighPowerMode( bool Enable ) override;

        bool HighPowerMode() const override;
        bool IsHighPowerModeAvailable() const override;
        bool IsHighPowerModeSupported() const override;
        bool IsFilterSwitchAvailable() const override;
        bool IsMJPEGAvailable() const override;
        bool IsVideoTypeSupported( Core::eVideoMode mode ) const override;

        void GetDistortionModel( Core::DistortionModel &Model ) const override;

        double ImagerWidth() const override;

        int   GetSubVersion() const; //== returns the subversion of the camera
        FILE* GetBitStreamFile( int subVersion ) const; // returns the bitstream for the subVersion
    };
}
