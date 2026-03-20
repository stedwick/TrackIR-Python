//======================================================================================================
// Copyright 2010, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include "cameracommonglobals.h"

namespace CameraLibrary
{
    class cObject;

    class CLAPI cSegment
    {
    public:
        cSegment() = default;
        ~cSegment() = default;

        // Public cSegment Interface
        int StartX() const;
        int StartY() const;
        int Length() const;
        int StopX() const;

        // cSegment & Object References
        cSegment* Next() const;

    };
}
