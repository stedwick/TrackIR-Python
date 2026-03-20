//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
// NOLINTBEGIN(misc-non-private-member-variables-in-classes, misc-unused-parameters)
#pragma once

#include <vector>
#include <string>
#include <memory>

#include "Core/UID.h"
#include "Core/Serializer.h"
#include "Core/ThreadLock.h"

#include "queue.h"
#include "frame.h"
#include "synchronizer.h"
#include "camerawindow.h"
#include "healthmonitor.h"
#include "helpers.h"
#include "threading.h"
#include "cameralibraryglobals.h"
#include "corefunctions.h"
#include "inputmanagerbase.h"
#include "ActiveIOTypes.h"  

#pragma warning( push )
#pragma warning( disable: 4251 ) // Warning about "needs to have dll-interface" on STL types

namespace Core
{
    struct DistortionModel;
}

namespace CameraLibrary
{
    class Frame;
    class cInputBase;
    class cCameraCommand;
    class cCameraModule;
    class cCameraListener;
    class cSyncFeatures;
    class cBlockControlBase;
    struct sStatusLightColor;
    struct sObjectModeSettings;
    struct sSyncSettings;
    enum eImagerGain;
    enum eStatusLEDs;
    enum eCameraState;
    using PIXEL = int;
    
    // LensType for Cameras which can have standard lens or a wide lens
	enum LensType
	{
		STANDARD_LENS = 0, // Lens is standard lens for the base camera
		WIDE_LENS = 1    // Lens is a wide lens
	};

    // Camera Body Type for Cameras which can change Body styles 
	enum CameraBodyStyle
	{
		PRIMEX = 0,
		SLIMX = 1
	};

    // Possible PoE configurations the camera can be in
    enum ePoEState
    {
        POECLASS3 = 0, // POE up to 13W class 0-3 
        POECLASS4 = 1, // POE+ 25.5W (works with slimx120)
        POECLASS6 = 2, // POE++ 51W class 6 (not used)
        POECLASS8 = 3, // POE++ 71.3W class 8 (works with slimx120 and px120)
        POECLASSUNKOWN = 4 
    };

    enum eRinglightType
    {
        NO_RINGLIGHT = 0,                       // Slim Cameras and other devices
        STANDARD,                       // USB Cameras 
        NARROW_LENS_AND_RINGLIGHT,      // Prime and PrimeX
        WIDE_LENS_AND_RINGLIGHT         // PrimeX W cameras
    };

    enum eFilterSwitchType
    {
        NO_FILTER_SWITCH = 0,           // No filter switch avilable
        STANDARD_FILTER_SWITCH,         // Default filter switch
        STATIC_850,                     // VersaX only
        BANDPASS_850,                   // VersaX only
        STATIC_940,                     // VersaX only
        BANDPASS_940                    // VersaX only
    };

    class CLAPI Camera : public cInputListener, public Window
    {
    public:
        Camera();
        virtual ~Camera();

        virtual bool isInDuplexMode() const;
        virtual void PacketTest( int packetCount ) { }

        // Intended Public Command Interface

        std::shared_ptr<const Frame> NextFrame();           // Fetch next available frame
        std::shared_ptr<const Frame> LatestFrame();         // Fetch latest frame (empties queue)

        const char* Name() const;                           // Returns name of camera

        void Start();                                       // Start Camera (starts frames)
        void Stop( bool TurnNumericOff = true );            // Stop  Camera (stops frames )

        bool IsCameraRunning() const;                       // Is camera started

        void SetNumeric( bool enabled, int value );         // Turn on/off numeric camera LEDs
        void SetExposure( int value );                      // Set Camera Exposure
        void SetThreshold( int value );                     // Set Camera Threshold
        virtual void SetIntensity( int value );             // Set Camera Intensity
        void SetPrecisionCap( int value );                  // Set Precision Packet Size Cap
        virtual void SetShutterDelay( int value );          // Set Shutter Delay (in usecs)
        virtual void SetStrobeOffset( int value );          // Set IR Illumination Delay

        virtual void SetBrightness( int value );            // Set Ringlight Brightness (Not supported on cameras yet)
        // Camera Frame Rate Controls ==================================================================

        // When calling SetFrameRate(), the value is a a of the camera's maximum
        // frame rate for USB devices, with the exception of the OptiTrack Flex13.
        // For Ethernet devices as well as the OptiTrack Flex 13, when calling
        // SetFrameRate(), the value is the desired frame rate.
        virtual void SetFrameRate( int value );
        virtual int FrameRate() const;

