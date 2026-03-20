//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once

// System includes
#include <cstring> // for memcpy
#include <iostream>
#include <limits>
#include <algorithm>

// Windows has a nasty habit of defining macros for max and min that inevitably interfere with other code.
#pragma push_macro( "max" )
#pragma push_macro( "min" )
#undef max
#undef min

namespace Core
{
    /// <summary>
    /// A 3D vector representation, including a number of vector operations.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<typename T>
    class cVector3
    {
    public:
        cVector3() = default; // No initialization

        template <typename U>
        cVector3( const cVector3<U>& other )
        {
            mVals[0] = T( other.X() );
            mVals[1] = T( other.Y() );
            mVals[2] = T( other.Z() );
        }
        cVector3( T x, T y, T z )
        {
            mVals[0] = x;
            mVals[1] = y;
            mVals[2] = z;
        }
        template <typename U>
        cVector3( U x, U y, U z )
        {
            mVals[0] = T( x );
            mVals[1] = T( y );
            mVals[2] = T( z );
        }

        /// <summary>Set the vector values.</summary>
        void SetValues( T x, T y, T z ) { mVals[0] = x; mVals[1] = y; mVals[2] = z; }
        template <typename U>
        void SetValues( U x, U y, U z ) { mVals[0] = T( x ); mVals[1] = T( y ); mVals[2] = T( z ); }

        T X() const { return mVals[0]; }
        T& X() { return mVals[0]; }
        T Y() const { return mVals[1]; }
        T& Y() { return mVals[1]; }
        T Z() const { return mVals[2]; }
        T& Z() { return mVals[2]; }

        T operator[]( int idx ) const { return mVals[idx]; }
        T& operator[]( int idx ) { return mVals[idx]; }

        /// <summary>Access to the data array.</summary>
        const T* Data() const { return mVals; }

        /// <summary>Access to the data array.</summary>
        T* Data() { return mVals; }

        /// <summary>Scale the vector by the given scalar.</summary>
        void Scale( T scale )
        {
            mVals[0] *= scale;
            mVals[1] *= scale;
            mVals[2] *= scale;
        }

        /// <summary>Returns a scaled version of the vector.</summary>
        cVector3 Scaled( T scale ) const
        {
            cVector3   returnVal( *this );
            
            returnVal.Scale( scale );

            return returnVal;
        }

        /// <summary>Normalize the vector (in place) to unit length.</summary>
        void Normalize()
        {
            T   len = Length();

            if( len > (T) 0 )
            {
                Scale( ((T) 1 ) / len );
            }
        }

        /// <summary>Return a normalized version of the vector.</summary>
        cVector3 Normalized() const
        {
            cVector3   returnVal( *this );

            returnVal.Normalize();

            return returnVal;
        }

        /// <summary>Return true if normalized.</summary>
        bool IsNormalized() const
        {
            return ( abs( (T)1 - LengthSquared() ) < 1e-6 );
        }

        /// <summary>Calculate the squared length of the vector.</summary>
        T LengthSquared() const
        {
            return Dot( *this );
        }

        /// <summary>Calculate the length of the vector.</summary>
        T Length() const
        {
            return sqrt( LengthSquared() );
        }

        /// <summary>Calculate the squared distance to the given point.</summary>
        T DistanceSquared( const cVector3 &pnt ) const
        {
            T x = pnt.mVals[0] - mVals[0];
            T y = pnt.mVals[1] - mVals[1];
            T z = pnt.mVals[2] - mVals[2];

            return ( x * x + y * y + z * z );
        }

        /// <summary>Calculate the distance to the given point.</summary>
        T Distance( const cVector3 &pnt ) const
        {
            return sqrt( DistanceSquared( pnt ) );
        }

        /// <summary>Dot product of this with given vector.</summary>
        T Dot( const cVector3 &other ) const
        {
            return ( mVals[0] * other.mVals[0] + mVals[1] * other.mVals[1] + mVals[2] * other.mVals[2] );
        }

        /// <summary>Returns this cross other.</summary>
        cVector3 Cross( const cVector3 &other ) const
        {
            T x = mVals[1] * other.mVals[2] - mVals[2] * other.mVals[1];
            T y = mVals[2] * other.mVals[0] - mVals[0] * other.mVals[2];
            T z = mVals[0] * other.mVals[1] - mVals[1] * other.mVals[0];

            return cVector3( x, y, z );
        }

