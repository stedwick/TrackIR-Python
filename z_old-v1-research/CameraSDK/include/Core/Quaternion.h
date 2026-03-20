//======================================================================================================
// Copyright 2013, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <limits>
#include <cmath>

#include "Platform.h"

// Local includes
#include "Core/BuildConfig.h"
#include "Vector3.h"
#include "Matrix4.h"

namespace Core
{
    /// <summary>
    /// A generalized quaternion value, which can also be used in normalized or auto-normalized form to represent
    /// a rotation value.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<typename T, bool AutoNormalize>
    class cQuaternion
    {
    public:
        cQuaternion() : mX( 0 ), mY( 0 ), mZ( 0 ), mW( 1 )
        {
        }

        cQuaternion( T x, T y, T z, T w ) : mX( x ), mY( y ), mZ( z ), mW( w )
        {
            if( AutoNormalize )
            {
                Normalize();
            }
        }

        template<typename U>
        cQuaternion( U x, U y, U z, U w )
        {
            mX = (T) x;
            mY = (T) y;
            mZ = (T) z;
            mW = (T) w;

            if( AutoNormalize )
            {
                Normalize();
            }
        }

        template <bool DoAuto>
        cQuaternion( const cQuaternion<T, DoAuto>& val )
        {
            ::memcpy( &mX, val.Data(), 4 * sizeof( T ) );

            if( AutoNormalize )
            {
                Normalize();
            }
        }

        template <typename U, bool DoAuto>
        cQuaternion( const cQuaternion<U, DoAuto>& val )
        {
            mX = T( val.X() );
            mY = T( val.Y() );
            mZ = T( val.Z() );
            mW = T( val.W() );

            if( AutoNormalize )
            {
                Normalize();
            }
        }

        /// <summary>Set the quaternion values.</summary>
        void SetValues( T x, T y, T z, T w )
        {
            mX = x;
            mY = y;
            mZ = z;
            mW = w;

            if( AutoNormalize )
            {
                Normalize();
            }
        }

        T X() const { return mX; }
        T Y() const { return mY; }
        T Z() const { return mZ; }
        T W() const { return mW; }

        T operator[]( int idx ) const
        {
            switch( idx )
            {
            case 0:
                return mX;
            case 1:
                return mY;
            case 2:
                return mZ;
            case 3:
                return mW;
            }

            throw std::out_of_range( "Quaternion index out of range. 0 <= index < 4" );
        }

        /// <summary>Access to the data array.</summary>
        const T* Data() const { return &mX; }

        /// <summary>Access to the data array.</summary>
        T* Data() { return &mX; }

        cQuaternion operator-() const
        {
            return cQuaternion( -mX, -mY, -mZ, -mW );
        }

        /// <summary>
        // Left to right quaternion multiplication
        // <code>result = this * q;</code>
        /// </summary>
        cQuaternion Times( const cQuaternion &q ) const
        {
            return cQuaternion(
                mW * q.mX + mX * q.mW - mZ * q.mY + mY * q.mZ,
                mW * q.mY + mY * q.mW - mX * q.mZ + mZ * q.mX,
                mW * q.mZ + mZ * q.mW - mY * q.mX + mX * q.mY,
                mW * q.mW - mX * q.mX - mY * q.mY - mZ * q.mZ );
        }

        bool operator==( const cQuaternion& q ) const
        {
            return ( mX == q.mX && mY == q.mY && mZ == q.mZ && mW == q.mW );
        }

        bool operator!=( const cQuaternion& q ) const
        {
            return ( mX != q.mX || mY != q.mY || mZ != q.mZ || mW != q.mW );
        }

        /// <summary>Less-than comparator, used mostly for sorting.</summary>
        bool operator<( const cQuaternion& q ) const
        {
            return ( mX < q.mX && mY < q.mY && mZ < q.mZ && mW < q.mW );
        }

