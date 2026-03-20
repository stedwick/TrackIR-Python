//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> Prime 13W Camera <<<

#include "camerarev31.h"

namespace CameraLibrary
{
    class CameraRev32 : public CameraRev31
    {
    public:
        CameraRev32();

        //== Camera Information =========================================================================----

        void GetDistortionModel( Core::DistortionModel& model ) const override;

    };
}