        virtual void SetFrameDecimation( int value );
        virtual int FrameDecimation() const;

        int GrayscaleDecimation() const;

        int PrecisionCap() const;
        virtual int ShutterDelay();

        // IR illumination delay
        virtual int StrobeOffset();

        int Exposure() const;
        int Threshold() const;

        // IR illumination intensity
        virtual int Intensity();

        void SetVideoType( Core::eVideoMode value );
        Core::eVideoMode VideoType() const;

        virtual bool IsVideoTypeSupported( Core::eVideoMode mode ) const;

        virtual bool IsVideoTypeSynchronous( Core::eVideoMode value = Core::UnknownMode ) const;

        float DataRate() const;
        float PacketSize() const;

        // Decimate Grayscale Size
        void SetGrayscaleDecimation( int value );

        void SendEmptyFrames( bool enable );
        void SendInvalidFrames( bool enable );

        // Only decode scene data that you actually use.
        void SetLateDecompression( bool enable );
        bool LateDecompression() const;

        // Device information
        uint32_t Serial() const;
        uint32_t MaskedSerialNumber() const;
        uint32_t GetPrefix() const;

        const char* SerialString() const;
        int Model() const;
        int SubModel() const;
        int Revision() const;
        int HardwareInterface() const;

#ifdef WIN32
        std::wstring SerialWideString() const;
#endif // WIN32


        // Ringlight info
        virtual eRinglightType RinglightType() const;
        virtual int RinglightWavelength() const;        // returns -1 if info not supported
        virtual std::string RinglightSerial() const;    // returns "" if not supported
        

        // Fetch hardware assigned Camera ID
        virtual int  CameraID() const;
        virtual bool CameraIDValid() const;
        virtual std::vector<unsigned char> UniqueCameraID() const {
            return {};
        }

        // Set IR Bandpass Filter
        virtual void SetIRFilter( bool enabled );
        virtual bool IRFilter() const;
        virtual bool IsFilterSwitchAvailable() const {
            return false;
        }
        virtual eFilterSwitchType FilterSwitchType() const;

        // Imager AGC control
        void SetAGC( bool enable );
        bool AGC() const;
        virtual bool IsAGCAvailable() const {
            return false;
        }

        // Imager AEC control
        void SetAEC( bool enable );
        bool AEC() const;
        virtual bool IsAECAvailable() const {
            return false;
        }

        void SetImagerGain( eImagerGain imagerGain );
        virtual eImagerGain ImagerGain() const;
        virtual bool IsImagerGainAvailable() const {
            return false;
        }
        virtual int  ImagerGainLevels() const;

        virtual void SetHighPowerMode( bool enable ) { }
        virtual bool HighPowerMode() const {
            return false;
        }
        virtual bool IsHighPowerModeAvailable() const {
            return false;
        }
        virtual bool IsHighPowerModeSupported() const {
            return false;
        }
        int LowPowerSetting() const;

        // Current camera frame rate (frames/sec)
        virtual int ActualFrameRate() const;

        // MJPEG Quality Level (0-125)
        void SetMJPEGQuality(int value, bool force = false );
        virtual int MJPEGQuality() const;
        virtual bool IsMJPEGAvailable() const {
            return false;
        }
        virtual int  MJPEGQualityIndex() const {
            return 0;
        }

        virtual bool IsContinuousIRAvailable() const {
            return false;
        }
        virtual void SetContinuousIR( bool enable ) { }
        virtual bool ContinuousIR() const;

        virtual void SetRinglightEnabledWhileStopped( bool enable );
        virtual bool RinglightEnabledWhileStopped() const;

        virtual bool IsHardwareFiltered( Core::eVideoMode value = Core::UnknownMode ) const;

        // SmartNav4's switch state
        int SwitchState();

        // Camera Health Information
        HealthMonitor* Health();
        const HealthMonitor* Health() const;

        // Distortion Model
        virtual void GetDistortionModel( Core::DistortionModel& model ) const { }

        // Imager Windowing ============================================================================

        // Reset Camera Window
        void ResetWindow();

        // Adjust Camera Window
        virtual void SetWindow( int x1, int y1, int x2, int y2 );

        // Imager windowing capability
        virtual bool IsWindowingSupported() const;