        /// <summary>Compare two quaternions to within a tolerance. Also accounts for the possibility that
        /// the quaternion is on the opposite side of the hypersphere.</summary>
        bool Equals( const cQuaternion& q, T tolerance ) const
        {
            if( abs( mW - q.mW ) <= tolerance )
            {
                return ( abs( mX - q.mX ) <= tolerance && abs( mY - q.mY ) <= tolerance
                    && abs( mZ - q.mZ ) <= tolerance );
            }
            // May be equivalent by having all opposite signs.
            return ( abs( mX + q.mX ) <= tolerance && abs( mY + q.mY ) <= tolerance
                && abs( mZ + q.mZ ) <= tolerance && abs( mW + q.mW ) <= tolerance );
        }

        /// <summary>Returns the dot product with another quaternion.</summary>
        inline T Dot( const cQuaternion& q ) const
        {
            return ( mX * q.mX + mY * q.mY + mZ * q.mZ + mW * q.mW );
        }

        /// <summary>Calculate the norm of the quaternion (i.e. the sum of squares of the vector components).</summary>
        inline T Norm() const
        {
            return ( mX * mX + mY * mY + mZ * mZ + mW * mW );
        }

        /// <summary>Calculate the magnitude of the quaternion. For auto-normalized quaternions, this value will always be one.</summary>
        inline T Magnitude() const
        {
            return std::sqrt( Norm() );
        }

        /// <summary>Calculate and return a unit quaternion from this quaternion.</summary>
        inline cQuaternion Normalized() const
        {
            return ( AutoNormalize ? *this : Scaled( 1 / Magnitude() ) );
        }

        /// <summary>Normalize to a unit quaternion.</summary>
        inline void Normalize()
        {
            T mag = Magnitude();

            if( mag > 0 )
            {
                InternalScale( T( 1 ) / mag );
            }
            else
            {
                mW = (T)1.0;
            }
        }

        /// <summary>Calculate a conjugate quaternion from this one.</summary>
        inline cQuaternion Conjugate() const
        {
            return cQuaternion( -mX, -mY, -mZ, mW, true );
        }

        /// <summary>When called on a quaternion object q, returns the inverse.</summary>
        inline cQuaternion Inverse() const
        {
            cQuaternion result( *this );

            result.Invert();

            return result;
        }

        /// <summary>Invert the quaternion in place.</summary>
        inline void Invert()
        {
            *this = Conjugate();
        }

        /// <summary>When called on a quaternion object q, returns a quaternion scaled by s.</summary>
        inline cQuaternion Scaled( T s ) const
        {
            // Don't attempt to scale a unit quaternion.
            return ( AutoNormalize ? *this : cQuaternion( mX * s, mY * s, mZ * s, mW * s ) );
        }

        /// <summary>Scales the quaternion in place.</summary>
        inline void Scale( T s )
        {
            // Don't attempt to scale a unit quaternion
            if( !AutoNormalize )
            {
                mX *= s;
                mY *= s;
                mZ *= s;
                mW *= s;
            }
        }

        /// <summary>Rotate the given vector through the rotation of this quaternion.</summary>
        inline cVector3<T> Rotate( const cVector3<T> &vec ) const
        {
            // this is a well-known optimisation
            cVector3<T> t = XYZ().Cross( vec );
            cVector3<T> u = t * W() + XYZ().Cross( t );

            // if 'this' is not pre-normalized, the result can be wrong. we need to scale by the norm.
            return ( vec + u * ( AutoNormalize ? T( 2 ) : ( T( 2 ) / Norm() ) ) );
        }

        /// <summary>Calculate the angle (in radians) between this quaternion and the one given</summary>
        T AngleTo( const cQuaternion& q ) const
        {
            T       innerProduct = Dot( q );
            T       val = innerProduct * innerProduct * 2 - 1;
            if( val > T( 1.0 ) )
            {
                val = T( 1.0 );
            }
            else if( val < T( -1.0 ) )
            {
                val = T( -1.0 );
            }
            return (T) acos( val );
        }

        /// <summary>
        /// Calculate a rough parametric distance between two quaternions. When the angle between the two
        /// quaternions is zero, the return value will be zero. When it is 180 degrees, the return value will be one.
        /// </summary>
        T ParametricDistance( const cQuaternion& q ) const
        {
            T innerProduct = Normalized().Dot( q.Normalized() );

            return ( 1 - innerProduct * innerProduct );
        }

