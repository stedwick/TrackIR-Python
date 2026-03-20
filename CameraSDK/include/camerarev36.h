//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once


#include "camerarev30.h"

namespace CameraLibrary
{
    class CameraRev36 : public CameraRev30
    {
    public:
        CameraRev36( bool needBlockingController = true );

        void GetDistortionModel( Core::DistortionModel &Model ) const override; // Distortion Model
        int MaximumFrameRateValue() const override;   // Returns the maximum frame rate

        // Direct H.264 Access (when camera is in Video Mode)

        // SetColorCompression Parameters:

        // Mode: 0 = Variable Bit Rate, 1 = Constant Bit Rate. Default: CBR
        // Quality: Value affects compression, resulting image quality, and data rate when the camera
        //    is in VBR Mode.  The range is 0.0 to 1.0 and adjusts image compression.  The default 
        //    value for this setting is 0.2.  This setting is specific to VBR Mode.
        // BitRate: Value affects compression, resulting image quality, and data rate when the
        //    camera is in CBR Mode.  The range is 0.0 to 1.0 as an approximate percentage of 100 MB/s.
        //    The default value of 0.5 will yield approximately 50 MB/s of H.264 compressed data from
        //    the camera.  By comparison setting a value of 0.2 would yield approximately 20 MB/s of
        //    H.264 compressed data from the camera.  This value is specific to CBR Mode.

        void SetColorCompression( int Mode, float Quality, float BitRate ) override;

        bool IsColor() const override;
        void SetColorMatrix( sColorMatrix Matrix ) override;
        void SetColorGamma( float Gamma ) override;
        void SetColorPrescalar( float R, float G1, float G2, float B ) override;
        void SetColorEnhancement( float LNoiseThreshold,
            float LEdgeStrength, float LHaloSuppress, float RNoiseThreshold,
            float REdgeStrength, float RHaloSuppress ) override;

        sColorMatrix ColorMatrix() override;
        float ColorGamma() const override;
        int ColorMode() const override;
        float ColorCompression() const override;
        sColorPrescalar ColorPrescalar() override;
        float ColorBitRate() const override;
        void ColorEnhancement( float &LNoiseThreshold, float &LEdgeStrength, float &LHaloSuppress, 
            float &RNoiseThreshold, float &REdgeStrength, float &RHaloSuppress ) override;

        bool IsMJPEGAvailable() const override { return false; }
        bool IsVideoTypeSupported( Core::eVideoMode mode ) const override;

        bool IsIRIlluminationAvailable() const override;

    };
}
