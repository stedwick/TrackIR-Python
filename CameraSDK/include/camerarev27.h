//======================================================================================================
// Copyright 2015, NaturalPoint Inc.
//======================================================================================================
#pragma once


#include "camerarev21.h"

namespace CameraLibrary
{
    class CLAPI CameraRev27 : public CameraRev21
    {
    public:
        CameraRev27() = default;
        virtual ~CameraRev27() = default;

        bool IsIRIlluminationAvailable() const override;

    };
}
