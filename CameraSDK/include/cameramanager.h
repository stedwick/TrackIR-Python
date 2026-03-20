//======================================================================================================-----
// Copyright 2010, NaturalPoint Inc.
//======================================================================================================-----
#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <memory>

#include "Core/Time.h"
#include "Core/UID.h"

#include "cameralibraryglobals.h"
#include "timebase.h"
#include "threading.h"
#include "cameratypes.h"
#include "inputmanagerbase.h"
#include "activetag.h"

#pragma warning( push )
#pragma warning( disable: 4251 ) // Warning about "needs to have dll-interface" on STL types

namespace std
{
    class mutex;
    class recursive_mutex;
}

namespace Core
{
    class cIReader;
    class cIWriter;
}

namespace Armory
{
    class cArmory;
}

namespace CameraLibrary
{
    class Camera;
    class CameraList;
    class HardwareKeyList;
    class HardwareDeviceList;
    class ActiveTagList;
    class HubList;
    class CameraEntry;
    class cCameraManagerListener;
    class cInputManager;

    struct sSyncSettings;

    using HardwareKey = Camera;
    using Hub = Camera;
    using cDevice = Camera;

    // Synchronization System ===============================================================----

    class CLAPI cSyncFeatures
    {
    public:
        cSyncFeatures();
        ~cSyncFeatures() = default;

        enum  eSyncSystemTypes
        {
            EthernetSyncSystem,
            USBSyncSystem,
            UnknownSyncSystem
        };

        eSyncSystemTypes SyncSystemType;

        bool CustomSyncSupport;
        bool ShutterGoggleSupportByManufacturer;
        bool ShutterGoggleSupportBySlider;
        bool RequireGogglesVideoFrameRate;

        int SyncOutputCount;
        bool SyncVesaStereoOut;

        float DefaultInternalSyncFrequency;

        bool SyncOffsetIsGlobal;

        bool GlassesInputSourceSupport[SyncInputSourceCount];
        bool SyncInputSourceSupport[SyncInputSourceCount];
        bool TriggerSourceSupport[SyncInputSourceCount];
        bool SyncInputTriggerSupport[SyncInputSourceCount];
        bool SyncOutput[CameraLibrary::SyncOutputCount];
        bool SyncOutputPolarity;
        bool WiredSyncSupport;
        bool SyncInternalFreq;
        bool RecordTriggering;
        bool InputMonitoring;
        bool SyncInDividerBySlider;
        bool SyncInputMultiplier;
    };

    struct CLAPI sSyncSettings
    {
        sSyncSettings();


        eSyncMode Mode;
        eSyncType SyncType;

        // Shutter Goggles

        int GoggleType;                 // 1 = Stereographics, 2 = NuVision 60Gx, 3= NuVision APG6000
        int VideoFrameRate;             // Video Frame Rate
        float CustomOffs;               // Custom Offset Slider Value (0-100)

        // Shutter Goggles Slider Approach

        long GogglesSliderFrameRate;    // Shutter Frame Rate 
        double GogglesSliderOffset;     // Shutter Slider Offset (0-100)

        // Custom Sync Settings

        int CameraExposure;             // Camera exposure (all cameras are locked to a single exposure)
        int ImagerScanRate;             // Camera Frame Rate

        eSyncCameraSync CameraSync;
        eSyncInputSource SyncInputSource;
        eSyncInputSource SyncInputTrigger;
        int SyncInputDivider;
        int SyncInputMultiplier;
        float InternalSyncGeneratorFrequency;
        float GlobalSyncOffset;
        float ExternalTriggerFreq;

        //=== First Sync Output ==============----

        eSyncOutputPhase SyncOutput1Phase;
        eSyncOutputPulseDuration SyncOutput1PulseDuration;
        eSyncOutputPolarity SyncOutput1Polarity;

        //=== Second Sync Output ============----

        eSyncOutputPhase SyncOutput2Phase;
        eSyncOutputPulseDuration SyncOutput2PulseDuration;
        eSyncOutputPolarity SyncOutput2Polarity;

        //=== Third Sync Output =============----

        eSyncOutputPhase SyncOutput3Phase;
        eSyncOutputPulseDuration SyncOutput3PulseDuration;
        eSyncOutputPolarity SyncOutput3Polarity;

        //=== Fourth Sync Output ============----

        eSyncOutputPhase SyncOutput4Phase;
        eSyncOutputPulseDuration SyncOutput4PulseDuration;
        eSyncOutputPolarity SyncOutput4Polarity;

        // VESA Stereo Out =================----

        eSyncOutputPhase SyncOutputVesaPhase;
        eSyncOutputPulseDuration SyncOutputVesaPulseDuration;
        eSyncOutputPolarity SyncOutputVesaPolarity;

        //=== Record Triggering ==============----

        eSyncInputSource SyncRecordTrigger;
        eSyncInputSource SyncRecordEdge;

        eUSBSyncInControl USBSyncInControl;
        bool RecordActive;

        // Helpers ==--