        // Returns exact coords of windowing given passed in coords. Current windowing size
        // can be queried via Top(), Left(), Width(), Height()
        virtual void CalcWindow( int& x1, int& y1, int& x2, int& y2 ) const;

        // Status LEDs =================================================================================

        // Turn Camera LEDs On/Off
        void SetLED( eStatusLEDs LED, bool enable );
        void SetAllLED( eStatusLEDs LED );

        // All Status LED to (0-->255)
        void SetStatusIntensity( int Intensity );

        // Number of status ring LEDs
        virtual int  StatusRingLightCount() const;

        virtual void SetStatusRingLights( int count, const sStatusLightColor* lightColors );
        virtual void SetStatusRingRGB( unsigned char R, unsigned char G, unsigned char B );

        // IR Illumination LEDs ========================================================================

        // IR Illumination ring presence
        virtual bool IsIRIlluminationAvailable() const;

        // Blocking ====================================================================================

        void SetEnableBlockingMask( bool enabled );
        bool IsBlockingMaskEnabled() const;

        void AddBlockingRectangle( int x1, int y1, int x2, int y2 );
        void RemoveBlockingRectangle( int x1, int y1, int x2, int y2 );
        void SetBitMaskPixel( int X, int Y, bool mask );
        void ClearBlockingMask();
        void GetBlockingMask( unsigned char* buffer, int bufferSize );
        void SetBlockingMask( const unsigned char* buffer, int bufferSize );
        virtual void UpdateBlockingMask();
        int BlockingMaskWidth() const;
        int BlockingMaskHeight() const;
        int BlockingGrid() const;


        // Get Max Object Diameter ( centroid diameter )
        // override this if camera's max object diameter is larger than 56 ( 64 - 2 * 4 )
        virtual int  MaxObjectDiameter() const;

        // Camera Physical Constants ======================================================----

        // Dimensions the the imager chip in the camera, in mm.
        virtual double ImagerWidth() const {
            return 0.0;
        }
        virtual double ImagerHeight() const {
            return 0.0;
        }

        virtual double FocalLength() const {
            return 0.0;
        }
        virtual int HardwareFrameRate() const {
            return 0;
        }

        // Number of imager pixels in the horizontal and vertical directions.
        virtual int PhysicalPixelWidth() const;
        virtual int PhysicalPixelHeight() const;

        // Additional =====================================================================----

        void SetTextOverlay( bool enable );
        void SetMarkerOverlay( bool enable );

        bool TextOverlay() const;
        bool MarkerOverlay() const;

        void SetName( const char* name );

        bool IsInitialized() const;
        bool IsDisconnected() const;

        virtual eCameraState State() const;
        Core::cUID UID() const;

        bool IsFrameQueueEmpty() const;
        size_t AllocatedFrameCount() const;
        bool HasAvailableFrames() const;

        cInputManager::ConnectionTypes ConnectionType() const; // Returns type of input USB, etc.
        virtual bool IsVirtual() const;

        // Less Commonly Used Public Methods

        virtual void AttachInput( cInputBase* input );     // Manually attach a camera input
        cInputBase* DetachInput();                         // Manually detach a camera input

        const char* DevicePath() const;                     // Returns Device Path for USB

        std::shared_ptr<cCameraCommand> SendCommand( std::unique_ptr<cCameraCommand> command ); // Push a camera command

        bool IsCommandQueueEmpty() const;                   // Know if camera is busy with
        // communicating with the device

        void AttachModule( cCameraModule* module );         // Attach additional functionality
        void RemoveModule( cCameraModule* module );         // Remove additional functionality
        int ModuleCount() const;                            // Number of attached modules
        cCameraModule* Module( int index );                 // Get pointer to attached module
        const cCameraModule* Module( int index ) const;     // Get pointer to attached module

        void AttachListener( cCameraListener* listener );   // Attach for camera events
        void RemoveListener( cCameraListener* listener );   // Remove camera listener
        void ListenerLock();                                // Used by Camera SDK sample
        void ListenerUnlock();                              // Used by Camera SDK sample
        void Shutdown();                                    // Permanently shutdown camera

        virtual bool IsCamera() const {
            return true;
        }      // Reports of device is a camera
        virtual bool IsHardwareKey() const {
            return false;
        } // For separation of cameras & keys
        virtual bool IsHub() const {
            return false;
        }        // For separation of cameras & Hubs
        virtual bool IsUSB() const {
            return true;
        }
        virtual bool IsEthernet() const {
            return false;
        }
        virtual bool IsTBar() const {
            return false;
        }
        virtual bool IsSyncAuthority() const {
            return false;
        }
        virtual bool IsBaseStation() const {
            return false;
        }
        virtual bool IsActive2() const {
            return false;
        }
        virtual bool IsWiredTag() const {
            return false;
        }
        virtual bool IsESync() const {
            return false;
        }

