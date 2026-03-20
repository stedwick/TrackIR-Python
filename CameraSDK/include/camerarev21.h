//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

// This is the >>> Flex13 <<<

#include "camerarev12.h"
#include "camerarevisions.h"

namespace CameraLibrary
{
    class CLAPI CameraRev21 : public CameraRev12
    {
    public:
        CameraRev21();
        ~CameraRev21();

        bool IsFrameRateValid(int frameRate) const override;
        bool IsHighPowerModeAvailable() const override;
        bool IsFilterSwitchAvailable() const override;
        bool IsHighPowerModeSupported() const override;
        bool IsAGCAvailable() const override;
        bool IsAECAvailable() const override;
        bool IsHardwareFiltered( Core::eVideoMode Value = Core::UnknownMode ) const override;
        bool IsVideoTypeSupported( Core::eVideoMode mode ) const override;
        bool IsVideoTypeSynchronous( Core::eVideoMode Value = Core::UnknownMode ) const override;

        // Camera Physical Constants ======================================================----

        double ImagerWidth() const override;
        double ImagerHeight() const override;
        int HardwareFrameRate() const override;
        int PhysicalPixelWidth() const override;
        int PhysicalPixelHeight() const override;

        bool IsImagerGainAvailable() const override;
        int ImagerGainLevels() const override;

        void SetIntensity( int Value ) override;            // Set Camera Intensity

        // Imager Windowing ===============================================================----

        bool IsWindowingSupporting() const;
        void CalcWindow( int &X1, int &Y1, int &X2, int &Y2 ) const override;

        int MJPEGQualityIndex() const override;

        int MinimumExposureValue() const override;          // Returns the minimum camera exposure
        int MaximumExposureValue() const override;          // Returns the maximum camera exposure

        int MinimumFrameRateValue() const override;         // Returns the minimum frame rate
        int MaximumFrameRateValue() const override;         // Returns the maximum frame rate

        int ActualFrameRate() const override;               // Current camera frame rate (frames/sec)

        void GetDistortionModel( Core::DistortionModel &Model ) const override; // Distortion Model

    };

    struct sFlex13Object
    {
        float x;
        float y;
        float roundness;
        unsigned int total_luminosity;
        unsigned short diameter;
    };
}
