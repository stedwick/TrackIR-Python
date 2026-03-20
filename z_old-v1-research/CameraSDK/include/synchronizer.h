//======================================================================================================
// Copyright 2010, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <mutex>

#include "cameralibraryglobals.h"

#pragma warning( push )
#pragma warning( disable: 4251 ) // Warning about "needs to have dll-interface" on STL types

namespace std
{
    class mutex;
}

namespace CameraLibrary
{
    class Frame;
    class Camera;

    /// <summary>A class used to track a buffer of frame objects for an individual camera.</summary>
    class CLAPI Synchronizer
    {
    public:
        Synchronizer( const Camera& owner );
        ~Synchronizer();

        /// <summary>Retrieves an available frame from the pool. If no frames are available, a null pointer is returned.</summary>
        std::shared_ptr<Frame> GetAvailableFrame();

        /// <summary>Add a frame to the queue of completed frames, which can then be retrieved with GetCompletedFrame().</summary>
        void PostFrame( const std::shared_ptr<const Frame>& frame );

        /// <summary>Retrieve the next frame in the completed frames queue. If the queue is empty, a null frame is returned.</summary>
        std::shared_ptr<const Frame> GetCompletedFrame();

        /// <summary>Flush any pending frames.</summary>
        void Flush();

        /// <summary>Number of frames currently allocated for use in the frame pool. The frame pool will grow
        /// or shrink under demand.</summary>
        size_t AllocatedFrameCount() const;

        /// <summary>True if all allocated frames are also available.</summary>
        bool AllFramesAvailable() const;

        /// <summary>True if the pool has frames available for use.</summary>
        bool HasAvailableFrames() const;

    };
}

#pragma warning( pop )
