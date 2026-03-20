//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> Slim 3U <<<

#include "camerarev12.h"
#include "camerarevisions.h"

namespace CameraLibrary
{
    class CameraRev14 : public CameraRev12
    {
    public:
        CameraRev14();
        virtual ~CameraRev14() = default;

        bool IsFrameRateValid(int frameRate) const override;
        int ActualFrameRate() const override;
        int MaximumFrameRateValue() const override;
        int HardwareFrameRate() const override { return 120; }
        void Internal_InitializeCamera() override;

        bool IsFilterSwitchAvailable() const override;
        bool IsIRIlluminationAvailable() const override;
        eRinglightType RinglightType() const override;

        int   GetSubVersion() const;  //== returns the subversion of the camera
		FILE* GetBitStreamFile( int subVersion ) const; // returns the bitstream for the subVersion
    };
}
