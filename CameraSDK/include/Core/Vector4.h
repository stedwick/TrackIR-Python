//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once
 
// System includes
#include <cstring> //== for memcpy
#include <iostream>

// Local includes
#include "Core/BuildConfig.h"

namespace Core
{
    template<typename T> class cMatrix4;

    /// <summary>
    /// A 4D vector representation, including a number of vector operations.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<typename T>
    class cVector4
    {
    public:
        cVector4() = default; // No initialization

        template <typename U>
        cVector4( const cVector4<U>& other )
        {
            mVals[0] = T( other.X() );
            mVals[1] = T( other.Y() );
            mVals[2] = T( other.Z() );
            mVals[3] = T( other.W() );
        }
        cVector4( T x, T y, T z, T w )
        {
            mVals[0] = x;
            mVals[1] = y;
            mVals[2] = z;
            mVals[3] = w;
        }
        template <typename U>
        cVector4( U x, U y, U z, U w )
        {
            mVals[0] = T( x );
            mVals[1] = T( y );
            mVals[2] = T( z );
            mVals[3] = T( w );
        }
        cVector4( const T vals[4] )
        {
            ::memcpy( mVals, vals, 4 * sizeof( T ) );
        }

        /// <summary>Set the vector values.</summary>
        void SetValues( T x, T y, T z, T w ) { mVals[0] = x; mVals[1] = y; mVals[2] = z; mVals[3] = w; }
        template <typename U>
        void SetValues( U x, U y, U z, U w ) { mVals[0] = T( x ); mVals[1] = T( y ); mVals[2] = T( z ); mVals[3] = T( w ); }
        void SetValues( const T vals[4] ) { ::memcpy( mVals, vals, 4 * sizeof( T ) ); }

        T X() const { return mVals[0]; }
        T& X() { return mVals[0]; }
        T Y() const { return mVals[1]; }
        T& Y() { return mVals[1]; }
        T Z() const { return mVals[2]; }
        T& Z() { return mVals[2]; }
        T W() const { return mVals[3]; }
        T& W() { return mVals[3]; }

        T operator[]( int idx ) const { return mVals[idx]; }
        T& operator[]( int idx ) { return mVals[idx]; }

        /// <summary>Access to the data array.</summary>
        const float* Data() const { return mVals; }

        /// <summary>Post-multiply the vector by the given matrix.</summary>
        void Multiply( const cMatrix4<T>& m )
        {
            cMatrix4<T> m1( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, mVals[0], mVals[1], mVals[2], mVals[3] );
            m1 *= m;

            mVals[0] = m1.Value( 3, 0 );
            mVals[1] = m1.Value( 3, 1 );
            mVals[2] = m1.Value( 3, 2 );
            mVals[3] = m1.Value( 3, 3 );
        }

        /// <summary>Dot product of this with given vector.</summary>
        T Dot( const cVector4 &other ) const
        {
            return ( mVals[0] * other.mVals[0] + mVals[1] * other.mVals[1] + mVals[2] * other.mVals[2]
                + mVals[3] * other.mVals[3] );
        }

        //====================================================================================
        // Mathematical and assignment operators
        //====================================================================================

        cVector4 operator+( const cVector4 &rhs ) const
        {
            return cVector4( mVals[0] + rhs.mVals[0], mVals[1] + rhs.mVals[1], mVals[2] + rhs.mVals[2], mVals[3] + rhs.mVals[3] );
        }
        cVector4 operator+( T rhs ) const
        {
            return cVector4( mVals[0] + rhs, mVals[1] + rhs, mVals[2] + rhs, mVals[3] + rhs );
        }
        cVector4& operator+=( const cVector4 &rhs )
        {
            mVals[0] += rhs.mVals[0];
            mVals[1] += rhs.mVals[1];
            mVals[2] += rhs.mVals[2];
            mVals[3] += rhs.mVals[3];

            return *this;
        }
        cVector4& operator+=( T rhs )
        {
            mVals[0] += rhs;
            mVals[1] += rhs;
            mVals[2] += rhs;
            mVals[3] += rhs;

            return *this;
        }

        cVector4 operator-( const cVector4 &rhs ) const
        {
            return cVector4( mVals[0] - rhs.mVals[0], mVals[1] - rhs.mVals[1], mVals[2] - rhs.mVals[2], mVals[3] - rhs.mVals[3] );
        }
        cVector4 operator-( T rhs ) const
        {
            return cVector4( mVals[0] - rhs, mVals[1] - rhs, mVals[2] - rhs, mVals[3] - rhs );
        }
        cVector4& operator-=( const cVector4 &rhs )
        {
            mVals[0] -= rhs.mVals[0];
            mVals[1] -= rhs.mVals[1];
            mVals[2] -= rhs.mVals[2];
            mVals[3] -= rhs.mVals[3];

            return *this;
        }
        cVector4& operator-=( T rhs )
        {
            mVals[0] -= rhs;
            mVals[1] -= rhs;
            mVals[2] -= rhs;
            mVals[3] -= rhs;

            return *this;
        }