        eSyncOutputPhase* SyncOutputPhase( int SyncOutputIndex );
        eSyncOutputPulseDuration * SyncOutputPulseDuration( int SyncOutputIndex );
        eSyncOutputPolarity* SyncOutputPolarity( int SyncOutputIndex );

        eSyncOutputPhase* VesaSyncOutputPhase();
        eSyncOutputPulseDuration* VesaSyncOutputPulseDuration();
        eSyncOutputPolarity* VesaSyncOutputPolarity();

        bool ExternalSignalSetsFrameRate() const;
    };


    class CLAPI CameraManager
        : public cInputManagerListener
    {
    public:
        bool WaitForInitialization();                       // Optional execution stall until cameras are init'd
        bool AreCamerasInitialized();                       // Check and see if all attached cameras are init'd
        bool AreCamerasShutdown();                          // Check and see if all cameras are shutdown
        void Shutdown();                                    // Shutdown Camera Library

        std::shared_ptr<Camera> GetCameraBySerial( unsigned int Serial ); // Get a camera by camera serial number
        std::shared_ptr<Camera> GetCamera( const Core::cUID& UID ); // Get a camera by UID (UIDs come from CameraList)
        std::shared_ptr<Camera> GetCamera();                // Get an attached & initialized camera
        std::shared_ptr<Camera> GetPrimaryTBar();           // Gets the primary camera of a TBar

        void GetCameraList( CameraList &List );             // Used by CameraList to self-populate

        std::shared_ptr<HardwareKey> GetHardwareKey();      // Get an attached & initialized hardware key

        std::shared_ptr<cDevice> GetDevice( const Core::cUID& UID ) const; // Get device by UID (UIDs from HardwareDeviceList)
        std::shared_ptr<cDevice> GetSyncDevice() const;     // Get primary sync authority
        std::shared_ptr<Camera> GetBasestation();           // Get an initialized basestation

        void PrepareForSuspend();                           // Power Management to prepare for system suspend
        void ResumeFromSuspend();                           // Power Management to resume after system suspend

        Core::cTime StartTime() const;                      // System start time.
        double TimeStamp() const;                           // Fetch system timestamp since startup (in seconds)

        void RegisterListener( cCameraManagerListener* );   // Register Camera Manager listener
        void UnregisterListener();                          // Unregister Camera Manager listener

        void AddCamera( const std::shared_ptr<Camera>& cam ); // Add virtual cameras to camera manager list
        void RemoveCamera( const Camera* device );          // Remove cameras from camera manager list

        std::shared_ptr<Camera> GetBasestationBySerialString( std::string serialString ); // Get a basestation by its serial string
        std::shared_ptr<Camera> GetBasestationBySerial( unsigned int Serial );        // Get a basestation by its serial number
        std::shared_ptr<Camera> GetBasestationByTag( sActiveTagSettings& tag );       // Get a basestation by one of its connected tags
        bool GetActiveTagBySerial(unsigned int Serial, sActiveTagSettings& settings); // Get a tag by its tag serial
        void GetActiveTagList( ActiveTagList &List );

        void AddActiveTag(const sActiveTagSettings& tagSettings);   // add an active tag to cameramanager list
        void RemoveActiveTag(const sActiveTagSettings& tagSettings); // remove an active tag from cameramanager list

        static void SetCanGoLive( bool canGoLive );

        // scanning for newly connected cameras, and add them
        void ScanForCameras();                              

        void ApplySyncSettings( sSyncSettings  SyncSettings ); // Apply Synchronization Settings
        void GetSyncSettings( sSyncSettings &SyncSettings ); // Get Current Sync Settings
        const sSyncSettings & SyncSettings() const;         // Get Current Sync Settings
        void SoftwareTrigger();                             // Trigger to shutter cameras
        eSyncMode SyncMode();                               // Returns the Current Sync Mode
        void UpdateRecordingBit( bool Recording );          // Update real-time app info
        cSyncFeatures GetSyncFeatures();                    // Returns what sync features are
                                                            // available given connected hardware
        const char* SyncDeviceName();                       // Returns the name of the synch
                                                            // device name, or empty string if
                                                            // none connected.

        bool ShouldLockCameraExposures();                   // Should app force all camera exposures equal
        bool ShouldForceCameraRateControls();               // Should app force all camera frame rates equal
        bool ShouldApplySyncOnExposureChange();             // Should app reapply sync settings on exp change

        void SuggestCameraIDOrder( const std::vector<int>& cameraIDList ); // Suggest CameraID order
        void SetCustomCameraIDs( const std::map<int, int>& customCameraIDs, bool allowDuplicateIDs = false ); // Set custom camera IDs

        static Camera* CameraFactory( int Revision, bool Init = true, int Serial = 0
            , bool startCameraEngineThread = true, int subModel = 0 );    // Virtual Cameras

        // CameraManager Singleton Methods =============================================================---

        static CameraManager& X();                          // Access to CameraManager Singleton
        static CameraManager* Ptr();                        // Access to CameraManager Singleton
        static bool IsActive();                             // Is CameraManager Singleton instantiated?
        static void DestroyInstance();                      // Destroy CameraManager Singleton

    };

