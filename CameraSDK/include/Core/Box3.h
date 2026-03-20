//======================================================================================================
// Copyright 2014, NaturalPoint Inc.
//======================================================================================================
#pragma once

// Local includes
#include "Core/Vector3.h"

namespace Core
{
    /// <summary>
    /// A 3D box object, including methods for comparing boxes.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<typename T>
    class cBox3
    {
    public:
        cBox3() = default; // No initialization

        /// <summary>Initialize a new box with extrema.</summary>
        cBox3( const cVector3<T> &minPnt, const cVector3<T> &maxPnt )
        {
            // TODO : Put checks in place to ensure passed values are arranged correctly.
            SetValues( minPnt, maxPnt );
        }

        /// <summary>Set the vector values.</summary>
        void SetValues( const cVector3<T> &minPnt, const cVector3<T> &maxPnt )
        {
            for( int i = 0; i < 3; ++i )
            {
                if( minPnt[i] < maxPnt[i] )
                {
                    mMin[i] = minPnt[i];
                    mMax[i] = maxPnt[i];
                }
                else
                {
                    mMin[i] = maxPnt[i];
                    mMax[i] = minPnt[i];
                }
            }
        }

        /// <summary>Retrieve components of the box.</summary>
        cVector3<T> Min() const { return mMin; }
        cVector3<T> Max() const { return mMax; }

        /// <summary>Returns true if the box has no area.</summary>
        bool Empty() const { return ( mMin == mMax ); }

        //==============================================================================================
        // Mathematical and assignment operators
        //==============================================================================================

        cBox3 operator-() const { return cBox3( -mMin, -mMax ); }

        //==============================================================================================
        // Comparison operators
        //==============================================================================================

        bool operator==( const cBox3 &rhs ) const
        {
            return ( mMin == rhs.mMin && mMax == rhs.mMax );
        }

        bool operator!=( const cBox3 &rhs ) const { return !( *this == rhs ); }

        /// <summary>Compare two vectors to within a tolerance.</summary>
        bool Equals( const cBox3 &rhs, T tolerance ) const
        {
            tolerance *= tolerance;

            return ( ( mMin - rhs.mMin ).LengthSquared() < tolerance
                && ( mMax - rhs.mMax ).LengthSquared() < tolerance );
        }

        //==============================================================================================
        // Helper constants
        //==============================================================================================
        
        static const cBox3<T> kZero;

    private:
        cVector3<T> mMin;
        cVector3<T> mMax;
    };

    template <typename T>
    const cBox3<T> cBox3<T>::kZero( cVector3<T>( 0, 0 ), cVector3<T>( 0, 0 ) );

    using cBox3f = cBox3<float>;
    using cBox3d = cBox3<double>;
    using cBox3i = cBox3<int>;
}
