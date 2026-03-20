//======================================================================================================
// Copyright 2010, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <vector>
#include <memory>

#include "cameralibraryglobals.h"
#include "cameracommonglobals.h"

#pragma warning( push )
#pragma warning( disable: 4251 ) // Warning about "needs to have dll-interface" on STL types

namespace Core
{
    class cTimeCode;
    class cPrecisionTimestamp;
}

namespace CameraLibrary
{
    class Frame;

    // FrameGroup object contains a synchronized group of frames from multiple cameras when the 
    // cModuleSync is utilized to synchronize frame data.
    class CLAPI FrameGroup
    {
    public:
        FrameGroup();
        ~FrameGroup() = default;

        enum Modes
        {
            None = 0,
            Software,
            Hardware
        };

        std::vector<std::shared_ptr<const Frame>> GetAllFrames() const;
        std::shared_ptr<const Frame> GetFrame( int index ) const;
        int Count() const;

        void AddFrame( std::shared_ptr<const Frame> frame );

        void SetMode( Modes mode );
        Modes Mode() const;

        void SetTimeStamp( double timeStamp );
        void SetTimeSpread( double timeSpread );
        void SetEarliestTimeStamp( double timeStamp );
        void SetLatestTimeStamp( double timeStamp );

        double TimeSpread() const;
        double TimeStamp() const;
        double EarliestTimeStamp() const;
        double LatestTimeStamp() const;

        int FrameID() const;

        Core::cTimeCode TimeCode() const;
        Core::cPrecisionTimestamp PrecisionTimestamp() const;

        double TimeSpreadDeviation( int index ) const;

        // <summary>Vector of camera serial numbers who had a dropped frame in this frame group.</summary>
        const std::vector<int>& DroppedFrames() const;

    };
}

#pragma warning( pop )
