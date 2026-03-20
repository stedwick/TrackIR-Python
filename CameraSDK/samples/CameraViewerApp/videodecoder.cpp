extern "C"
{
#include "libavutil/pixfmt.h"
#include "libavutil/imgutils.h"
#include "libavutil/error.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
}

#include "Core/DebugSystem.h"
#include "Core/Serializer.h"
#include "Core/Timer.h"

#include "frame.h"
#include "camera.h"

#include "videodecoder.h"

using namespace CameraLibrary;

namespace {
    // Return bytes-per-pixel for the subset of target pixel formats we support writing into
    inline int BytesPerPixelFor(AVPixelFormat fmt)
    {
        switch (fmt)
        {
        case AV_PIX_FMT_GRAY8: return 1;
        case AV_PIX_FMT_RGB24:
        case AV_PIX_FMT_BGR24: return 3;
        case AV_PIX_FMT_ARGB:
        case AV_PIX_FMT_ABGR:
        case AV_PIX_FMT_RGBA:
        case AV_PIX_FMT_BGRA:  return 4;
        default:               return 4;
        }
    }

    // Pretty-print an FFmpeg error code
    inline const char* AvErrStr(int err)
    {
        thread_local char buf[256];
        av_strerror(err, buf, sizeof(buf));
        return buf;
    }
}

//== H264 Video Decoder ======================================================================-----
cModuleVideoDecompressorLibav::cModuleVideoDecompressorLibav()
    : color_conversion_context(nullptr)
    , libav_initialized(false)
    , libav_decode_frame(nullptr)
    , libav_frame(nullptr)
    , av_decoder(nullptr)
    , decoder_context(nullptr)
{
}

cModuleVideoDecompressorLibav::~cModuleVideoDecompressorLibav()
{
    Shutdown();
}

bool cModuleVideoDecompressorLibav::PostVideoData(Camera*,
    const unsigned char* Buffer, long bufferSize,
    const Frame*,
    int frameWidth, int frameHeight,
    unsigned char* alignedFrameBuffer, long alignedFrameBufferSize)
{
    return PostVideoData(nullptr, Buffer, bufferSize,
        nullptr, frameWidth, frameHeight,
        alignedFrameBuffer, alignedFrameBufferSize,
        AV_PIX_FMT_RGBA);
}

