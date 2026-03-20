//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

// This is the >>> Prime Camera <<<

#include "camerarev11.h"

namespace CameraLibrary
{
    class CameraRev26 : public CameraRev11
    {
    public:
        CameraRev26( bool needBlockingController = true );

        // Camera overrides
        double ImagerWidth() const override;
        double ImagerHeight() const override;
        void GetDistortionModel( Core::DistortionModel& model ) const override;
        int MaximumFrameRateValue() const override;          // Returns the maximum frame rate
        int MaximumFullImageFrameRateValue() const override; // Returns the maximum full image frame rate
        bool IsFilterSwitchAvailable() const override;
        bool IsContinuousIRAvailable() const override;
        bool IsMJPEGAvailable() const override { return false; }
        bool IsVideoTypeSupported( Core::eVideoMode mode ) const override;
        void SetRinglightEnabledWhileStopped( bool enable ) override;
        bool RinglightEnabledWhileStopped() const override;
        int StatusRingLightCount() const override;
        void SetStatusRingLights( int count, const sStatusLightColor* lightColors ) override;
        bool IsImagerGainAvailable() const override;
        int ImagerGainLevels() const override;
        bool SetParameter( const char* parameterName, float parameterValue ) override;
        void QueryHardwareTimeStampValue( int userData ) override;
        void QueryHardwareTimeInfo( const sHardwareTimeInfo& timeInfo ) const override;
        bool IsHardwareTimeInfoSupported() const override;
        eRinglightType RinglightType() const override;
    };
}
