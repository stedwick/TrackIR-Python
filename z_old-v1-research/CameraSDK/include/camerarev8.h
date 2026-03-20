//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> SmartNav 4 <<<

#include "camerarev7.h"
#include "camerarevisions.h"

namespace CameraLibrary
{
    class cInputBase;

    class CameraRev8 : public CameraRev7
    {
    public:
        CameraRev8() = default;
        virtual ~CameraRev8() = default;

        //== Camera Physical Constants ======================================================----

        double ImagerWidth() const override { return SMARTNAV4_IMAGERWIDTH; }
        double ImagerHeight() const override { return SMARTNAV4_IMAGERHEIGHT; }
        double FocalLength() const override { return SMARTNAV4_FOCALLENGTH; }
        int HardwareFrameRate() const override { return 100; }

        bool IsAGCAvailable() const override { return false; }
        bool IsAECAvailable() const override { return false; }

        void AttachInput( cInputBase *Input ) override;
        int CameraID() const override;
        bool CameraIDValid() const override { return false; }

    };
}
