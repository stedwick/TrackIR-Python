//======================================================================================================
// Copyright 2010, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <vector>

#include "Core/UID.h"

#include "Core/ThreadLock.h"
#include "timebase.h"
#include "cameracommonglobals.h"
#include "cameralibraryglobals.h"

#pragma warning( push )
#pragma warning( disable: 4251 ) // Warning about "needs to have dll-interface" on STL types

namespace CameraLibrary
{
    class Bitmap;
    class cHealthMonitorListener;

    class CLAPI cHealthItem
    {
    public:
        cHealthItem();

        double TimeStamp;
        double Duration;
        bool Inverted;
        char Text[kHealthTextMaxLen];
        bool LastIndication;
        mutable int TriggerCount;
        double TriggerTimeStamp;
        int HealthType;
        Core::cUID ID;

        bool Indicated() const;
    };

    class CLAPI HealthMonitor
    {
    public:
        HealthMonitor();
        virtual ~HealthMonitor() = default;

        enum Types
        {
            Health_Alive = 0,
            Health_Receiving_Frames,
            Health_Invalid_Packet,
            Health_Invalid_Header,
            Health_Invalid_Footer,
            Health_Invalid_Grayscale,
            Health_Segment_Overflow,
            Health_Object_Overflow,
            Health_Aggregation_Overflow,
            Health_Frame_Queue_Overflow,
            Health_Large_Command_Queue,
            Health_FrameGroup_Queue_Overflow,
            Health_Missing_Expected_Frame,
            Health_Camera_Stalled,
            Health_Partial_FrameGroup_Delivered,
            Health_Not_In_Hardware_Sync,
            Health_Increased_FrameGroup_Buffering,
            Health_Out_Of_Band_FrameID,
            Health_XACT_Error,
            Health_Duplicate_FrameID,
            Health_Lost_Hardware_TimeStamping,
            Health_Synchonizer_Queue_Overflow,
            Health_Missing_Sync_Telemetry,
            Health_Missing_Active_Telemetry,
            Health_FrameID_Mismatch,
            Health_Out_Of_Order_Group_Delivery,
            Health_Dropped_Network_Packet,
            Health_Active_LED_Count_Changed,
            Health_Type_Count
        };

        //== Primary methods ====================================--

        void AttachListener( cHealthMonitorListener *listener );
        void RemoveListener( cHealthMonitorListener *listener );

        bool QueryIndicator( Types healthType ) const;
        const char* IndicatorText( Types healthType ) const;
        const cHealthItem* HealthItem( Types healthType ) const;

        //==^^===================================================--

        void Report( Types healthType, double duration, const char* text );

        void SetupInverseReporting( Types healthType, const char* text );

        void Rasterize( Bitmap* back );

        void Update();
        void ClearIndicators();

        void SetOwnerID( const Core::cUID &id );

        static double TriggerFloor( Types healthType );

    };

    class CLAPI cHealthMonitorListener
    {
    public:
        virtual ~cHealthMonitorListener() = default;

        virtual void IndicatorNotification( HealthMonitor::Types healthType, cHealthItem* healthItem, bool indicated, const char* message ) = 0;
    };
}

#pragma warning( pop )
