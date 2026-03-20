//======================================================================================================
// Copyright 2010, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include "cameracommonglobals.h"
#include "cameralibraryglobals.h"
#include "camerarevisions.h"
#include <map>
#include <string>
#include "helpers.h"

namespace CameraLibrary
{
    class Frame;
    class cInputBase;
    class cInputListener;
    class cInputManagerListener;

    // Internal use. Not intended for Camera SDK users but necessary for proper functionality.
    class cInputObject
    {
	public:

		// add a new rev when adding a new camera or device
		enum DeviceTypes
		{
			UnknownCamera = -1, // Invalid/Unknown Device
			BaseCamera = 0,     // Camera
			Rev5 = 2,
			Rev6,               // CameraRev6
			Rev7,               // CameraRev7
			Rev8,               // CameraRev8
			Rev9,               // CameraRev9
			Rev10,              // CameraRev10
			Rev11,              // CameraRev11
			Rev12,              // CameraRev12
			Rev13,              // CameraRev13
			Rev14,              // CameraRev14
			Rev15,              // CameraRev15
			Rev16,              // CameraRev16
			Rev17,              // CameraRev17
			Rev18,              // CameraRev18
			Rev21,              // CameraRev21
			Rev22,              // CameraRev22
			Rev23,              // CameraRev23
			Rev24,              // CameraRev24
			Rev25,              // CameraRev25
			Rev26,              // CameraRev26
			Rev27,              // CameraRev27
			Rev28,              // CameraRev28
			Rev29,              // CameraRev29
			Rev30,              // CameraRev30
			Rev31,              // CameraRev31
			Rev32,              // CameraRev32
			Rev33,              // CameraRev33
			Rev34,              // CameraRev34
			Rev35,              // CameraRev35
			Rev36,              // CameraRev36
			Rev37,              // CameraRev37
			Rev38,              // CameraRev38  // base station 1.0
			Rev49,              // CameraRev49 PrimeX 41
			Rev50,              // CameraRev50 PrimeX 22
			Rev51,              // CameraRev51 PRIMEX 13
			Rev52,              // CameraRev52 PRIMEX 13 + W
			Rev53,              // CameraRev53 SLIMX 13 + E
			Rev54,              // CameraRev54 SlimX 41
			Rev55,              // CameraRev55 PrimeX 120
			Rev56,              // CameraRev56 SlimX 120
			Rev57,              // CameraRev57 Active Base Station
			Rev58,              // CameraRev58 CinePuck
			Rev59,              // CameraRev49 PrimeX 41 + W
			Rev61,              // CameraRev54 SLIMX 41 + W
			Rev63,              // CameraRev50 PrimeX 22 + W
			Rev64,              // CameraRev55 PrimeX 120 + W
			Rev65,              // CameraRev56 SlimX 120 + W
			Rev66,              // CameraRev66 SlimX 22 ZU3 V2
			Rev67,              // CameraRev67 PrimeX 22 ZU3 V2
			Rev68,              // CameraRev68 PrimeX 41 ZU3 V2
			Rev69,              // CameraRev66 SlimX 41 ZU3 V2
			Rev70,              // CameraRev66 SlimX 22 + W ZU3 V2
			Rev71,              // CameraRev67 PrimeX 22 + W ZU3 V2
			Rev72,              // CameraRev68 PrimeX 41 + W ZU3 V2
			Rev73,              // CameraRev66 SlimX 41 + W ZU3 V2
			Rev74,              // CameraRev74 DUO OVERMATCH Prototype USB
            Rev75,              // CameraRev75 Wired Anchor Puck
            Rev80,              // CameraRev81 VersaX22
            Rev81,              // CameraRev81 VersaX41
            Rev82,              // CameraRev81 VersaX120
            Rev83,              // CameraRev83 eSync 3 PTP
		};

        cInputObject();
        ~cInputObject() = default;

		void SetSubModel(int subModel);
		int SubModel();

		DeviceTypes DeviceType();
		void SetDeviceType(cInputObject::DeviceTypes type);

        // returns Device type if is a USB device
		static DeviceTypes GetUSBDeviceType( const std::string devicePath );

		// returns Device type if is a Ethernet device
        static DeviceTypes GetEthernetDeviceType( const int rev );

        // Add a new map when adding a new USB camera rev
        static const std::map<std::string, cInputObject::DeviceTypes> usbDeviceMap;

        // add a new mapping when adding a new Ethernet Camera or device
        static const std::map<int, cInputObject::DeviceTypes> ethernetDeviceMap;

    private:        

		DeviceTypes mDeviceType;
		int mSubModel;
    };

    // Internal use, not intended for Camera SDK users but are necessary
    // for proper functionality.

    class CLAPI cInputManager
    {
    public:
        cInputManager() : mListener( nullptr ) { }
        virtual ~cInputManager() = default;

        enum ConnectionTypes
        {
            NoConnection = 0,
            USB,
            File,
            Memory,
            Ethernet
        };


    private:
        cInputManagerListener* mListener;
    };


    // Internal use. Not intended for Camera SDK users but necessary for proper functionality.
    class cVirtualConfigurationData
    {
    public:
        cVirtualConfigurationData() : CameraID( 0 ), CameraSubModel( 0 ) { }
        virtual ~cVirtualConfigurationData() = default;

        char CameraName[CameraLibrary::kCameraNameMaxLen];
        int CameraWidth;
        int CameraHeight;
        int CameraFrameRate;
        int CameraRevision;
        int CameraSerial;
        int CameraSubModel;
        int CameraID;
    };

    // Internal use. Not intended for Camera SDK users but necessary for proper functionality.
    class CLAPI cInputListener
    {
    public:
        virtual ~cInputListener() = default;

        virtual void IncomingData( unsigned char* buffer, long bufferSize ) = 0;
        virtual void IncomingComm( unsigned char* buffer, long bufferSize ) = 0;
        virtual void IncomingFrame( CameraLibrary::Frame* frame ) = 0;
        virtual void IncomingDebugMsg( const char* text ) = 0;
        virtual void IncomingDisconnect() = 0;
        virtual void SendVirtualConfigurationData( cVirtualConfigurationData* ) = 0;
        virtual void IncomingXACTError() = 0;
        virtual void IncomingTelemetryCameraTemp( float cameraTemp ) = 0;
        virtual void IncomingTelemetryMasterMAC( const char mac[6], bool master ) = 0;
		virtual void IncomingTelemetryAdditional(unsigned char* additionalBuffer, long additionalBufferSize) = 0;
    };

    // Internal use. Not intended for Camera SDK users but necessary for proper functionality.
    class CLAPI cInputManagerListener
    {
    public:
        virtual ~cInputManagerListener() = default;

        virtual void DeviceConnected() = 0;
        virtual void DeviceRemoved( const char* devicePath = nullptr ) = 0;
        virtual bool ShouldConnectCamera( const char* networkInterface, const char* cameraSerial ) = 0;
        virtual bool ContainsConnectedDevice( const char* cameraSerial ) const = 0;
        virtual void DeviceDetected( const char* deviceSerial ) = 0;
    };
}