        /// <summary>Returns the angle (in radians) between this and another vector.</summary>
        T AngleBetween( const cVector3& other ) const
        {
            // angle between two vectors is defined by
            // dotProduct = a.x * b.x + a.y * b.y + a.z * b.z = a.len() * b.len * cos(angle)
            // 
            // thus: cos(angle) = dotProduct / (a.len * b.len) 
            // therefore: angle = acos( dotProduct / (a.len * b*len ) )

            // find (a.len * b.len)
            T len = Length() * other.Length();

            // bound result
            len = std::max<T>( std::numeric_limits<T>::min(), len );

            // find dotProduct / (a.len * b.len)
            T val = Dot( other ) / len;

            // bound result
            val = std::min<T>( (T)1, std::max<T>( (T)( -1 ), val ) );

            return acos( val );
        }

        /// <summary>
        /// Returns a linear interpolated vector some percentage 't' between two vectors.
        /// The parameter t is usually in the range [0,1], but this method can also be used to extrapolate
        /// vectors beyond that range.
        /// </summary>
        static cVector3 Lerp( const cVector3 &v1, const cVector3 &v2, T t )
        {
            T k1 = 1 - t;
            T k2 = t;

            T x = k1 * v1.mVals[0] + k2 * v2.mVals[0];
            T y = k1 * v1.mVals[1] + k2 * v2.mVals[1];
            T z = k1 * v1.mVals[2] + k2 * v2.mVals[2];

            cVector3 result( x, y, z );
            
            return result;
        }

        static void LineLineIntersect( const cVector3& p1, const cVector3& p2, const cVector3& p3, const cVector3& p4, cVector3& pa, cVector3& pb )
        {
            cVector3 p13, p43, p21;
            T d1343, d4321, d1321, d4343, d2121;
            T numer, denom;
            T mua, mub;

            p13.mVals[0] = p1.mVals[0] - p3.mVals[0];
            p13.mVals[1] = p1.mVals[1] - p3.mVals[1];
            p13.mVals[2] = p1.mVals[2] - p3.mVals[2];
            p43.mVals[0] = p4.mVals[0] - p3.mVals[0];
            p43.mVals[1] = p4.mVals[1] - p3.mVals[1];
            p43.mVals[2] = p4.mVals[2] - p3.mVals[2];
            p21.mVals[0] = p2.mVals[0] - p1.mVals[0];
            p21.mVals[1] = p2.mVals[1] - p1.mVals[1];
            p21.mVals[2] = p2.mVals[2] - p1.mVals[2];

            d1343 = p13.mVals[0] * p43.mVals[0] + p13.mVals[1] * p43.mVals[1] + p13.mVals[2] * p43.mVals[2];
            d4321 = p43.mVals[0] * p21.mVals[0] + p43.mVals[1] * p21.mVals[1] + p43.mVals[2] * p21.mVals[2];
            d1321 = p13.mVals[0] * p21.mVals[0] + p13.mVals[1] * p21.mVals[1] + p13.mVals[2] * p21.mVals[2];
            d4343 = p43.mVals[0] * p43.mVals[0] + p43.mVals[1] * p43.mVals[1] + p43.mVals[2] * p43.mVals[2];
            d2121 = p21.mVals[0] * p21.mVals[0] + p21.mVals[1] * p21.mVals[1] + p21.mVals[2] * p21.mVals[2];

            denom = d2121 * d4343 - d4321 * d4321;
            numer = d1343 * d4321 - d1321 * d4343;

            mua = numer / denom;
            mub = ( d1343 + d4321 * ( mua ) ) / d4343;

            pa.mVals[0] = p1.mVals[0] + mua * p21.mVals[0];
            pa.mVals[1] = p1.mVals[1] + mua * p21.mVals[1];
            pa.mVals[2] = p1.mVals[2] + mua * p21.mVals[2];

            pb.mVals[0] = p3.mVals[0] + mub * p43.mVals[0];
            pb.mVals[1] = p3.mVals[1] + mub * p43.mVals[1];
            pb.mVals[2] = p3.mVals[2] + mub * p43.mVals[2];
        }

        //====================================================================================
        // Mathematical and assignment operators
        //====================================================================================

        cVector3 operator+( const cVector3 &rhs ) const
        {
            return cVector3( mVals[0] + rhs.mVals[0], mVals[1] + rhs.mVals[1], mVals[2] + rhs.mVals[2] );
        }
        cVector3 operator+( T rhs ) const
        {
            return cVector3( mVals[0] + rhs, mVals[1] + rhs, mVals[2] + rhs );
        }
        cVector3& operator+=( const cVector3 &rhs )
        {
            mVals[0] += rhs.mVals[0];
            mVals[1] += rhs.mVals[1];
            mVals[2] += rhs.mVals[2];

            return *this;
        }
        cVector3& operator+=( T rhs )
        {
            mVals[0] += rhs;
            mVals[1] += rhs;
            mVals[2] += rhs;

            return *this;
        }

