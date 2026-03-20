//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

// This is the >>> V100:R1 <<<

#include "camera.h"
#include "camerarevisions.h"


namespace CameraLibrary
{
    class CLAPI CameraRev7 : public Camera
    {
    public:
        CameraRev7();
        virtual ~CameraRev7() = default;

        int CameraID() const override;

        // Camera Physical Constants ======================================================----

        void GetDistortionModel( Core::DistortionModel &model ) const override;

        // Camera Information =============================================================----

        int MinimumExposureValue() const override;
        int MaximumExposureValue() const override;

        double ImagerWidth() const override;
        double ImagerHeight() const override { return V100_IMAGERHEIGHT; }
        double FocalLength() const override { return V100_FOCALLENGTH; }
        int HardwareFrameRate() const override { return 100; }

        int ActualFrameRate() const override;

        bool IsAGCAvailable() const override { return true; }
        bool IsAECAvailable() const override { return true; }

        bool IsContinuousIRAvailable() const override { return true; }
        void SetContinuousIR( bool enable ) override;

        bool IsVideoTypeSupported( Core::eVideoMode mode ) const override;


        // Camera Options ================================================================-----

        bool SetParameter( const char* parameterName, float parameterValue ) override;

        //  Option Name   Default   Range   Description
        //==----------------------------------------------------------------------
        //  acbrightness    0.9     0-1.0   AEC/AGC Target Brightness
        //  acpixels        0.67    0-1.0   AEC/AGC Pixel Samples
        //  aecrate         0.13    0-1.0   AEC Update Rate (lower = faster)
        //  agcrate         0.13    0-1.0   AGC Update Rate (lower = faster)
        //  aecfunc         0       0,1,2   AEC Control Function
        //  agcfunc         0       0,1,2   AGC Control Function
        //  aecshutter      0.23    0-1.0   AEC Maximum Shutter Time

        // IR Illumination LEDs ===========================================================----

        bool IsIRIlluminationAvailable() const override;       // IR Illumination ring presence

    };
}
