//======================================================================================================
// Copyright 2010, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <memory>

#include "cameracommonglobals.h"

namespace CameraLibrary
{
    class Camera;
    class Frame;
    class Bitmap;

    class CLAPI cCameraModule
    {
    public:
        cCameraModule() = default;
        virtual ~cCameraModule() = default;

        virtual void FrameRasterize( const Camera* camera, const Frame* frame, Bitmap* frameBuffer );
        virtual void PrePostFrame( Camera* camera, const std::shared_ptr<Frame>& frame );
        virtual bool PostFrame( Camera* camera, const std::shared_ptr<const Frame>& frame );
        virtual bool PostMJPEGData( Camera* camera, const unsigned char* buffer, long bufferSize, const Frame* frame, 
            int frameWidth, int frameHeight );
        virtual bool PostVideoData(Camera* camera, const unsigned char* buffer, long bufferSize, const Frame* frame,
            int frameWidth, int frameHeight,
            unsigned char* alignedFrameBuffer, long alignedFrameBufferSize);
        virtual bool PostVideoData(Camera* camera, const unsigned char* buffer, long bufferSize, const Frame* frame,
            int frameWidth, int frameHeight,
            unsigned char* alignedFrameBuffer, long alignedFrameBufferSize, int pixelFormat);
        virtual bool PostProcessImage(Camera* camera, const Frame* frame);

        virtual void IncomingDebugMsg( Camera* camera, const char* text );

        virtual void SettingsChanged( Camera* camera );     // Called whenever any command is sent to the camera

        virtual void AttachedTo( Camera* camera );          // Called when a module is attached to a camera
        virtual void RemovedFrom( Camera* camera );         // Called when a module is removed from a camera

        virtual void FrameQueueOverflow( Camera* camera );  // notification of frame queue overflow

        // Internal use only, these functions are not called
        virtual void IncomingData( Camera* camera, unsigned char* buffer, long bufferSize );
        virtual void IncomingComm( Camera* camera, unsigned char* buffer, long bufferSize );
        virtual void OutgoingComm( Camera* camera, const unsigned char* buffer, long bufferSize );
    };

    class CLAPI cModuleMJPEGStub : public cCameraModule
    {
    public:
        cModuleMJPEGStub() = default;
        ~cModuleMJPEGStub() = default;

        bool PostMJPEGData( Camera* camera, const unsigned char* buffer, long bufferSize, const Frame* frame,
            int frameWidth, int frameHeight ) override;
    };
}
