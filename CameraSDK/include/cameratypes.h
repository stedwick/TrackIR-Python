//======================================================================================================
// Copyright 2010, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <string>
#include <cstring>
#include <cstdint>

#include "cameralibraryglobals.h"
#include "timebase.h"

#pragma warning( push )
#pragma warning( disable: 4251 ) // Warning about "needs to have dll-interface" on STL types


namespace CameraLibrary
{
    enum eCameraState
    {
        Uninitialized = 0,
        InitializingDevice,
        InitializingCamera,
        Initializing,
        WaitingForChildDevices,
        WaitingForDeviceInitialization,
        Initialized,
        Disconnected,
        Shutdown
    };

    enum eSyncMode
    {
        SyncModeDefault = 0,
        SyncModeCustom,
        SyncModeShutterGoggles
    };

    enum eSyncType
    {
        SyncTypeOptiSync = 0,
        SyncTypeWiredSync
    };

    enum eSyncCameraSync
    {
        SyncAutoUSBSync = 0,
        SyncForceUSBSync
    };

    enum eSyncInputSource
    {
        SyncInputSourceWired = 0,                   // TBar Only
        SyncInputSourceInternal = 0,
        SyncInputSourceSMPTEInput,
        SyncInputSourceSyncExternal = 2,
        SyncInputSourceSyncExternalEitherEdge = 2,
        SyncInputSourceSyncExternalRisingEdge,
        SyncInputSourceSyncExternalFallingEdge,
        SyncInputSourceSyncExternalHighGated,
        SyncInputSourceSyncExternalLowGated,
        SyncInputSourceUSB,                         // Not allowed for TBar
        SyncInputSourceGenlockVideo,
        SyncInputSourceLowImpedance,
        SyncInputSourceHighImpedance,
        SyncInputSourceIsolated,
        SyncInputSourceOptiHub,
        SyncInputSourceSoftware,
        SyncInputSourceInternalAnalogSampleClock,
        SyncInputSourceHighImpedance1,
        SyncInputSourceHighImpedance2,
        SyncInputSourceHighImpedance3,
        SyncInputSourceVesaStereoIn,
        SyncInputSourcePTPInput,
        SyncInputSourceHighImpedance4,            // New for eSync3 PTP
        SyncInputSourceCount
    };

    enum eSyncOutputPhase
    {
        SyncOutputInPhase = 0,
        SyncOutputDelayedByGlobalOffset
    };

    enum eSyncOutputPulseDuration
    {
        SyncOutputPulseDurationExposureTime = 0,
        SyncOutputPulseDurationPassThrough,
        SyncOutputPulseDurationRecordingLevel,
        SyncOutputPulseDurationRecordingPulse,
        SyncOutputExposureTime,
        SyncOutputSelectedSync,
        SyncOutputAdjustedSync,
        SyncOutputSMPTE,
        SyncOutputGenlockVideo,
        SyncOutputLowImpedance,
        SyncOutputHighImpedance,
        SyncOutputIsolated,
        SyncOutputRecordStartStopPulse,
        SyncOutputInternalAnalogSampleClock,
        SyncOutputGatedInternalAnalogSampleClock,
        SyncOutputOptiHub,
        SyncOutputHighImpedance1,
        SyncOutputHighImpedance2,
        SyncOutputHighImpedance3,
        SyncOutputVesaStereoIn,
        SyncOutputDisabled,
        SyncOutputHighImpedance4, // New for eSync3 PTP
        SyncOutputCount
    };

    enum eSyncOutputPolarity
    {
        SyncOutputPolarityNormal = 0,
        SyncOutputPolarityInverted
    };

    enum eUSBSyncInControl
    {
        USBSyncInControlGateClosedDisabled = 0,
        USBSyncInControlGateOpenEnabled
    };