        cVector3<T> XYZ() const
        {
            return cVector3<T>( mX, mY, mZ );
        }

        /// <summary>
        /// Returns a linear interpolated quaternion some percentage 't' between two quaternions.
        /// The parameter t is usually in the range [0,1], but this method can also be used to extrapolate
        /// rotations beyond that range.
        /// </summary>
        inline static cQuaternion Lerp( const cQuaternion& q1, const cQuaternion& q2, T t )
        {
            // In order for slerp to behave correctly, the input quaternions need to be unit length.
            // The caller needs to ensure that.

            T cosOmega = q1.Dot( q2 );
            T mult = 1;

            if( cosOmega < 0 )
            {
                // Input quaternions are on the opposite side of the hypersphere from each other.
                // Negate one to ensure they interpolate along the shortest path.
                mult = -1;
            }

            T k1 = 1 - t;
            T k2 = t;

            T x = k1 * q1.mX + k2 * q2.mX * mult;
            T y = k1 * q1.mY + k2 * q2.mY * mult;
            T z = k1 * q1.mZ + k2 * q2.mZ * mult;
            T w = k1 * q1.mW + k2 * q2.mW * mult;

            cQuaternion result( x, y, z, w, false );
            result.Normalize();

            return result;
        }
        inline cQuaternion Lerp( const cQuaternion& q, T t )
        {
            return Lerp( *this, q, t );
        }

        /// <summary>
        /// Returns a spherical linear interpolated quaternion some percentage 't' between two quaternions.
        /// The parameter t is usually in the range [0,1], but this method can also be used to extrapolate
        /// rotations beyond that range.
        /// </summary>
        inline static cQuaternion Slerp( const cQuaternion& q1, const cQuaternion& q2, T t )
        {
            // In order for slerp to behave correctly, the input quaternions need to be unit length.
            // The caller needs to ensure that.

            T cosOmega = q1.Dot( q2 );
            T mult = 1;

            if( cosOmega < 0 )
            {
                // Input quaternions are on the opposite side of the hypersphere from each other.
                // Negate one to ensure they interpolate along the shortest path.
                mult = -1;
                cosOmega = -cosOmega;
            }

            T k1;
            T k2;

            // Lerp and slerp will differ at machine precision level up to very near 1 for cosOmega.
            if( cosOmega > T( 0.999999 ) )
            {
                // Just use simple lerp to simplify calculations when the angle is very small, and to
                // avoid divide by zero in the slerp calcs.
                k1 = 1 - t;
                k2 = t;
            }
            else
            {
                T sinOmega = std::sqrt( 1 - cosOmega * cosOmega );
                T omega = atan2( sinOmega, cosOmega );
                T fDiv = 1 / omega; // We ensured omega would not be zero by checking if cosOmega is near 1
                k1 = sin( ( 1 - t ) * omega ) * fDiv;
                k2 = sin( t * omega ) * fDiv;
            }

            T x = k1 * q1.mX + k2 * q2.mX * mult;
            T y = k1 * q1.mY + k2 * q2.mY * mult;
            T z = k1 * q1.mZ + k2 * q2.mZ * mult;
            T w = k1 * q1.mW + k2 * q2.mW * mult;

            cQuaternion result( x, y, z, w, false );
            result.Normalize();

            return result;
        }
        inline cQuaternion Slerp( const cQuaternion& q, T t )
        {
            return Slerp( *this, q, t );
        }

        /// <summary>Set this quaternion to one that rotates from the world frame to the frame given by an X axis and Y direction.</summary>
        inline void FromAxes( const cVector3<T>& xAxisUnnormalized, const cVector3<T>& yDir )
        {
            cVector3<T> zAxis( xAxisUnnormalized.Cross( yDir ).Normalized() );
            cVector3<T> yAxis( zAxis.Cross( xAxisUnnormalized ).Normalized() );
            cVector3<T> xAxis( xAxisUnnormalized.Normalized() );
            T matrix[9];

            matrix[0] = xAxis.X();
            matrix[1] = xAxis.Y();
            matrix[2] = xAxis.Z();
            matrix[3] = yAxis.X();
            matrix[4] = yAxis.Y();
            matrix[5] = yAxis.Z();
            matrix[6] = zAxis.X();
            matrix[7] = zAxis.Y();
            matrix[8] = zAxis.Z();

            FromOrientationMatrix( matrix );
        }