        virtual bool IsVersaX() const {
            return false;
        }
        virtual cSyncFeatures SyncFeatures();               // Return system synchronization features

        void SetObjectColor( PIXEL Color );                 // Set color for rasterized objects
        PIXEL ObjectColor();                                // Get rasterized segment color

        void SetDuplexObjectColor(PIXEL Color); // Set color objects in the video image when in duplex mode
        PIXEL DuplexObjectColor() const;        // Get color objects in the video image when in duplex mode

        virtual void FrameSize( Core::eVideoMode mode, int& width, int& height, float& scale ) const;

        virtual int MJPEGDecimationFactor() const;

        virtual void SetEnablePayload( bool enable );       // Some cameras support sending empty
        // frames by disabling payload.
        virtual bool IsEnablePayload() const;

        // Camera/Ringlight Temperature
        virtual bool IsCameraTempValid() const;             // Is camera temperature valid
        virtual float CameraTemp() const;                   // Camera temperature

        virtual bool IsRinglightTempValid() const;
        virtual float RinglightTemp() const;

        virtual ePoEState PoEState() const;               // Device PoE+ Status

        enum eLLDPDetection
        {
            LLDP_Automatic = 0,
            LLDP_Override
        };

        void SetLLDPDetection( eLLDPDetection detection ); // LLDP PoE+ detection
        virtual bool IsLLDPDetectionAvailable() {
            return false;
        } // feature detection
        eLLDPDetection LLDPDetection();

        // Camera Information
        virtual int MinimumExposureValue() const;           // Returns the minimum camera exposure
        virtual int MaximumExposureValue() const;           // Returns the maximum camera exposure

        virtual int MinimumFrameRateValue() const;          // Returns the minimum frame rate
        virtual int MaximumFrameRateValue() const;          // Returns the maximum frame rate
        virtual int MaximumFullImageFrameRateValue() const; // Returns the maximum full image frame rate

        virtual bool IsFrameRateValid( int frameRate ) const; // returns true if entered frame rate is valid for the device

        virtual int MinimumThreshold() const;               // Returns the minimum threshold
        virtual int MaximumThreshold() const;               // Returns the maximum threshold

        virtual int MinimumIntensity() const;               // Returns the minimum intensity
        virtual int MaximumIntensity() const;               // Returns the maximum intensity

        virtual int MaximumMJPEGRateValue() const;          // Returns the maximum MJPEG rate

        // Additional Camera Options
        virtual bool SetParameter( const char* parameterName, float parameterValue );
        virtual bool SetParameter( const char* parameterName, const char* parameterValue );

        // Device Non-Volatile Data Storage
        virtual int StorageMaxSize();

        // Device Non-Volatile File System
        virtual int LoadFile( const char* filename, unsigned char* buffer, int bufferSize ) const;
        virtual int LoadFile( const char* filename, Core::cSerializer& serial ) const;
        virtual bool SaveFile( const char* filename, unsigned char* buffer, int bufferSize ) const;
        virtual bool SaveFile( const char* filename, Core::cSerializer& serial ) const;

        // Device OptiHub awareness (Flex 3 & Flex 13 Only)

        enum eOptiHubConnectivity
        {
            NoOptiHubConnection,
            OptiHubV1,
            OptiHubV2
        };

        eOptiHubConnectivity OptiHubConnectivity() const;

        // Color Camera Commands
        virtual bool IsColor() const;

        virtual void SetColorMatrix( sColorMatrix matrix );
        virtual void SetColorGamma( float gamma );
        virtual void SetColorPrescalar( float R, float G1, float G2, float B );

        // Compression Mode: 0 = Variable Bit Rate, 1 = Constant Bit Rate
        virtual void SetColorCompression( int Mode, float Quality, float BitRate );

        virtual sColorMatrix ColorMatrix();
        virtual float ColorGamma() const;
        virtual sColorPrescalar ColorPrescalar();
        virtual int ColorMode() const;
        virtual float ColorCompression() const;
        virtual float ColorBitRate() const;