    class CLAPI CameraEntry
    {
    protected:
        friend class CameraList;
        friend class HardwareKeyList;
        friend class HardwareDeviceList;
        friend class ActiveTagList;
        friend class HubList;

        CameraEntry() = default;

        CameraEntry( const char* name, const char* serialString, unsigned int serial, const Core::cUID & id, int revision
            , eCameraState state, bool virtualDevice );

    public:
        Core::cUID UID() const;                             // Camera's universal unique ID
        unsigned int Serial() const;                        // Camera's serial number
        int Revision() const;                               // Camera's revision
        const char*  Name() const;                          // Camera's name
        eCameraState State() const;                         // Camera's state
        bool IsVirtual() const;
        const char* SerialString() { return (const char*) mSerialString; } // Camera's alphanumeric serial

    protected:
        friend class CameraManager;
        void SetName( const char* Name );
        void SetSerialString( const char* Name );
        void SetUID( const Core::cUID& UID );
        void SetSerial( unsigned int Value );
        void SetState( eCameraState State );
        void SetRevision( int Revision );
        void SetVirtual( bool Virtual );

    private:
        char mName[kCameraNameMaxLen];
        char mSerialString[kCameraNameMaxLen];
        Core::cUID mUID;
        unsigned int mSerialNumber;
        int mRevision;
        eCameraState  mState;
        bool mVirtual;
    };

    class CLAPI CameraList
    {
    public:
        CameraList();                                       // Create a CameraList to see what
        ~CameraList() = default;                            // cameras are available

        CameraEntry& operator[]( int index );               // Index the list by CameraList[index]
        int Count() const;                                  // Number of entries in the CameraList
        void Refresh();                                     // Repopulate the list

    protected:
        friend class CameraManager;
        void AddEntry( const CameraEntry & entry );

    private:
        std::vector<CameraEntry> mEntries;
        void ClearList();
    };

    class CLAPI HardwareKeyList
    {
    public:
        HardwareKeyList();                                  // Create a HardwareKeyList to see what
        ~HardwareKeyList() = default;                       // hardware keys are available

        CameraEntry& operator[]( int index );               // Index the list by HardwareKeyList[index]
        int Count() const;                                  // Number of entries in the HardwareKeyList

    protected:
        friend class CameraManager;
        void SetCount( int Value );

    private:
        int mCameraCount = 0;
        CameraEntry mCameraEntry[kMaxCameras];
    };

    class CLAPI HubList
    {
    public:
        HubList();                                          // Create a HubList to see what
        ~HubList() = default;                               // OptiHubs are available

        CameraEntry& operator[]( int index );               // Index the list by HubList[index]
        int Count() const;                                  // Number of entries in the HubList

    protected:
        friend class CameraManager;
        void SetCount( int Value );

    private:
        int mCameraCount = 0;
        CameraEntry mCameraEntry[kMaxCameras];
    };

    class CLAPI HardwareDeviceList
    {
    public:
        HardwareDeviceList();                               // Create a HardwareDeviceList to see what
        ~HardwareDeviceList() = default;                    // hardware devices are available

        CameraEntry& operator[]( int index );               // Index by HardwareDeviceList[index]
        int Count() const;                                  // Number of entries in the List

    protected:
        friend class CameraManager;
        void SetCount( int Value );

    private:
        int mCameraCount = 0;
        CameraEntry mCameraEntry[kMaxCameras];
    };


    class CLAPI ActiveTagList
    {
    public:
        ActiveTagList();                                       // Create a CameraList to see what
        ~ActiveTagList() = default;                            // cameras are available

        sActiveTagSettings& operator[](int index);               // Index the list by CameraList[index]
        int Count() const;                                  // Number of entries in the CameraList

    protected:
        friend class CameraManager;
        void SetCount(int Value);
        void AddEntry(const sActiveTagSettings& entry);

    private:
        std::vector<sActiveTagSettings> mActiveTags;
        int mActiveTagCount = 0;
    };



    class CLAPI cCameraManagerListener
    {
    public:
        virtual ~cCameraManagerListener() = default;

        // A camera has been connected to the system
        virtual void CameraConnected() { }

        // A camera has been removed from the system
        virtual void CameraRemoved() { }

        // Global synchronization settings have changed
        virtual void SyncSettingsChanged() { }

        // A camera has completed initialization
        virtual void CameraInitialized() { }

        // A synchronization device has initialized
        virtual void SyncAuthorityInitialized() { }

        // Synchronization device removed from the system
        virtual void SyncAuthorityRemoved() { }

        // An active tag has been initialized
        virtual void ActiveTagInitialized() { }

        // An active tag have been removed form the system
        virtual void ActiveTagRemoved() { }

        // Accept or reject an incoming camera connection request.
        virtual bool ShouldConnectCamera( const char* NetworkInterface, const char* CameraSerial );

        // Internal Use
        virtual void CameraMessage( int Type, int Value, int ID ) { }
        virtual Camera* RequestUnknownDeviceImplementation( int Revision );
        virtual void RequestDevicesUpdate() { }
    };
}

#pragma warning( pop )