    enum eSyncInfoBits
    {
        SyncInfo_SyncInfoValid = 1,    // Bit 0: Sync/TimeCode Info Present
        SyncInfo_TimeCodeValid = 2,    // Bit 1: TimeCode Valid
        SyncInfo_RecordingBit = 4,    // Bit 2: Recording Bit
        SyncInfo_ExternalLock = 8,    // Bit 3: External  GenLock    
        SyncInfo_ClkStatusOut1 = 16,   // Bit 4: Clock Status Output1
        SyncInfo_ClkStatusOut2 = 32,   // Bit 5: Clock Status Output2
        SyncInfo_ClkStatusOut3 = 64,   // Bit 6: Clock Status Output3
        SyncInfo_ClkStatusOut4 = 128,   // Bit 7: Clock Status Output4
        SyncInfo_PrecisionTimestampValid = 256 // Bit 8: PrecisionTimestamp Valid 
    };

    enum eImagerGain
    {
        Gain_Level0 = 0,
        Gain_Level1 = 1,
        Gain_Level2 = 2,
        Gain_Level3 = 3,
        Gain_Level4 = 4,
        Gain_Level5 = 5,
        Gain_Level6 = 6,
        Gain_Level7 = 7
    };

    enum eSyncLockSource
    {
        eSync_Input_SMPTE = 0,
        eSync_Input_Video = 1,
        eSync_Input_LoZ = 2,
        eSync_Input_HiZ = 3,
        eSync_Input_Isolated = 4,
        eSync_Input_Reserved = 5,
        eSync_Input_IASC = 6   // Internal Analog Sample Clock
    };

    enum eSyncMonitorType
    {
        eSync_Monitor_None = 0,
        eSync_Monitor_SMPTE = 1,
        eSync_Monitor_Video = 2,
        eSync_Monitor_LoZ = 3,
        eSync_Monitor_HiZ = 4,
        eSync_Monitor_Isolated = 5,
        eSync_Monitor_Reserved = 6,
        eSync_Monitor_IASC = 7,
        eSync_Monitor_VESA = 8,
        eSync_Monitor_Input_1 = 9,
        eSync_Monitor_Input_2 = 10,
        eSync_Monitor_Input_3 = 11,
        eSync_Monitor_PTP = 12,
        eSync_Monitor_Input_4 = 13, // New for eSync3 PTP
        eSync_Monitor_Type_Count
    };

    enum eIMUType
    {
        Legacy = 0,
        NoIMU,
        Pro
    };


    const unsigned int kSIGNAL_NONE_MASK = 0x40000000;
    const unsigned int kSIGNAL_OUT_OF_RANGE_MASK = 0x80000000;
    const unsigned int kSIGNAL_PERIOD_MASK = 0x3FFFFFFF;

    class cInputMonitorItem
    {
    public:
        cInputMonitorItem( unsigned int inputMonitorPeriod, eSyncMonitorType inputMonitorType )
            : mInputMonitorPeriod( inputMonitorPeriod )
            , mInputMonitorType( inputMonitorType )
        {
        }

        cInputMonitorItem() : mInputMonitorPeriod( 0 ), mInputMonitorType( eSync_Monitor_None )
        {
        }

        float Frequency() const
        {
            float period = (float) ( mInputMonitorPeriod & kSIGNAL_PERIOD_MASK );
            if( period > 0 )
            {
                return 100000000.0f / ( period );
            }
            return 0;
        }

        eSyncMonitorType Type() const
        {
            return mInputMonitorType;
        }

        bool IsNoSignal() const
        {
            if( mInputMonitorPeriod == 0 )
            {
                return true;
            }

            return ( mInputMonitorPeriod & kSIGNAL_NONE_MASK ) != 0;
        }

        bool IsOutOfRange() const
        {
            if( mInputMonitorPeriod == 0 )
            {
                return true;
            }

            return ( mInputMonitorPeriod & kSIGNAL_OUT_OF_RANGE_MASK ) != 0;
        }

        unsigned int Period() const
        {
            return mInputMonitorPeriod;
        }

    private:
        unsigned int mInputMonitorPeriod;
        eSyncMonitorType mInputMonitorType;
    };

