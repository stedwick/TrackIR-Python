//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

// This is the >>> OptiTrack S250e <<<

#include "camera.h"
#include "DuplexFrameHelper.h"

#pragma warning( push )
#pragma warning( disable: 4251 ) // Warning about "needs to have dll-interface" on STL types


namespace CameraLibrary
{
    class cModuleIJGDecompressor;


    class CLAPI CameraRev11 : public Camera
    {
    public:
        CameraRev11( bool needBlockingController = true );
        ~CameraRev11();

        // Camera overrides
        bool IsFilterSwitchAvailable() const override;
        bool IsHardwareFiltered( Core::eVideoMode value = Core::UnknownMode ) const override;
        bool IsVideoTypeSynchronous( Core::eVideoMode value = Core::UnknownMode ) const override;
        bool IsVideoTypeSupported( Core::eVideoMode mode ) const override;
        bool IsUSB() const override { return false; };
        bool IsEthernet() const override { return true; };
        bool IsMJPEGAvailable() const override;
        int  MJPEGQualityIndex() const override;
        bool IsContinuousIRAvailable() const override;
        void SetContinuousIR( bool Enable ) override;
        int MinimumExposureValue() const override;
        int MaximumExposureValue() const override;
        int MinimumFrameRateValue() const override;
        int MaximumFrameRateValue() const override;
        int MaximumMJPEGRateValue() const override;
        int HardwareFrameRate() const override;
        int ActualFrameRate() const override;
        void GetDistortionModel( Core::DistortionModel &model ) const override;
        double ImagerWidth() const override;
        double ImagerHeight() const override;

        bool IsCameraTempValid() const override;
        float CameraTemp() const override;

        bool IsRinglightTempValid() const override;
        float RinglightTemp() const override;

        ePoEState PoEState() const override;

        void SetEnablePayload( bool enable ) override;
        bool IsEnablePayload() const override;

        bool IsIRIlluminationAvailable() const override;

        void PacketJunction( unsigned char* buffer, long bufferSize, unsigned long long startTimestamp, 
            unsigned long long endTimestamp ) override;

    };

}

#pragma warning( pop )
