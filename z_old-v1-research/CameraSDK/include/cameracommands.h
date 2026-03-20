//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== Internal use, not intended for Camera SDK users but are necessary
//== for proper functionality.

#include "Core/Timer.h"

#include "cameralibraryglobals.h"
#include "cameratypes.h"
#include "camera.h"

namespace CameraLibrary
{
    class CLAPI cDeferredCameraCommand : public cCameraCommand
    {
    public:
        virtual ~cDeferredCameraCommand() = default;

        bool IsComplete() const override;
        void SetComplete() override;

    private:
        bool mComplete = false;
    };


    class CLAPI cCommand_StorageWriteChunk : public cCameraCommand
    {
    public:
        cCommand_StorageWriteChunk( int StartAddress, unsigned char* buffer, int bufferSize );
        static cCommand_StorageWriteChunk* Factory( int StartAddress, unsigned char* buffer, int bufferSize );

    protected:
        cCommand_StorageWriteChunk() = default;
        void Execute() override;

    private:
        int mStartAddress;
        unsigned char mChunkData[256];
        int mChunkDataSize;
    };

    class CLAPI cCommand_StorageReadChunk : public cDeferredCameraCommand
    {
    public:
        cCommand_StorageReadChunk( int StartAddress, int bufferSize );

        unsigned char *Buffer();

        void Execute() override;
        bool WaitForCompletion() override { return true; }
        int ExpectedResponsePacketType() const override;
        void CameraResponse( unsigned char *buffer, long bufferSize ) override;

    private:
        int mStartAddress;
        unsigned char mChunkData[256];
        int mChunkDataSize;
    };
}