    enum eStatusLEDs
    {
        // Standard Status LEDs
        GreenStatusLED = 0x20,
        RedStatusLED = 0x10,
        CaseStatusLED = 0x40,
        IlluminationLED = 0x80,

        // TrackIR5
        LeftRedStatusLED = 16,
        LeftGreenStatusLED = 32,
        RightRedStatusLED = 1,
        RightGreenStatusLED = 2,

        // SmartNav4
        SNAV4FrontRedStatusLED = 0x10,
        SNAV4FrontGreenStatusLED = 0x20,
        SNAV4BottomStatusLED = 0x40,

        // Prime Series
        AimAssistLED = 0x100,
        HibernationStatusRing = 0x200
    };

    struct sCameraResolution
    {
        sCameraResolution() : ResolutionID( 0 ), Width( 0 ), Height( 0 ) { }
        sCameraResolution( int ID, int width, int height ) : ResolutionID( ID ), Width( width ), Height( height ) { }
        int ResolutionID;
        int Width;
        int Height;
    };

    struct sStatusLightColor
    {
        unsigned char Red;
        unsigned char Green;
        unsigned char Blue;
    };

    struct sObjectModeSettings
    {
        int MinObjectDiameter;
        int MaxObjectDiameter;
        int MinPixelIntensity;
        int MaxPixelIntensity;
        int AspectTolerance;
        int ObjectMargin;
        int AspectBase;
        int AspectStepSize;
        int ObjectSkew;
        int ROIIntrusionDetection;
        float ROIGuardBand;

        sObjectModeSettings()
        {
            MinObjectDiameter = 1;
            MaxObjectDiameter = 56;  // default value for all cameras except (PrimeX, SlimX) 120
            MinPixelIntensity = 48;
            MaxPixelIntensity = 255;
            AspectTolerance = 64;
            ObjectMargin = 4;
            AspectBase = 1;
            AspectStepSize = 1;
            ObjectSkew = 64;
            ROIIntrusionDetection = 1;
            ROIGuardBand = 0.5f;
        }

    };

    struct sTimeStampValueResponse
    {
        double TimeStamp;      // Camera Hardware Time Stamp (in seconds) ===--
        double RoundTripTime;  // Query round trip time (in seconds)
        int    UserData;       // User data passed back via this variable
    };

    struct sHardwareTimeInfo
    {
        sHardwareTimeInfo() : Options( 0 ), UserData( 0 ), CommandResponse( 0 ) { }

        unsigned int Options;
        unsigned int TimeLow;
        unsigned int TimeHigh;
        unsigned int TimerFreq;
        unsigned int RefTimeLow;
        unsigned int RefTimeHigh;
        unsigned int RefTimeFreq;
        unsigned int RefFrameId;
        unsigned int UserData;
        double       RoundTripTime;
        unsigned int CommandResponse;
    };

    struct sColorMatrix
    {
        sColorMatrix()
        {
            // Color matrix coefficients are stored row major
            // Initialize to identity matrix with zero offset

            for( int index = 0; index < 9; index++ )
            {
                ColorMatrixCoefficients[index] = 0.0f;
            }
            for( int index = 0; index < 3; index++ )
            {
                ColorOffsetCoefficients[index] = 0.0f;
            }

            ColorMatrixCoefficients[0] = 1.0f;
            ColorMatrixCoefficients[4] = 1.0f;
            ColorMatrixCoefficients[8] = 1.0f;
        }

        float ColorMatrixCoefficients[9];
        float ColorOffsetCoefficients[3];
    };

    struct sColorPrescalar
    {
        sColorPrescalar() : R( 1.0f ), G1( 1.0f ), G2( 1.0f ), B( 1.0f ) { }

        float R;
        float G1;
        float G2;
        float B;
    };


    CLAPI const char* CameraStateText( eCameraState State );

    CLAPI float Version();
    CLAPI const char* VersionString();
    CLAPI const char* BuildStamp();
    CLAPI int BuildNumber();
};

#pragma warning( pop )
