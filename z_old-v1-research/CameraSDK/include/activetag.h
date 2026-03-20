//======================================================================================================
// Copyright 2018, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include "Core/Frame.h"
#include "Core/Quaternion.h"
#include "Core/Vector3.h"
#include "cameratypes.h"

#pragma warning( push )
#pragma warning( disable: 4251 ) // Warning about "needs to have dll-interface" on STL types

namespace CameraLibrary
{
    class cIWriter;
    class cIReader;
    static const unsigned int ACTIVE_1_MAX_TAG_ID = 0xE0000063;

    // Individual tag telemetry now available in CameraManager for ActiveIO (active 2) devices
    struct sActiveTagSettings
    {
        unsigned int serial = 0;
        unsigned int basestationSerial = 0;
        unsigned int rfChannel = -1;
        unsigned int tagID = ACTIVE_1_MAX_TAG_ID;
        int ledCount = 8;
        eIMUType imuType = eIMUType::NoIMU;
        bool isEthernet = false;

    };

    class CORE_API cActiveTag
    {
    public:
        // Default constructor only used to support loading from file.
        cActiveTag() = default;
        cActiveTag( const Core::cRotationf& orientation, unsigned int tagID, Core::FrameIndex frameIDTimeStamp,
            int timeStampOffsetNs );

        cActiveTag(const Core::cRotationf& orientation, Core::FrameIndex frameID, sActiveTagSettings tagInfo);
        cActiveTag( const Core::cRotationf& orientation, const Core::cVector3f acceleration, Core::FrameIndex frameID, sActiveTagSettings tagInfo );

        cActiveTag(sActiveTagSettings tagInfo);

        ~cActiveTag() = default;

        const Core::cRotationf Rotation() const;
        const Core::cQuaternionf Orientation() const;
        const Core::cVector3f Acceleration() const;

        // Tag ID is a composition of RFChannel and UplinkID for legacy Active Devices
        unsigned int TagID() const;
        unsigned int BatteryLevel() const;
        void SetBatteryLevel(unsigned int batteryMv);

        unsigned int RFChannel() const;         // returns -1 for Active IO (active 2) tags
        unsigned int UplinkID() const;          // returns 0 for Active IO (active 2) tags

        // active 2 functionality
        unsigned int TagSerial() const;         // returns -1 for legacy tags
        unsigned int MaskedTagSerial() const;   // returns -1 for legacy tags

        bool IsEthernet() const;
        bool IsActive2() const;
        int LedCount() const;
        bool IMUType() const;

        bool IsStale() const;
        void SetStale(bool isStale);

        Core::FrameIndex FrameIDTimeStamp() const;

        /// <summary>Offset (in ns) from the beginning of the frame.</summary>
        int TimeStampOffset() const;

        /// <summary>Number of available GPIO pin states.</summary>
        int GPIOPinCount() const;

        /// <summary>GPIO pin state.</summary>
        bool GPIOPin( int which ) const;


    };
}

#pragma warning( pop )
