//======================================================================================================
// Copyright 2018, NaturalPoint Inc.
//======================================================================================================
#pragma once

//======================================================================================================
// Need extreme care and thorough testing when modifying this class
//======================================================================================================

#include <queue>
#include <mutex>

#include "Core/Frame.h"

#include "cameramodulebase.h"
#include "healthmonitor.h"
#include "threading.h"
#include "helpers.h"
#include "framegroup.h"

#ifdef __PLATFORM__LINUX__
#include <semaphore.h>
#include <stdlib.h>
#endif

#pragma warning( push )
#pragma warning( disable: 4251 ) // Warning about "needs to have dll-interface" on STL types

namespace std
{
    class mutex;
}

namespace CameraLibrary
{
    class CLAPI cModuleSyncListener
    {
    public:
        virtual ~cModuleSyncListener() = default;
        virtual void FrameGroupAvailable() = 0;
    };

    // PERFORMANCE RECOMMENDATIONS
    // Even more important for performance: Make PostFrame() re-entrant so all cameras can post simultaneously
    // If this synchronizer ever needs to go over 200 cameras or so, it would be beneficial to sort the
    // mCameras[] as cameras are added and then change the FindCameraIndex() to do a binary search for
    // the camera index.  That should allow this synchronizer to go to extremely high camera counts.
    class CLAPI cModuleSync : public cCameraModule, public HealthMonitor
    {
    public:
        enum RunningModes
        {
            Hardware = 0,
            Software,
            RunningModeCount
        };

        enum eTimeStampCalculation
        {
            SystemClock = 0,  // Default
            FrameIDBased
        };

        enum eOptimization
        {
            /// <summary>
            /// Does not queue the frames. As soon as a different frame ID comes in, existing frames are sent out to the application.
            /// Can cause 2D frame drops for late arriving frames.
            /// </summary>
            ForceTimelyDelivery = 0,

            /// <summary>
            /// Detects if the frames are 5% or more out of order(i.e. frames arriving late) and if so enable queueing of frames. 
            /// Otherwise sends out the frames as soon as a different frame ID comes in.
            /// Can cause 2D frame drops for late arriving frames, if out of order amount is less than 5%.
            /// </summary>
            FavorTimelyDelivery,

            /// <summary>
            /// Queue frames up to kSyncFrameDepth(60) per camera.
            /// Can cause 3D frame drops in next frame if the current frame had to wait long for all frames.
            /// </summary>
            ForceCompleteDelivery,

            eOptimizationCount
        };

        struct sSyncDebug
        {
            int FrameID;
            double TimeStamp;
            double SortValue;
            int CameraID;
        };

        cModuleSync();
        ~cModuleSync();

        void AttachListener( cModuleSyncListener* listener );
        void RemoveListener( cModuleSyncListener* listener );

        void AddCamera( const std::shared_ptr<Camera>& camera );
        void RemoveCamera( unsigned int serialNumber );
        void RemoveAllCameras();

        int CameraCount() const;
        std::shared_ptr<Camera> GetCamera( int Index );

        void SetOptimization( eOptimization OptimizationMode );
        eOptimization Optimization();

        void SetAllowIncompleteGroups( bool enable );
        bool AllowIncompleteGroups();

        void SetSuppressOutOfOrder( bool enable );

        FrameGroup::Modes LastFrameGroupMode();
        double LastFrameGroupSpread();

        void SetTimeStampMode( eTimeStampCalculation Mode );
        eTimeStampCalculation TimeStampMode();

        std::shared_ptr<const FrameGroup> GetFrameGroup();

        float FrameDeliveryRate() const;

        static cModuleSync* Create();
        static void Destroy( cModuleSync* sync );

    };
}

#pragma warning( pop )
