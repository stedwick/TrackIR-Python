//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <vector>
#include <cstdint>

#include "Core/Frame.h"

#include "activetag.h"
#include "cameralibraryglobals.h"
#include "cameratypes.h"
#include "object.h"
#include "segment.h"

#pragma warning( push )
#pragma warning( disable: 4251 ) // Warning about "needs to have dll-interface" on STL types

namespace Core
{
    class cIReader;
    class cIWriter;
}

namespace CameraLibrary
{
    class Bitmap;
    class Camera;
    class CompressedFrame;
    class HealthMonitor;

    const int kSynchronizerMaxInputCount = 10;

    /// <summary>Data for a single camera on a single frame.</summary>
    class CLAPI Frame
    {
    public:
        explicit Frame( const Camera& sourceCamera );
        Frame(const Frame& other, bool copyPacketData = true);
        ~Frame();

        int ObjectCount() const;            // Number of visible objects
        int FrameID() const;                // Frame Identifier
        Core::eVideoMode FrameType() const; // Frame Type
        int MJPEGQuality() const;           // For MJPEG Frames, MJPEG Quality (1-100)

        /// <summary>True if this is a frame from a camera source (not a sync or other device type).</summary>
        bool IsFromCamera() const;

        /// <summary>True if this frame is from a sync authority source.</summary>
        bool IsFromSyncAuthority() const;

        /// <summary>True if this frame is from a TBar source.</summary>
        bool IsFromTBar() const;

        /// <summary>True if this frame is from a color camera source.</summary>
        bool IsFromColorCamera() const;

        /// <summary>True if this frame is from a Base Station source.</summary>
        bool IsFromBaseStation() const;

        /// <summary>True if this frame is from a camera that has a filter switcher.</summary>
        bool IsFromFilterSwitchedCamera() const;

        /// <summary>True if this frame is from a synchronous video source.</summary>
        bool IsFromSynchronousVideoCamera() const;

        /// <summary>True if this frame is from a camera that supports hardware timing.</summary>
        bool IsFromHardwareTimingCamera() const;

        const cObject* Object( int index ) const; // Object Accessor
        const ObjectLink* Link( int index ) const;

        bool IsInvalid() const;         // Is frame invalid/corrupt
        bool IsEmpty() const;           // Is frame empty
        bool IsGrayscale() const;       // Is frame image data (Grayscale or MJPEG)
        bool IsImage() const;           // Is frame image data

        int Width() const;              // Frame width (in pixels)
        int Height() const;             // Frame height (in pixels)
        int Left() const;               // Frame left edge (in pixels)
        int Top() const;                // Frame top edge (in pixels)
        int Right() const;              // Frame right edge (in pixels)
        int Bottom() const;             // Frame bottom edge (in pixels)

        float Scale() const;            // Effective size of pixel (in pixels)

        double TimeStamp() const;       // Frame timestamp

        // Synchronization Telemetry (The only time these functions return valid information
        //                            is when this object is the result of calling GetFrame()
        //                            on an OptiTrack eSync device)
        //                            
        // If you want synchronized telemetry to compliment one or more Ethernet cameras the
        // best way to achieve this is to attach both the eSync and the cameras to a
        // synchronizer.  The frame groups returned from the synchronizer will then include
        // synchronized camera and synchronization telemetry.

        bool IsSynchInfoValid() const;  // Reports if the calls to the rest of the
                                        // synchronization functions will return
                                        // valid/meaningful information.

        bool IsTimeCodeValid() const;   // Is there valid TimeCode information
        bool IsPrecisionTimestampValid() const;   // Is there valid PrecisionTimestamp information
        bool IsExternalLocked() const;  // Synchronization is locked to a signal
        bool IsRecording() const;       // eSync is reporting that recording should take place
        Core::cTimeCode TimeCode() const;
        Core::cPrecisionTimestamp PrecisionTimestamp() const;

        // Meta-data for the camera this frame came from.
        unsigned int Serial() const;
        int Revision() const;
        int CameraID() const;

        // Active Tag Telemetry ========================

        int ActiveTagTelemetryCount() const;
        const cActiveTag* ActiveTagTelemetry( int index ) const;

        // Hardware Based Timing Information ====

        double HardwareTimeStampValue() const; // Hardware Time Stamp ( in seconds )
        unsigned long long HardwareTimeStamp() const; // Hardware Time Stamp
        bool IsHardwareTimeStamp() const;   // Is there hardware-based time information
        unsigned int HardwareTimeFreq() const; // Frequency of Hardware Time Stamp
        bool MasterTimingDevice() const;    // Is this the master of all reported time

        // Rasterization Functionality ==========

        void Rasterize( Camera& camera, unsigned int width, unsigned int height, unsigned int span,
            unsigned int bitsPerPixel, void* Buffer ) const;
        void RasterizeHandleDuplex(const Camera& camera, Bitmap& back) const;

        void Rasterize( Camera& camera, Bitmap* BitmapRef, bool renderDuplexObjects = true ) const;

        // Uncommonly Needed Methods =================================

        unsigned char* GrayscaleData( Camera& camera );
        const unsigned char* GrayscaleData( Camera& camera ) const;
        int GrayscaleDataSize() const;

        int ImageDataSize() const;

        void SetObjectCount( int count );
        void RemoveObject( int index );

    };


    CLAPI void DeleteFrame( Frame* frame );
    CLAPI void DeleteCompressedFrame( CompressedFrame* cframe );
}

#pragma warning( pop )