        cVector4 operator*( T rhs ) const
        {
            return cVector4( mVals[0] * rhs, mVals[1] * rhs, mVals[2] * rhs, mVals[3] * rhs );
        }
        cVector4& operator*=( T rhs )
        {
            mVals[0] *= rhs;
            mVals[1] *= rhs;
            mVals[2] *= rhs;
            mVals[3] *= rhs;

            return *this;
        }

        cVector4 operator/( T rhs ) const
        {
            return cVector4( mVals[0] / rhs, mVals[1] / rhs, mVals[2] / rhs, mVals[3] / rhs );
        }
        cVector4& operator/=( T rhs )
        {
            mVals[0] /= rhs;
            mVals[1] /= rhs;
            mVals[2] /= rhs;
            mVals[3] /= rhs;

            return *this;
        }

        cVector4 operator-() const { return cVector4( -mVals[0], -mVals[1], -mVals[2], -mVals[3] ); }

        template <typename U>
        cVector4& operator=( const cVector4<U>& other )
        {
            mVals[0] = T( other.X() );
            mVals[1] = T( other.Y() );
            mVals[2] = T( other.Z() );
            mVals[3] = T( other.W() );

            return *this;
        }

        //====================================================================================
        // Type conversion helpers
        //====================================================================================

        template<typename U>
        cVector4<U> ConvertToType() const
        {
            return cVector4<U>( (U) mVals[0], (U) mVals[1], (U) mVals[2], (U) mVals[3] );
        }

        //====================================================================================
        // Comparison operators
        //====================================================================================

        /// <summary>Useful for basic sorting operations. A "strictly less-than" comparison.</summary>
        bool operator<( const cVector4 &rhs ) const
        {
            return ( mVals[0] < rhs.mVals[0] && mVals[1] < rhs.mVals[1] && mVals[2] < rhs.mVals[2] && mVals[3] < rhs.mVals[3] );
        }

        /// <summary>Useful for basic sorting operations. Defined as "not less than or equal to".</summary>
        bool operator>( const cVector4 &rhs ) const
        {
            return !(*this < rhs) && !(*this == rhs);
        }

        bool operator==( const cVector4 &rhs ) const
        {
            return ( mVals[0] == rhs.mVals[0] && mVals[1] == rhs.mVals[1]
                && mVals[2] == rhs.mVals[2] && mVals[3] == rhs.mVals[3] );
        }

        bool operator!=( const cVector4 &rhs ) const { return !( *this == rhs ); }

        /// <summary>Compare two vectors to within a tolerance.</summary>
        bool Equals( const cVector4& rhs, T tolerance ) const
        {
            return ( fabs( mVals[0] - rhs.mVals[0] ) < tolerance && fabs( mVals[1] - rhs.mVals[1] ) < tolerance
                && fabs( mVals[2] - rhs.mVals[2] ) < tolerance && fabs( mVals[3] - rhs.mVals[3] ) < tolerance );
        }

        //====================================================================================
        // Helper constants
        //====================================================================================
        
        static const cVector4 kZero;

    private:
        T mVals[4];
    };

    template <typename T>
    const cVector4<T> cVector4<T>::kZero( 0, 0, 0, 0 );

    using cVector4f = cVector4<float>;
    using cVector4d = cVector4<double>;

    //=========================================================================
    // Stream I/O operators
    //=========================================================================
    template< typename T>
    std::wostream& operator<<( std::wostream& os, const cVector4<T>& vec )
    {
        os << L"(" << vec[0] << L"," << vec[1] << L"," << vec[2] << L"," << vec[3] << L")";
        return os;
    }

    template<typename T>
    std::ostream& operator<<( std::ostream& os, const cVector4<T>& v )
    {
        T x = v.X();
        T y = v.Y();
        T z = v.Z();
        T w = v.W();
        os.write( (char *) &x, sizeof( T ) );
        os.write( (char *) &y, sizeof( T ) );
        os.write( (char *) &z, sizeof( T ) );
        os.write( (char *) &w, sizeof( T ) );
        return os;
    }

    template<typename T>
    std::istream& operator>>( std::istream& is, cVector4<T>& v )
    {
        T x, y, z, w;
        is.read( (char *) &x, sizeof( T ) );
        is.read( (char *) &y, sizeof( T ) );
        is.read( (char *) &z, sizeof( T ) );
        is.read( (char *) &w, sizeof( T ) );
        v.SetValues( x, y, z, w );
        return is;
    }
}
