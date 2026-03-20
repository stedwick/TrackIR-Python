//======================================================================================================
// Copyright 2013, NaturalPoint Inc.
//======================================================================================================
#pragma once

// Local includes
#include "Core/Vector3.h"
#include "Core/Quaternion.h"

namespace Core
{
    /// <summary>Represents a "simple" transform (i.e. scale is assumed to be 1).</summary>
    template <typename T>
    class cSimpleTransform
    {
    public:
        cSimpleTransform() : mTranslation( cVector3<T>::kZero ), mRotation( cQuaternion<T,true>::kIdentity )
        {
        }
        cSimpleTransform( const cVector3<T> &translation, const cQuaternion<T,true> &rotation ) :
            mTranslation( translation ), mRotation( rotation )
        {
        }

        /// <summary>Set the translation component.</summary>
        inline void SetTranslation( const cVector3<T> &translate )
        {
            mTranslation = translate;
        }

        /// <summary>Set the rotation component.</summary>
        inline void SetRotation( const cQuaternion<T,true> &rotate )
        {
            mRotation = rotate;
        }

        /// <summary>Accumulates the given rotation into the transform.</summary>
        inline void Rotate( const cQuaternion<T,true> &rotation )
        {
            mRotation = mRotation.Times( rotation );
        }

        /// <summary>
        /// Accumulates the given translation into the transform. Given translation is understood to
        /// to be in the local coordinate system.
        /// </summary>
        inline void Translate( const cVector3<T> &translation )
        {
            mTranslation += mRotation.Rotate( translation );
        }

        /// <summary>Retrieve the translation component.</summary>
        inline const cVector3<T>& Translation() const { return mTranslation; }

        /// <summary>Retrieve the rotation component.</summary>
        inline const cQuaternion<T,true>& Rotation() const { return mRotation; }
        
        /// <summary>Invert this transform in place.</summary>
        inline void Invert()
        {
            mRotation.Invert();
            mTranslation = mRotation.Rotate( -mTranslation );
        }

        /// <summary>Transforms our coordinate system by the given transform.</summary>
        inline void Transform( const cSimpleTransform &transform )
        {
            // this = this.Times(transform)
            mTranslation += mRotation.Rotate( transform.mTranslation );
            mRotation = mRotation.Times( transform.mRotation );
        }

        /// <summary>Transforms our coordinate system by the inverse of the given transform.</summary>
        inline void InverseTransform( const cSimpleTransform &transform )
        {
            // this = transform.Inverse().Times(this)
            mTranslation = transform.InverseTransform( mTranslation );
            mRotation  = transform.mRotation.Inverse().Times( mRotation );
        }

        /// <summary>
        /// Transform the given point through this transform. Transform order is Rotate-Translate by convention.
        /// </summary>
        inline cVector3<T> Transform( const cVector3<T> &point ) const
        {
            return cVector3<T>( mRotation.Rotate( point ) + mTranslation );
        }

        /// <summary>
        /// Transform the given point through the inverse of this transform. Transform order is Rotate-Translate by convention.
        /// </summary>
        inline cVector3<T> InverseTransform( const cVector3<T> &point ) const
        {
            return cVector3<T>( mRotation.Inverse().Rotate( point - mTranslation ) );
        }

        /// <summary>Returns true if the transform is the identity (i.e. no transform).</summary>
        inline bool IsIdentity() const
        {
            return ( mTranslation == cVector3<T>::kZero && mRotation == cQuaternion<T,true>::kIdentity );
        }

        /// <summary>Does a comparison with another transform. This is an approximate comparison.</summary>
        inline bool Equals( const cSimpleTransform& other, T tolerance ) const
        {
            return ( mTranslation.Equals( other.mTranslation, tolerance ) && mRotation.Equals( other.mRotation, tolerance ) );
        }

        /// <summary>Multiplies a global transform by an offset transform.</summary>
        inline static cSimpleTransform Product( cSimpleTransform globalTransform, const cSimpleTransform& offsetTransform )
        {
            globalTransform.Transform( offsetTransform );
            return globalTransform;
        }

        /// <summary>Returns a zero transform.  This means 'no translation or rotation'.</summary>
        static const cSimpleTransform kZeroTransform;

        //====================================================================================
        // Comparison operators
        //====================================================================================

        bool operator==( const cSimpleTransform& rhs ) const
        {
            return ( mTranslation == rhs.mTranslation && mRotation == rhs.mRotation );
        }

        bool operator!=( const cSimpleTransform& rhs ) const
        {
            return ( mTranslation != rhs.mTranslation || mRotation != rhs.mRotation );
        }

        //====================================================================================
        // Type conversion helpers
        //====================================================================================

        template<typename U>
        cSimpleTransform<U> ConvertToType() const
        {
            return Core::cSimpleTransform<U>( mTranslation.ConvertToType<U>(), mRotation.ConvertToType<U>() );
        }

    private:
        cVector3<T> mTranslation;
        cQuaternion<T,true> mRotation;
    };

    template <typename T>
    const cSimpleTransform<T> cSimpleTransform<T>::kZeroTransform( cVector3<T>::kZero, cQuaternion<T,true>::kIdentity );

    template<>
    const cSimpleTransform<float> cSimpleTransform<float>::kZeroTransform(cVector3<float>::kZero, cQuaternion<float, true>(0.0f, 0.0f, 0.0f, 1.0f)); 
    template<>
    const cSimpleTransform<double> cSimpleTransform<double>::kZeroTransform(cVector3<double>::kZero, cQuaternion<double, true>(0.0, 0.0, 0.0, 1.0));

    // Helpful type definitions

    using cSimpleTransformf = cSimpleTransform<float> CORE_API;
    using cSimpleTransformd = cSimpleTransform<double> CORE_API;
}