        /// <summary>Create a quaternion from 2 vectors.</summary>
        inline static cQuaternion FromVectors( const cVector3<T>& from, const cVector3<T>& to )
        {
            cQuaternion q;
            q.VectorToVector( from, to );
            return q;
        }

        /// <summary>Create quaternion that rotates a vector1 to vector2</summary>
        inline void VectorToVector( const cVector3<T>& vec1, const cVector3<T>& vec2 )
        {
            cVector3<T> v1( vec1 ), v2( vec2 ), vHalf, vCross;

            v1.Normalize();
            v2.Normalize();
            vHalf = v1 + v2;

            if( vHalf.LengthSquared() < 0.00001 )
            {
                // v1 & v2 are pointing opposite from each other, use any orthogonal vector to v1
                T x = std::abs( v1.X() );
                T y = std::abs( v1.Y() );
                T z = std::abs( v1.Z() );

                if( x > y && x > z )
                    vHalf.SetValues( v1.Y(), -v1.X(), v1.Z() );
                else if( y > z && y > x )
                    vHalf.SetValues( v1.X(), v1.Z(), -v1.Y() );
                else
                    vHalf.SetValues( -v1.Z(), v1.Y(), v1.X() );
            }

            vHalf.Normalize();
            vCross = vHalf.Cross( v2 );

            mW = vHalf.Dot( v2 );
            mX = vCross.X();
            mY = vCross.Y();
            mZ = vCross.Z();

            Normalize();

        }

        /// <summary>Set Axis Angle. Angle is encoded in vector's length</summary>
        void SetAxisAngle( const cVector3<T>& aa )
        {
            T radians = aa.Length();

            if( abs( radians ) > std::numeric_limits<T>::epsilon() )
            {
                mW = cos( radians / T( 2.0 ) );

                // Divide the length out.
                cVector3<T> v = aa / radians * sin( radians / T( 2.0 ) );

                mX = v.X();
                mY = v.Y();
                mZ = v.Z();
            }
            else
            {
                mX = mY = mZ = T( 0.0 );
                mW = T( 1.0 );
            }
        }

        /// <summary>Set Axis Angle by specifying separate axis and angle. Axis may be un-normalized.</summary>
        void SetAxisAngle( const cVector3<T>& axis, T angle )
        {
            if( abs( angle ) > std::numeric_limits<T>::epsilon() )
            {
                cVector3<T> v( axis.Normalized() );

                mW = cos( angle / T( 2.0 ) );

                v *= sin( angle / T( 2.0 ) );

                mX = v.X();
                mY = v.Y();
                mZ = v.Z();
            }
            else
            {
                mX = mY = mZ = T( 0.0 );
                mW = T( 1.0 );
            }
        }

        /// <summary>Get Axis Angle. Angle is encoded in vector's length</summary>
        inline cVector3<T> AxisAngle() const
        {
            T   x = mX, y = mY, z = mZ, w = mW;

            if( mW < 0 )
            {
                x = -x;
                y = -y;
                z = -z;
                w = -w;
            }

            cVector3<T> v( x, y, z );
            T radians = T( 2.0 ) * acos( w );

            v.Normalize();
            v *= radians;

            return v;
        }

        //====================================================================================
        // Type conversion helpers
        //====================================================================================

        template<typename U>
        Core::cQuaternion<U, AutoNormalize> ConvertToType() const
        {
            return Core::cQuaternion<U, AutoNormalize>( (U) X(), (U) Y(), (U) Z(), (U) W() );
        }

        //== everything below this line is subject to coordinate system handedness and is work in progress ==--

        /// <summary>Initialize quaternion to a rotation in X.</summary>
        template<typename U>
        inline void SetRotationX( U angle )
        {
            T f = (T) ( angle / 2.0 );
            T s = sin( f );
            mW = cos( f );
            mX = s;
            mY = 0;
            mZ = 0;
        }