        cVector3 operator-( const cVector3 &rhs ) const
        {
            return cVector3( mVals[0] - rhs.mVals[0], mVals[1] - rhs.mVals[1], mVals[2] - rhs.mVals[2] );
        }
        cVector3 operator-( T rhs ) const
        {
            return cVector3( mVals[0] - rhs, mVals[1] - rhs, mVals[2] - rhs );
        }
        cVector3& operator-=( const cVector3 &rhs )
        {
            mVals[0] -= rhs.mVals[0];
            mVals[1] -= rhs.mVals[1];
            mVals[2] -= rhs.mVals[2];

            return *this;
        }
        cVector3& operator-=( T rhs )
        {
            mVals[0] -= rhs;
            mVals[1] -= rhs;
            mVals[2] -= rhs;

            return *this;
        }

        cVector3 operator*( T rhs ) const
        {
            return cVector3( mVals[0] * rhs, mVals[1] * rhs, mVals[2] * rhs );
        }

        cVector3 operator*( const cVector3 &rhs ) const
        {
            return cVector3( mVals[0] * rhs.mVals[0], mVals[1] * rhs.mVals[1], mVals[2] * rhs.mVals[2] );
        }

        cVector3& operator*=( T rhs )
        {
            mVals[0] *= rhs;
            mVals[1] *= rhs;
            mVals[2] *= rhs;

            return *this;
        }

        cVector3 operator/( T rhs ) const
        {
            return cVector3( mVals[0] / rhs, mVals[1] / rhs, mVals[2] / rhs );
        }
        cVector3& operator/=( T rhs )
        {
            mVals[0] /= rhs;
            mVals[1] /= rhs;
            mVals[2] /= rhs;

            return *this;
        }

        cVector3 operator-() const { return cVector3( -mVals[0], -mVals[1], -mVals[2] ); }

        template <typename U>
        cVector3& operator=( const cVector3<U>& other )
        {
            mVals[0] = T( other.X() );
            mVals[1] = T( other.Y() );
            mVals[2] = T( other.Z() );

            return *this;
        }

        //====================================================================================
        // Type conversion helpers
        //====================================================================================

        template<typename U>
        cVector3<U> ConvertToType() const
        {
            return cVector3<U>( (U) mVals[0], (U) mVals[1], (U) mVals[2] );
        }

        //====================================================================================
        // Comparison operators
        //====================================================================================

        /// <summary>Useful for basic sorting operations. A "strictly less-than" comparison.</summary>
        bool operator<( const cVector3 &rhs ) const
        {
            return ( mVals[0] < rhs.mVals[0] && mVals[1] < rhs.mVals[1] && mVals[2] < rhs.mVals[2] );
        }

        /// <summary>Useful for basic sorting operations. Defined as "not less than or equal to".</summary>
        bool operator>( const cVector3 &rhs ) const
        {
            return !(*this < rhs) && !(*this == rhs);
        }

        bool operator==( const cVector3 &rhs ) const
        {
            return ( mVals[0] == rhs.mVals[0] && mVals[1] == rhs.mVals[1]
                && mVals[2] == rhs.mVals[2] );
        }

        bool operator!=( const cVector3 &rhs ) const { return !( *this == rhs ); }

        /// <summary>Compare two vectors to within a tolerance.</summary>
        bool Equals( const cVector3& rhs, T tolerance ) const
        {
            return ( fabs( mVals[0] - rhs.mVals[0] ) < tolerance && fabs( mVals[1] - rhs.mVals[1] ) < tolerance
                && fabs( mVals[2] - rhs.mVals[2] ) < tolerance );
        }

        //====================================================================================
        // Helper constants
        //====================================================================================
        
        static const cVector3 kZero;

    private:
        T mVals[3];
    };

    template <typename T>
    const cVector3<T> cVector3<T>::kZero( (T)0, (T)0, (T)0 );

    using cVector3f = cVector3<float>;
    using cVector3d = cVector3<double>;

    //=========================================================================
    // Stream I/O operators
    //=========================================================================
    template< typename T>
    std::wostream& operator<<( std::wostream& os, const cVector3<T>& vec )
    {
        os << L"(" << vec[0] << L"," << vec[1] << L"," << vec[2] << L")";
        return os;
    }

    template<typename T>
    std::ostream& operator<<( std::ostream& os, const cVector3<T>& v )
    {
        T x = v.X();
        T y = v.Y();
        T z = v.Z();
        os.write( (char *) &x, sizeof( T ) );
        os.write( (char *) &y, sizeof( T ) );
        os.write( (char *) &z, sizeof( T ) );
        return os;
    }

    template<typename T>
    std::istream& operator>>( std::istream& is, cVector3<T>& v )
    {
        T x, y, z;
        is.read( (char *) &x, sizeof( T ) );
        is.read( (char *) &y, sizeof( T ) );
        is.read( (char *) &z, sizeof( T ) );
        v.SetValues( x, y, z );
        return is;
    }
}

#pragma pop_macro( "min" )
#pragma pop_macro( "max" )