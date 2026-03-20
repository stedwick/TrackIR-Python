//======================================================================================================
// Copyright 2019, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include "Core/DebugSystem.h"
#include "Core/SimpleTransform.h"
#include "Core/UID.h"
#include "Core/Label.h"
#include "Core/AssetTypes.h"

namespace Core
{
    struct sConstraintData
    {
        // Distance from model-position to marker, if tracked.
        float Distance() const
        {
            return tracked ? constraintPosition.Distance( markerPosition ) : 0;
        }

        bool Tracked() const
        {
            return tracked;
        }

        Core::cLabel label = Core::cLabel::kInvalid;     // Label of the constraint.
        cVector3f constraintPosition = cVector3f::kZero; // World-space constraint location.
        cVector3f markerPosition = cVector3f::kZero;     // World-space actual marker location.
        float size = 0.0f;                               // Calculated size of the 3D marker.
        bool tracked = false;                            // Whether a marker was assigned to this constraint (markerPosition is valid).
    };

    class cBoneAnimation
    {
    public:

        cBoneAnimation() : mID( Core::cUID::kInvalid ), mConstraintCount( 0 ), mTracked( false ), mDofLimitViolated( false )
        {
        }

        cBoneAnimation( const Core::cUID &id, int constraintCount ) :
            mID( id ), mConstraintCount( constraintCount ), mTracked( false ), mDofLimitViolated( false )
        {
        }

        cBoneAnimation( const Core::cUID &id, const Core::cSimpleTransformf &transform, const Core::cSimpleTransformf &localTransform, int constraintCount ) :
            mID( id ), mTransform( transform ), mLocalTransform( localTransform ), mConstraintCount( constraintCount ), mTracked( false ), mDofLimitViolated( false )
        {
        }

        const Core::cUID& ID() const
        {
            return mID;
        }

        int ConstraintCount() const
        {
            return mConstraintCount;
        }

        void SetTracked( bool onOff )
        {
            mTracked = onOff;
        }

        bool Tracked() const
        {
            return mTracked;
        }

        void SetDofLimitViolated( bool onOff )
        {
            mDofLimitViolated = onOff;
        }

        bool DofLimitViolated() const
        {
            return mDofLimitViolated;
        }

        void SetMeanMarkerError( float error ) const
        {
            mMeanMarkerError = error;
        }

        float MeanMarkerError() const
        {
            return mMeanMarkerError;
        }

        void SetLocalPosition( const Core::cVector3f &position )
        {
            mLocalTransform.SetTranslation( position );
        }

        const Core::cVector3f& LocalPosition() const
        {
            return mLocalTransform.Translation();
        }

        const Core::cRotationf& LocalRotation() const
        {
            return mLocalTransform.Rotation();
        }

        void SetLocalRotation( const Core::cRotationf &rotation )
        {
            mLocalTransform.SetRotation( rotation );
        }

        const Core::cSimpleTransformf& LocalTransform() const
        {
            return mLocalTransform;
        }

        void SetLocalTransform( const Core::cSimpleTransformf &transform )
        {
            mLocalTransform = transform;
        }

        const Core::cVector3f& Position() const
        {
            return mTransform.Translation();
        }

        void SetPosition( const Core::cVector3f &position )
        {
            mTransform.SetTranslation( position );
        }

        const Core::cRotationf& Rotation() const
        {
            return mTransform.Rotation();
        }

        void SetRotation( const Core::cRotationf &rotation )
        {
            mTransform.SetRotation( rotation );
        }

        const Core::cSimpleTransformf& Transform() const
        {
            return mTransform;
        }

        void SetTransform( const Core::cSimpleTransformf &transform )
        {
            mTransform = transform;
        }

        void UpdateTransform( const Core::cSimpleTransformf &parentTransform )
        {
            mTransform = parentTransform;
            mTransform.Transform( mLocalTransform );
        }

        const Core::cMatrix4f& Matrix( Core::cMatrix4f &m ) const
        {
            m.RotateTranslate( mTransform.Rotation(), mTransform.Translation() );
            return m;
        }

    private:
        Core::cUID mID;                          // ID of the originating bone or asset
        bool mTracked;                           // True if bone is currently tracking.
        bool mDofLimitViolated;                  // True if an associated DOF hits its limit (for visualisation)
        mutable float mMeanMarkerError = 0.0f;   // Average marker-to-model distance error

        int mConstraintCount;                    // Number of constraints attached to the bone.

        Core::cSimpleTransformf mLocalTransform; // Local position & rotation
        Core::cSimpleTransformf mTransform;      // World-space position & rotation
    };

}
