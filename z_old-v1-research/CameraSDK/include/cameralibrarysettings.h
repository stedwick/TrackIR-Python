//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

// These are top level settings for the Camera SDK. These settings need to be set as early as possible
// during program execution and will not have any effect if changes to these settings are made after
// any calls into the CameraManager class.

#include "singleton.h"

class CLAPI cCameraLibraryStartupSettings : public CameraLibrary::Singleton<cCameraLibraryStartupSettings>
{
public:
    cCameraLibraryStartupSettings();

    enum eUSBDevices
    {
        USB_Disabled = 0,
        USB_Licensing_Only,
        USB_Enabled
    };

    void EnableUSBDevices( eUSBDevices enableState );
    eUSBDevices USBDevices() const;

    void EnableEthernetDevices( bool enable );
    bool IsEthernetDevicesEnabled() const;

    void EnableDevelopment();
    bool IsDevelopmentEnabled() const;

    void SetEnableFilterSwitch( bool enable );
    bool IsFilterSwitchEnabled() const;

    enum eLLDPOptions
    {
        LLDP_Automatic = 0,
        LLDP_Override
    };

    void SetLLDPDetectionDefault( eLLDPOptions lldpSetting );
    eLLDPOptions LLDPDetectionDefault() const;

    // DelayCameraConnections is intended to be set to true if the host application intends to attach
    // a listener to the CameraManager and control camera connectivity via ShouldConnectCamera >and<
    // the host application is not able to attach a listener to the Camera Library immediately thus, this
    // mechanism is needed to simply postpone camera connections until the listener can be attached.
    // Ethernet cameras only.
    void SetDelayCameraConnections( bool delayConnections );
    bool IsDelayCameraConnections() const;

};
