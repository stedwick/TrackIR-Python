//======================================================================================================
// Copyright 2009, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <vector>
#include "cameracommonglobals.h"

namespace CameraLibrary
{
    class Camera;

    class CLAPI cDeviceDataStorageBase
    {
    public:
        cDeviceDataStorageBase( Camera* Device = nullptr );
        ~cDeviceDataStorageBase();


        virtual int MaximumSize();

    protected:
        unsigned char* mData; // Data block
        Camera* mDevice; // This is a pointer to the device

        int mDataMaxSize; // This is the maximum space available for storage
        int mDataSize; // This is the camera's frame buffer pixel height
    };
}
