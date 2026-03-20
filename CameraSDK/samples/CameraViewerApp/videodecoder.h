#pragma once

#include <stdio.h>
#include <mutex>

#include "Core/Platform.h"

#include "cameramodulebase.h"

extern "C" {
#include <libavformat/avio.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
}

//== Video Decoder Camera Module Utilizing Libav module of ffmpeg for Decoding color video data

namespace CameraLibrary
{
    const int kH264DefaultBufferSize = 32768;

    class cModuleVideoDecompressorLibav : public CameraLibrary::cCameraModule
    {
    public:
        cModuleVideoDecompressorLibav();
        ~cModuleVideoDecompressorLibav();

        void OutgoingComm(Camera* Camera, const unsigned char* Buffer, long BufferSize) override {}
        virtual bool PostVideoData(Camera* Camera, const unsigned char* Buffer, long BufferSize, const Frame* Frame,
            int FrameWidth, int FrameHeight, unsigned char* AlignedFrameBuffer, long AlignedFrameBufferSize) override;
        virtual bool PostVideoData(Camera* Camera, const unsigned char* Buffer, long BufferSize, const Frame* Frame,
            int FrameWidth, int FrameHeight, unsigned char* AlignedFrameBuffer, long AlignedFrameBufferSize, int pixelFormat) override;

        int Read(unsigned char* Buffer, int BufferSize);
        unsigned char* AlignedMemoryAllocation(long size);

    private:
        bool Initialize();
        void Shutdown();

        bool libav_initialized;

        SwsContext* color_conversion_context;
        AVFrame* libav_decode_frame;
        AVCodecContext* decoder_context;
        AVFrame* libav_frame;
        const AVCodec* av_decoder;

        static std::recursive_mutex decoder_open_lock;
    };
}