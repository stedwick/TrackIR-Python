//======================================================================================================
// Copyright 2010, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== INCLUDES ==========================================================================================

#include "cameracommonglobals.h"

//== GLOBAL DEFINITIONS AND SETTINGS ===================================================================

namespace CameraLibrary
{
    class CLAPI Window
    {
    public:
        Window();
        ~Window() { }

        unsigned short Width() const;
        unsigned short Height() const;
        unsigned short Left() const;
        unsigned short Top() const;
        unsigned short Right() const;
        unsigned short Bottom() const;

    };
}
