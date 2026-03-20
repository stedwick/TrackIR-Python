//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include "cameramodulebase.h"
#include "trajectorizer2d.h"

namespace CameraLibrary
{
    class CLAPI cModuleActiveLabel : public cCameraModule
    {
    protected:
        cModuleActiveLabel();
        ~cModuleActiveLabel();

    public:
        static cModuleActiveLabel * Create();
        static void Destroy( cModuleActiveLabel * object );

        void SetEnabled(bool Enabled);
        bool Enabled();

        void SetPatternDepth( int PatternDepth );
        int PatternDepth() const;

        unsigned int FullPattern() const;

        double TelemetryRunningAverageProcessingTime() const;

        void PrePostFrame( Camera* camera, const std::shared_ptr<Frame>& frame ) override;
    };
}