        /// <summary>Initialize quaternion to a rotation in Y.</summary>
        template<typename U>
        inline void SetRotationY( U angle )
        {
            T f = (T) ( angle / 2.0 );
            T s = sin( f );
            mW = cos( f );
            mX = 0;
            mY = s;
            mZ = 0;
        }

        /// <summary>Initialize quaternion to a rotation in Z.</summary>
        template<typename U>
        inline void SetRotationZ( U angle )
        {
            T f = (T) ( angle / 2.0 );
            T s = sin( f );
            mW = cos( f );
            mX = 0;
            mY = 0;
            mZ = (T) s;
        }

        /// <summary>Create quaternion orientation from a 3x3 row-major rotation matrix array.</summary>
        template<typename U>
        void FromOrientationMatrix( const U* m )
        {
            cQuaternion& q = *this;

            T trace = (T) ( m[0] + m[4] + m[8] );
            if( trace > 0 )
            {
                T s = ( (T) 0.5 ) / std::sqrt( trace + 1 );

                q.mW = (T) 0.25 / s;
                q.mX = (T) ( m[7] - m[5] ) * s;
                q.mY = (T) ( m[2] - m[6] ) * s;
                q.mZ = (T) ( m[3] - m[1] ) * s;
            }
            else
            {
                if( m[0] > m[4] && m[0] > m[8] )
                {
                    T s = (T) ( 2.0 * std::sqrt( 1.0 + m[0] - m[4] - m[8] ) );
                    q.mW = (T) ( m[7] - m[5] ) / s;
                    q.mX = (T) 0.25 * s;
                    q.mY = (T) ( m[1] + m[3] ) / s;
                    q.mZ = (T) ( m[2] + m[6] ) / s;
                }
                else
                {
                    if( m[4] > m[8] )
                    {
                        T s = (T) ( 2.0 * std::sqrt( 1.0 + m[4] - m[0] - m[8] ) );
                        q.mW = (T) ( m[2] - m[6] ) / s;
                        q.mX = (T) ( m[1] + m[3] ) / s;
                        q.mY = (T) 0.25 * s;
                        q.mZ = (T) ( m[5] + m[7] ) / s;
                    }
                    else
                    {
                        T s = (T) ( 2.0 * std::sqrt( 1.0 + m[8] - m[0] - m[4] ) );
                        q.mW = (T) ( m[3] - m[1] ) / s;
                        q.mX = (T) ( m[2] + m[6] ) / s;
                        q.mY = (T) ( m[5] + m[7] ) / s;
                        q.mZ = (T) 0.25 * s;
                    }
                }
            }
        }

        /// <summary>Output quaternion to a 3x3 row-major rotation matrix array.</summary>
        template<typename U>
        void ToOrientationMatrix( U* m ) const
        {
            const cQuaternion<T, AutoNormalize>& q = *this;

            T sqw = (T) ( q.mW * q.mW );
            T sqx = (T) ( q.mX * q.mX );
            T sqy = (T) ( q.mY * q.mY );
            T sqz = (T) ( q.mZ * q.mZ );

            T invs = 1 / ( sqx + sqy + sqz + sqw );

            m[0] = ( ( sqx - sqy - sqz + sqw ) * invs );
            m[4] = ( ( -sqx + sqy - sqz + sqw ) * invs );
            m[8] = ( ( -sqx - sqy + sqz + sqw ) * invs );

            T tmp1 = (T) ( q.mX * q.mY );
            T tmp2 = (T) ( q.mZ * q.mW );

            m[3] = ( 2 * ( tmp1 + tmp2 ) * invs );
            m[1] = ( 2 * ( tmp1 - tmp2 ) * invs );

            tmp1 = (T) ( q.mX * q.mZ );
            tmp2 = (T) ( q.mY * q.mW );
            m[6] = ( 2 * ( tmp1 - tmp2 ) * invs );
            m[2] = ( 2 * ( tmp1 + tmp2 ) * invs );
            tmp1 = (T) ( q.mY * q.mZ );
            tmp2 = (T) ( q.mX * q.mW );
            m[7] = ( 2 * ( tmp1 + tmp2 ) * invs );
            m[5] = ( 2 * ( tmp1 - tmp2 ) * invs );
        }

