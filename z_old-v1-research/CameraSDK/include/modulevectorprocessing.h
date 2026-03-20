//======================================================================================================
// Copyright 2010, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include "cameramodulebase.h"
#include "modulevector.h"
#include "coremath.h"

constexpr double  VECTOR_MIN_VALUE = -10000.0;
constexpr double  VECTOR_MAX_VALUE = 10000.0;

namespace CameraLibrary
{
    class Camera;

    class CLAPI cVectorProcessingSettings
    {
    public:
        cVectorProcessingSettings();

        ~cVectorProcessingSettings() = default;

        bool TrueView;
        bool ShowProcessed;
        bool ReplaceOriginal;
        bool ShowPivotPoint;
        double SmoothingX;
        double SmoothingY;
        double SmoothingZ;
        double SmoothingRotational;

        double ScaleTranslationX;
        double ScaleTranslationY;
        double ScaleTranslationZ;
        double ScaleRotationYaw;
        double ScaleRotationPitch;
        double ScaleRotationRoll;
        double PivotOffsetX;
        double PivotOffsetY;
        double PivotOffsetZ;
        double ProcessedScale;

        double CenterX;
        double CenterY;
        double CenterZ;

        Core::Matrix mCenterOrientation;

        int Arrangement;
    };

    class CLAPI cModuleVectorProcessing : public cCameraModule
    {
    public:
        cModuleVectorProcessing() = default;
        ~cModuleVectorProcessing() = default;

        static cModuleVectorProcessing* Create();
        static void Destroy( cModuleVectorProcessing* object );

        cVectorProcessingSettings * Settings();
        virtual void SetSettings( cVectorProcessingSettings &newSettings );

        void PushData( cModuleVector *vectorModule );

        virtual void Recenter();

        virtual void Smooth( float amountX, float amountY, float amountZ,
            float   AmountRotational, double &mRawX, double &mRawY, double &mRawZ,
            double &mRawYaw, double &mRawPitch, double &mRawRoll ) { }

        void GetPosition( double &X, double &Y, double &Z );
        void GetOrientation( double &yaw, double &pitch, double &roll );

        int  MarkerCount();
        void GetResult( int index, float &X, float &Y, float &Z );

        void ResetVectorMinMaxValue(double& ioVectorVal) { if (ioVectorVal < VECTOR_MIN_VALUE || VECTOR_MAX_VALUE > ioVectorVal) ioVectorVal = 0.0; }

    };
};