        virtual int CameraResolutionCount() const;
        virtual int CameraResolutionID() const;
        virtual sCameraResolution CameraResolution( int index ) const;
        virtual void SetCameraResolution( int ResolutionID );

        // Hardware Based Timing
        virtual void QueryHardwareTimeStampValue( int UserData );
        virtual bool IsHardwareTimeStampValueSupported();

        // Internal - for development
        virtual void SetColorEnhancement( float LNoiseThreshold,
            float LEdgeStrength, float LHaloSuppress, float RNoiseThreshold,
            float REdgeStrength, float RHaloSuppress );

        virtual void ColorEnhancement( float& LNoiseThreshold,
            float& LEdgeStrength, float& LHaloSuppress, float& RNoiseThreshold,
            float& REdgeStrength, float& RHaloSuppress );

        // Incoming Packet Processing
        virtual void PacketJunction( unsigned char* buffer, long bufferSize, unsigned long long startTimestamp, unsigned long long endTimestamp );

        // Hardware Based Timing

        virtual void QueryHardwareTimeInfo( const sHardwareTimeInfo& timeInfo ) const;
        virtual bool IsHardwareTimeInfoSupported() const;

        virtual bool IsCameraIDAssigned() {
            return false;
        };

        // Frame Prediction
        virtual bool ExpectFrameID( int frameID ) const {
            return true;
        }
        void SetLatestFrameID( int frameID );
        int LatestFrameID() const;

        // Direct JPEG Access (when camera is in MJPEG Mode)
        virtual int CompressedImageSize( const Frame* frame ) const;
        virtual int CompressedImage( const Frame* frame, unsigned char* buffer, int bufferSize ) const;

        
        // Set the time (microseconds) which active LEDs are exposed during the frame (only new Active devices)
        virtual void SetActiveLEDIlluminationTime( int value );

        // Returns the maximum active LED illumination time (microseconds)
        virtual int  MaximumActiveLEDIlluminationTimeValue() const {
            return 0;
        }

        // Set the pattern group for new active devices.
        // Each group is the index for blink patterns for 8 leds.
        // Some devices may use more than one group number if they have more than 8 leds.
        virtual void SetActivePatternGroup( int groupNumber );

        // Gets the on/off status of each led for new active devices. Each bit represents an LED; 0-on, 1-off.
        virtual unsigned int GetActiveLEDStatus() const {
            return -1;
        };

        virtual int ActiveTagCount() const {
            return 0;
        };

        virtual void SetIMUDecimationRate ( int rate );
        virtual void SetIMUFilterLevel ( int level );
        virtual void RecalibrateIMU ();

        virtual void SoftReset();


    protected:

    	virtual void Internal_LoadFirmware() { }

        virtual void Internal_PostInitialization() {}       // just used for active devices for now
        // gaurentees that tags are initialized after the basestations

        virtual void Internal_PreDisconnect() {}            // just used for active devices for now
        // gaurentees that tags are removed before the cameras and camera manager

        virtual void Internal_InitializeCamera();

        virtual void Internal_PopulateKnownConfigInfo();

        friend class cCommand_StorageWriteChunk;
        friend class cCommand_StorageReadChunk;
        virtual void Internal_StorageWriteChunk( int StartAddress, unsigned char* buffer, int bufferSize ) { }
        virtual void Internal_StorageReadChunk( int StartAddress, int Size ) { }

    };

    class CLAPI cCameraCommand
    {
    public:
        virtual ~cCameraCommand() = default;

        void SetCamera( Camera* cam );

        virtual bool WaitForCompletion();
        virtual bool IsComplete() const;
        virtual void SetComplete();
        virtual bool IsExpectingResponse() const;
        virtual int ExpectedResponsePacketType() const;

        virtual void CameraResponse( unsigned char* buffer, long bufferSize );
        virtual void CameraResponseMissing();

        virtual void Execute() = 0;

    protected:
        Camera* CameraPtr() const { return mCamera; }

    private:
        Camera* mCamera = nullptr;
    };

    class CLAPI cCameraListener
    {
    public:
        virtual ~cCameraListener() = default;

        virtual void FrameAvailable( const Camera& sender ) { }
        virtual void FrameOverflow( const Camera& sender ) { }
        virtual void ButtonPressed( const Camera& sender ) { }
        virtual void HardwareTimeStampValueResponse( const sTimeStampValueResponse& response ) { }

    };
}

#pragma warning( pop )
// NOLINTEND(misc-non-private-member-variables-in-classes, misc-unused-parameters)