        /// <summary>Create quaternion orientation from a matrix.</summary>
        void FromMatrix( const cMatrix4<T>& m )
        {
            T rot[9];
            rot[0] = m.Value( 0, 0 );
            rot[1] = m.Value( 1, 0 );
            rot[2] = m.Value( 2, 0 );
            rot[3] = m.Value( 0, 1 );
            rot[4] = m.Value( 1, 1 );
            rot[5] = m.Value( 2, 1 );
            rot[6] = m.Value( 0, 2 );
            rot[7] = m.Value( 1, 2 );
            rot[8] = m.Value( 2, 2 );

            FromOrientationMatrix( rot );
        }

        /// <summary>Create 4x4 transform matrix from quaternion.</summary>
        cMatrix4<T> ToMatrix() const
        {
            cMatrix4<T> m( cMatrix4<T>::kIdentity );
            T orientMatrix[9];

            ToOrientationMatrix( orientMatrix );

            m.SetValue( 0, 0, orientMatrix[0] );
            m.SetValue( 1, 0, orientMatrix[1] );
            m.SetValue( 2, 0, orientMatrix[2] );
            m.SetValue( 0, 1, orientMatrix[3] );
            m.SetValue( 1, 1, orientMatrix[4] );
            m.SetValue( 2, 1, orientMatrix[5] );
            m.SetValue( 0, 2, orientMatrix[6] );
            m.SetValue( 1, 2, orientMatrix[7] );
            m.SetValue( 2, 2, orientMatrix[8] );

            return m;
        }

        /// <summary>Returns an identity quaternion. This typically means 'no rotation'.</summary>
        static const cQuaternion kIdentity;

    private:
        T mX;
        T mY;
        T mZ;
        T mW;

        // An alternative constructor that does not pay attention to AutoNormalize
        cQuaternion( T x, T y, T z, T w, bool junk )
        {
            mX = x;
            mY = y;
            mZ = z;
            mW = w;
        }

        // Internal scaling method that does not respect AutoNormalize
        inline void InternalScale( T s )
        {
            mX *= s;
            mY *= s;
            mZ *= s;
            mW *= s;
        }
    };

    template <typename T, bool AutoNormalize>
    const cQuaternion<T, AutoNormalize> cQuaternion<T, AutoNormalize>::kIdentity( 0, 0, 0, 1 );

    //=========================================================================
    // Stream I/O operators
    //=========================================================================
    template<typename T, bool AutoNormalize>
    std::wostream& operator<<( std::wostream& os, const cQuaternion<T, AutoNormalize>& q )
    {
        os << L"(" << q.X() << L"," << q.Y() << L"," << q.Z() << L"," << q.W() << L")";

        return os;
    }

    template<typename T, bool AutoNormalize>
    std::ostream& operator<<( std::ostream& os, const cQuaternion<T, AutoNormalize>& q )
    {
        T x = q.X();
        T y = q.Y();
        T z = q.Z();
        T w = q.W();
        os.write( (char*) &x, sizeof( T ) );
        os.write( (char*) &y, sizeof( T ) );
        os.write( (char*) &z, sizeof( T ) );
        os.write( (char*) &w, sizeof( T ) );
        return os;
    }

    template<typename T, bool AutoNormalize>
    std::istream& operator>>( std::istream& is, cQuaternion<T, AutoNormalize>& v )
    {
        T x, y, z, w;
        is.read( (char*) &x, sizeof( T ) );
        is.read( (char*) &y, sizeof( T ) );
        is.read( (char*) &z, sizeof( T ) );
        is.read( (char*) &w, sizeof( T ) );
        v.SetValues( x, y, z, w );
        return is;
    }

    //== class short-hand definitions ==--

    using cQuaternionf = cQuaternion<float, false>;
    using cQuaterniond = cQuaternion<double, false>;
    using cRotationf = cQuaternion<float, true>;
    using cRotationd = cQuaternion<double, true>;
}

