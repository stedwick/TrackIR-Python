//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once

// Local includes
#include "Core/BuildConfig.h"

namespace Core
{
    // Forward declarations
    class cIWriter;
    class cIReader;
    class cTimeCode;
    class cPrecisionTimestamp;

    // An exact frame value
    using FrameIndex = int;

    enum eVideoMode 
    {
        SegmentMode              = 0,
        GrayscaleMode            = 1,
        ObjectMode               = 2,
        InterleavedGrayscaleMode = 3,
        PrecisionMode            = 4,
        BitPackedPrecisionMode   = 5,
        MJPEGMode                = 6,
        DuplexMode               = 11,
        VideoMode                = 9,
        SoftwareObjectMode       = 10,
        SynchronizationTelemetry = 99,
        UnknownMode              = 100              
    };

    /// <summary>
    /// Commonly-used frame constant values.
    /// </summary>
    class cFrameConstants
    {
    public:
        //==============================================================================================
        // Helper constants
        //==============================================================================================

        static const long kLargestNegativeInt  = ( (int) 1 ) << ( sizeof( int ) * 8 - 1 );
        static const long kLargestPositiveInt  = ~( ( (int) 1 ) << ( sizeof( int ) * 8 - 1 ) );

        static const FrameIndex kPositiveInfinity = kLargestPositiveInt;
        static const FrameIndex kNegativeInfinity = kLargestNegativeInt;
        static const FrameIndex kInvalid = kLargestNegativeInt;
    };

    /// <summary>
    /// An interface class representing a single frame of data from one camera or device.
    /// </summary>
    class CORE_API cICameraFrame
    {
    public:
        enum eDataType
        {
            None = 0,
            Camera = 1,
            Video = 1 << 1,
            Audio = 1 << 2
        };

        static const int kCompressedFrameVersion = 2;

        virtual ~cICameraFrame() = default;

        virtual void Save( Core::cIWriter *stream ) const = 0;
        virtual bool Load( Core::cIReader *stream, int version = kCompressedFrameVersion ) = 0;

        enum eCompressedFrameTypes
        {
            Packet,
            ObjectOnly,
            Original,
            TinyObjectOnly
        };

        virtual eCompressedFrameTypes CompressionType() const = 0;

        virtual bool IsInvalid() const = 0;
        virtual double TimeStamp() const = 0;
        virtual int ObjectCount() const = 0;
        virtual int SegmentCount() const = 0;
        virtual int FrameID() const = 0;
        virtual eVideoMode FrameType() const = 0;
        virtual unsigned int Serial() const = 0;
        virtual int Revision() const = 0;
        virtual int CameraID() const = 0;
        virtual long MemorySize() const = 0;

        virtual int Width() const = 0;
        virtual int Height() const = 0;
        virtual int Left() const = 0;
        virtual int Top() const = 0;
        virtual int Right() const = 0;
        virtual int Bottom() const = 0;

        virtual void RemoveData(eDataType dataTypesToRemove) = 0;
        virtual bool IsEmpty() const = 0;
        virtual bool IsSyncFrame() const = 0;

        virtual long long HardwareTimeStamp() const = 0;
        virtual unsigned int HardwareTimeFreq() const = 0;

        virtual bool IsTimeCodeValid() const = 0;
        virtual cTimeCode TimeCode() const = 0;

        virtual bool IsPrecisionTimestampValid() const = 0;
        virtual cPrecisionTimestamp PrecisionTimestamp() const = 0;

        virtual int ActiveTagTelemetryCount() const = 0;
        virtual unsigned char* ActiveTagTelemetry( int index ) const = 0;

        //======== NP INT ==============================================================================

        virtual unsigned char* ObjectData() const = 0;
        virtual unsigned char* SegmentData() const = 0;
        virtual unsigned char* PacketData() const = 0;
        virtual int PacketDataSize() const = 0;
    };

    /// <summary>
    /// The base class for all camera frame factories.
    /// </summary>
    class CORE_API cICameraFrameFactory
    {
    public:
        virtual ~cICameraFrameFactory() = default;

        /// <summary>Create a new instance of a camera frame.</summary>
        virtual cICameraFrame* CreateInstance() const = 0;
    };
}
