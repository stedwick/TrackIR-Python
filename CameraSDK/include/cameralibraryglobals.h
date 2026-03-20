//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include "cameracommonglobals.h"


namespace CameraLibrary
{

    const int kMaxCameras             = 320;
    const int kCameraNameMaxLen       = 128;
    const int kDevicePathLen          = 256;
    // TODO When we know 10 bit sizes update these constant definitions and names to accommodate 8 and 10 bit gray scale.
    constexpr int kPrimeX120_8bitPacketSizeLarge = 1388;
    constexpr int kPrimeX120_8bitLargePacketsPerFrame = 6144;
    constexpr int kPrimeX120_8bitPacketSizeSmall = 1386;
    constexpr int kPrimeX120_8bitSmallPacketsPerFrame = 3072;
    const int kMax8bitGreyScaleBufferLen = (kPrimeX120_8bitPacketSizeLarge* kPrimeX120_8bitLargePacketsPerFrame) + (kPrimeX120_8bitPacketSizeSmall * kPrimeX120_8bitSmallPacketsPerFrame);

    // Camera & Camera Packet Processing

    const int kMaxSegmentsPerFrame    = 4000;
    const int kMaxObjectsPerFrame     = 2000;
    const int kMaxObjectLinksPerFrame = 500;

    const int kFilenameMaxLen         = 260;
    const int kHealthTextMaxLen       = 40;
    const int kMaxInteger             = 9999999;
    const int kMaxShort               = 32767;

    // OS specific max packet sizes : http://support.microsoft.com/kb/832430
    // XP/2K3 for EHCI Bulk = 3344K 
    const int kMaxUSBPacketSize       = 800*1024;      // needs to be a multiple of endpoint size (512)
    const int kMaxFlex13USBPacketSize = 1280*1024+512; // needs to be a multiple of endpoint size (512)
    const int kMaxFullSpeedPacketSize = 32768;         // max size for USB "full-speed" (non-"hi-speed") devices. larger requests may cause crashes.
    const int kMaxPacketSize          = (4096 * 3072 * 10)/8 + 512; // Ethernet Max Packet Size

    // Max number of LEDS supported on Status Ring Lights
	const int kMax_Status_Ring_Leds    = 20; 
};