bool cModuleVideoDecompressorLibav::PostVideoData(Camera*,
    const unsigned char* Buffer, long bufferSize,
    const Frame*,
    int frameWidth, int frameHeight,
    unsigned char* alignedFrameBuffer, long alignedFrameBufferSize,
    int pixelFormat)
{
    if (!libav_initialized)
    {
        libav_initialized = Initialize();
    }

    if (!libav_initialized)
    {
        // If initialization failed, clear the output buffer to black and exit
        if (alignedFrameBuffer && alignedFrameBufferSize > 0)
            memset(alignedFrameBuffer, 0, static_cast<size_t>(alignedFrameBufferSize));
        return false;
    }

    if (!Buffer || bufferSize <= 0 || !alignedFrameBuffer || frameWidth <= 0 || frameHeight <= 0)
    {
        return false;
    }

    AVPacket packet = {};
    packet.data = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(Buffer));
    packet.size = static_cast<int>(bufferSize);

    int ret = avcodec_send_packet(decoder_context, &packet);
    if (ret < 0)
    {
        printf("Error sending packet to decoder: %s\n", AvErrStr(ret));
        return false;
    }

    ret = avcodec_receive_frame(decoder_context, libav_decode_frame);
    if (ret < 0)
    {
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            return false; // No frame available yet
        }
        printf("Error receiving frame from decoder: %s\n", AvErrStr(ret));
        return false;
    }

    if (!libav_decode_frame->data[0])
    {
        printf("Decoded frame has no data.\n");
        return false;
    }

    AVPixelFormat dstFmt = static_cast<AVPixelFormat>(pixelFormat);

    // Validate destination buffer size
    const int dstBpp = BytesPerPixelFor(dstFmt);
    const int64_t required = static_cast<int64_t>(frameWidth) *
                             static_cast<int64_t>(frameHeight) *
                             static_cast<int64_t>(dstBpp);

    if (alignedFrameBufferSize < required)
    {
        printf("Output buffer too small: need %lld, have %ld\n",
               static_cast<long long>(required), alignedFrameBufferSize);
        return false;
    }

    const int srcW = decoder_context->width;
    const int srcH = decoder_context->height;
    const AVPixelFormat srcFmt = decoder_context->pix_fmt;

    color_conversion_context = sws_getCachedContext(
        color_conversion_context,
        srcW, srcH, srcFmt,
        frameWidth, frameHeight, dstFmt,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!color_conversion_context)
    {
        printf("Error initializing color conversion context.\n");
        return false;
    }

    if (!libav_frame)
    {
        libav_frame = av_frame_alloc();
        if (!libav_frame)
        {
            printf("Error: unable to allocate destination AVFrame.\n");
            return false;
        }
    }

    ret = av_image_fill_arrays(
        libav_frame->data,
        libav_frame->linesize,
        alignedFrameBuffer,
        dstFmt,
        frameWidth,
        frameHeight,
        1);

    if (ret < 0)
    {
        printf("Error filling image arrays: %s\n", AvErrStr(ret));
        return false;
    }

    // Perform color conversion / scaling
    const int scaled = sws_scale(
        color_conversion_context,
        libav_decode_frame->data,
        libav_decode_frame->linesize,
        0,
        srcH,
        libav_frame->data,
        libav_frame->linesize);

    if (scaled <= 0 || !libav_frame->data[0])
    {
        printf("Error: sws_scale produced no frame data (ret=%d).\n", scaled);
        return false;
    }

    return true;
}

bool cModuleVideoDecompressorLibav::Initialize()
{
    std::lock_guard<std::recursive_mutex> lock(decoder_open_lock);

    if (libav_initialized)
        return true;

    av_decoder = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!av_decoder)
    {
        printf("Unable to find H.264 decoder.\n");
        return false;
    }

    decoder_context = avcodec_alloc_context3(av_decoder);
    if (!decoder_context)
    {
        printf("Unable to allocate decoder context.\n");
        return false;
    }

    AVDictionary* opts = nullptr;
    int res = avcodec_open2(decoder_context, av_decoder, &opts);
    if (opts)
        av_dict_free(&opts);

    if (res < 0)
    {
        printf("Unable to open decoder: %s (%d)\n", AvErrStr(res), res);
        avcodec_free_context(&decoder_context);
        return false;
    }

    libav_decode_frame = av_frame_alloc();
    if (!libav_decode_frame)
    {
        printf("Could not allocate decode frame.\n");
        avcodec_free_context(&decoder_context);
        return false;
    }

    libav_initialized = true;
    return true;
}

void cModuleVideoDecompressorLibav::Shutdown()
{
    if (!libav_initialized)
        return;

    libav_initialized = false;

    if (libav_decode_frame)
    {
        av_frame_free(&libav_decode_frame);
        libav_decode_frame = nullptr;
    }

    if (libav_frame)
    {
        av_frame_free(&libav_frame);
        libav_frame = nullptr;
    }

    if (color_conversion_context)
    {
        sws_freeContext(color_conversion_context);
        color_conversion_context = nullptr;
    }

    if (decoder_context)
    {
        avcodec_free_context(&decoder_context);
        decoder_context = nullptr;
    }

    av_decoder = nullptr;
}

unsigned char* cModuleVideoDecompressorLibav::AlignedMemoryAllocation(long size)
{
    return (unsigned char*)av_malloc(size);
}

std::recursive_mutex cModuleVideoDecompressorLibav::decoder_open_lock;
