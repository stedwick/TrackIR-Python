//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================

// Local includes
#include "Core/Vector3.h"

#if !defined(__PLATFORM__LINUX__)

namespace Core
{
    template<typename T>
    inline cMatrix4<T>::cMatrix4() 
    { 
        // No initialization 
    }

    template<typename T>
    inline cMatrix4<T>::cMatrix4( const cMatrix4& other )
    {
        ::memcpy( mVals, other.Data(), 16 * sizeof( T ) );
    }

    template<typename T>
    inline cMatrix4<T>::cMatrix4( T t11, T t12, T t13, T t14, T t21, T t22, T t23, T t24, T t31, T t32, T t33, T t34, T t41, T t42, T t43, T t44 )
    {
        mVals[0]  = t11;
        mVals[1]  = t12;
        mVals[2]  = t13;
        mVals[3]  = t14;
        mVals[4]  = t21;
        mVals[5]  = t22;
        mVals[6]  = t23;
        mVals[7]  = t24;
        mVals[8]  = t31;
        mVals[9]  = t32;
        mVals[10] = t33;
        mVals[11] = t34;
        mVals[12] = t41;
        mVals[13] = t42;
        mVals[14] = t43;
        mVals[15] = t44;
    }

    template<typename T>
    inline cMatrix4<T>::cMatrix4( const T vals[16] )
    {
        ::memcpy( mVals, vals, 16 * sizeof( T ) );
    }

    template<typename T>
    inline cMatrix4<T>::cMatrix4( const cVector3<T>& translation )
    {
        Translate( translation );
    }

    template<typename T>
    inline cMatrix4<T>::cMatrix4( const cQuaternion<T, true>& rotation, const cVector3<T>& translation )
    {
        RotateTranslate( rotation, translation );
    }

    template<typename T>
    inline void cMatrix4<T>::SetValues( T t11, T t12, T t13, T t14, T t21, T t22, T t23, T t24, T t31, T t32, T t33, T t34, T t41, T t42, T t43, T t44 )
    {
        mVals[0]  = t11;
        mVals[1]  = t12;
        mVals[2]  = t13;
        mVals[3]  = t14;
        mVals[4]  = t21;
        mVals[5]  = t22;
        mVals[6]  = t23;
        mVals[7]  = t24;
        mVals[8]  = t31;
        mVals[9]  = t32;
        mVals[10] = t33;
        mVals[11] = t34;
        mVals[12] = t41;
        mVals[13] = t42;
        mVals[14] = t43;
        mVals[15] = t44;
    }

    template<typename T>
    inline void cMatrix4<T>::SetValues( const T vals[16] ) 
    { 
        memcpy( mVals, vals, 16 * sizeof( T ) ); 
    }

    template<typename T>
    inline void cMatrix4<T>::SetValue( int row, int col, T val )
    {
        mVals[row * 4 + col] = val; 
    }

    template<typename T>
    inline T cMatrix4<T>::Value( int row, int col ) const 
    { 
        return mVals[row * 4 + col]; 
    }

    template<typename T>
    inline T& cMatrix4<T>::Value( int row, int col ) 
    { 
        return mVals[row * 4 + col]; 
    }

    template<typename T>
    inline void cMatrix4<T>::CopyValues( const cMatrix4<T> &src, int start, int count )
    {
        for( int i = 0; i < count; i++ )
            mVals[start + i] = src.mVals[start + i];
    }

    template<typename T>
    inline const T* cMatrix4<T>::Data() const 
    { 
        return mVals; 
    }

    template<typename T>
    inline void cMatrix4<T>::Translate( T x, T y, T z )
    {
        *this = kIdentity;

        mVals[12] = x;
        mVals[13] = y;
        mVals[14] = z;
    }

    template<typename T>
    inline void cMatrix4<T>::Translate( const cVector3<T>& v )
    {
        Translate( v.X(), v.Y(), v.Z() );
    }

    template<typename T>
    inline cVector3<T> cMatrix4<T>::Translation() const
    {
        return cVector3<T>( mVals[12], mVals[13], mVals[14] );
    }

    template<typename T>
    inline cQuaternion<T,true> cMatrix4<T>::Rotation() const
    {
        // We assume that the matrix is composed purely of affine transforms.
        cQuaternion<T, true> q;
        T m[9];

        m[0] = mVals[0];
        m[1] = mVals[4];
        m[2] = mVals[8];
        m[3] = mVals[1];
        m[4] = mVals[5];
        m[5] = mVals[9];
        m[6] = mVals[2];
        m[7] = mVals[6];
        m[8] = mVals[10];

        q.FromOrientationMatrix( m );

        return q;
    }

    template<typename T>
    inline void cMatrix4<T>::SetRotateX( T angle )
    {
        *this = kIdentity;

        T c = cos( angle );
        T s = sin( angle );

        mVals[5]  = c;
        mVals[10] = c;
        mVals[6]  = s;
        mVals[9]  = -s;
    }

    template<typename T>
    inline void cMatrix4<T>::SetRotateY( T angle )
    {
        *this = kIdentity;

        T c = cos( angle );
        T s = sin( angle );

        mVals[0]  = c;
        mVals[10] = c;
        mVals[2]  = -s;
        mVals[8]  = s;
    }

    template<typename T>
    inline void cMatrix4<T>::SetRotateZ( T angle )
    {
        *this = kIdentity;

        T c = cos( angle );
        T s = sin( angle );

        mVals[0]  = c;
        mVals[5]  = c;
        mVals[1]  = s;
        mVals[4]  = -s;
    }

    template<typename T>
    inline void cMatrix4<T>::SetRotation( T x, T y, T z, int rotationOrder )
    {
        cMatrix4 xMat, yMat, zMat;
        if( x != 0 )
            xMat.SetRotateX( x );
        else
            xMat = kIdentity;
        if( y != 0 )
            yMat.SetRotateY( y );
        else
            yMat = kIdentity;
        if( z != 0 )
            zMat.SetRotateZ( z );
        else
            zMat = kIdentity;

        switch( rotationOrder )
        {
        case EulOrdXYZr:
            *this = xMat * yMat * zMat;
            break;
        case EulOrdXZYr:
            *this = xMat * zMat * yMat;
            break;
        case EulOrdYXZr:
            *this = yMat * xMat * zMat;
            break;
        case EulOrdYZXr:
            *this = yMat * zMat * xMat;
            break;
        case EulOrdZXYr:
            *this = zMat * xMat * yMat;
            break;
        case EulOrdZYXr:
            *this = zMat * yMat * xMat;
            break;
        default:
            break;
        }
    }

    template<typename T>
    inline void cMatrix4<T>::SetRotation( const cQuaternion<T, true> &q )
    {
        *this = kIdentity;

        T m[9];
        q.ToOrientationMatrix( m );

        mVals[0] = m[0];
        mVals[4] = m[1];
        mVals[8] = m[2];
        mVals[1] = m[3];
        mVals[5] = m[4];
        mVals[9] = m[5];
        mVals[2] = m[6];
        mVals[6] = m[7];
        mVals[10] = m[8];
    }

    template<typename T>
    inline void cMatrix4<T>::SetScale( T x, T y, T z )
    {
        *this = kIdentity;

        mVals[0]  = x;
        mVals[5]  = y;
        mVals[10] = z;
    }

    template<typename T>
    inline void cMatrix4<T>::SetScale( const cVector3<T>& v )
    {
        SetScale( v.X(), v.Y(), v.Z() );
    }

    template<typename T>
    inline cVector3<T> cMatrix4<T>::Scale() const
    {
        // This assumes the matrix is composed only of affine transforms.
        return cVector3<T>( mVals[0], mVals[5], mVals[10] );
    }

    template<typename T>
    inline void cMatrix4<T>::RotateTranslate( const cQuaternion<T, true>& q, const cVector3<T>& v )
    {
        SetRotation( q );

        mVals[12] = v.X();
        mVals[13] = v.Y();
        mVals[14] = v.Z();
    }

    template<typename T>
    inline void cMatrix4<T>::ScaleRotateTranslate( const cVector3<T>& s, const cQuaternion<T, true>& q, const cVector3<T>& v )
    {
        RotateTranslate( q, v );

        cMatrix4 mS;
        mS.SetScale( s );
        SetToProduct( mS, *this );
    }

    template<typename T>
    inline T cMatrix4<T>::Determinant() const
    {
        return  mVals[0] * ( mVals[5] * ( mVals[10] * mVals[15] - mVals[11] * mVals[14] ) - mVals[9] * ( mVals[6] * mVals[15] + mVals[7] * mVals[14] ) + mVals[13] * ( mVals[6] * mVals[11] - mVals[7] * mVals[10] ) ) -
                mVals[4] * ( mVals[1] * ( mVals[10] * mVals[15] - mVals[11] * mVals[14] ) - mVals[9] * ( mVals[2] * mVals[15] + mVals[3] * mVals[14] ) + mVals[13] * ( mVals[2] * mVals[11] - mVals[3] * mVals[10] ) ) +
                mVals[8] * ( mVals[1] * (  mVals[6] * mVals[15] -  mVals[7] * mVals[14] ) - mVals[5] * ( mVals[2] * mVals[15] + mVals[3] * mVals[14] ) + mVals[13] * ( mVals[2] *  mVals[7] - mVals[3] *  mVals[6] ) ) -
                mVals[12] * ( mVals[1] * (  mVals[6] * mVals[11] -  mVals[7] * mVals[10] ) - mVals[5] * ( mVals[2] * mVals[11] + mVals[3] * mVals[10] ) +  mVals[9] * ( mVals[2] *  mVals[7] - mVals[3] *  mVals[6] ) );
    }

    template<typename T>
    inline void cMatrix4<T>::Invert()
    {
        SetToInverse( *this );
    }

    template<typename T>
    inline cMatrix4<T> cMatrix4<T>::Inverse() const
    {
        cMatrix4 m;
        m.SetToInverse( *this );
        return m;
    }

    template<typename T>
    inline void cMatrix4<T>::SetToInverse( const cMatrix4<T> m )
    {
        T d = ((T)1.0) / m.Determinant();

        mVals[0]  = ( m.Value( 1, 1 ) * m.Value( 2, 2 ) - m.Value( 1, 2 ) * m.Value( 2, 1 ) ) * d;
        mVals[1]  = ( m.Value( 0, 2 ) * m.Value( 2, 1 ) - m.Value( 0, 1 ) * m.Value( 2, 2 ) ) * d;
        mVals[2]  = ( m.Value( 0, 1 ) * m.Value( 1, 2 ) - m.Value( 0, 2 ) * m.Value( 1, 1 ) ) * d;
        mVals[3]  = (T)0.0;

        mVals[4]  = ( m.Value( 1, 2 ) * m.Value( 2, 0 ) - m.Value( 1, 0 ) * m.Value( 2, 2 ) ) * d;
        mVals[5]  = ( m.Value( 0, 0 ) * m.Value( 2, 2 ) - m.Value( 0, 2 ) * m.Value( 2, 0 ) ) * d;
        mVals[6]  = ( m.Value( 0, 2 ) * m.Value( 1, 0 ) - m.Value( 0, 0 ) * m.Value( 1, 2 ) ) * d;
        mVals[7]  = (T)0.0;

        mVals[8]  = ( m.Value( 1, 0 ) * m.Value( 2, 1 ) - m.Value( 1, 1 ) * m.Value( 2, 0 ) ) * d;
        mVals[9]  = ( m.Value( 0, 1 ) * m.Value( 2, 0 ) - m.Value( 0, 0 ) * m.Value( 2, 1 ) ) * d;
        mVals[10] = ( m.Value( 0, 0 ) * m.Value( 1, 1 ) - m.Value( 0, 1 ) * m.Value( 1, 0 ) ) * d;
        mVals[11] = (T)0.0;

        mVals[12] = -( m.Value( 3, 0 ) * mVals[0] + m.Value( 3, 1 ) * mVals[4] + m.Value( 3, 2 ) * mVals[8] );
        mVals[13] = -( m.Value( 3, 0 ) * mVals[1] + m.Value( 3, 1 ) * mVals[5] + m.Value( 3, 2 ) * mVals[9] );
        mVals[14] = -( m.Value( 3, 0 ) * mVals[2] + m.Value( 3, 1 ) * mVals[6] + m.Value( 3, 2 ) * mVals[10] );
        mVals[15] = (T)1.0;
    }

    template<typename T>
    inline cMatrix4<T> cMatrix4<T>::Transposed()
    {
        cMatrix4<T> ret;
        ret.SetToTranspose( *this );
        return ret;
    }

    template<typename T>
    inline void cMatrix4<T>::SetToTranspose( const cMatrix4<T> m )
    {
        mVals[0]  = m.Value( 0, 0 );
        mVals[1]  = m.Value( 1, 0 );
        mVals[2]  = m.Value( 2, 0 );
        mVals[3]  = m.Value( 3, 0 );
        mVals[4]  = m.Value( 0, 1 );
        mVals[5]  = m.Value( 1, 1 );
        mVals[6]  = m.Value( 2, 1 );
        mVals[7]  = m.Value( 3, 1 );
        mVals[8]  = m.Value( 0, 2 );
        mVals[9]  = m.Value( 1, 2 );
        mVals[10] = m.Value( 2, 2 );
        mVals[11] = m.Value( 3, 2 );
        mVals[12] = m.Value( 0, 3 );
        mVals[13] = m.Value( 1, 3 );
        mVals[14] = m.Value( 2, 3 );
        mVals[15] = m.Value( 3, 3 );
    }

    template<typename T>
    inline void cMatrix4<T>::SetToProduct( const cMatrix4<T> m1, const cMatrix4<T> m2 )
    {
        const T *v1 = m1.Data(), *v2 = m2.Data();

        for( int row = 0; row < 4; row++ )
        {
            for( int col = 0; col < 4; col++ )
            {
                mVals[row * 4 + col] = v1[row * 4] * v2[col] + v1[row * 4 + 1] * v2[col + 4] + v1[row * 4 + 2] * v2[col + 8] + v1[row * 4 + 3] * v2[col + 12];
            }
        }
    }

    template<typename T>
    inline void cMatrix4<T>::PerspectiveFovRH( float fovY, float aspect, float nearClipPlane, float farClipPlane )
    {
        /*
        xScale     0          0              0
        0        yScale       0              0
        0        0        zf/(zn-zf)        -1
        0        0        zn*zf/(zn-zf)      0
        where:
        yScale = cot(fovY/2)
        xScale = yScale / aspect ratio
        */
        float yScale = 1 / tan( fovY / 2 );
        float xScale = yScale / aspect;

        mVals[0]  = xScale;
        mVals[1]  = 0;
        mVals[2]  = 0;
        mVals[3]  = 0;
        mVals[4]  = 0;
        mVals[5]  = yScale;
        mVals[6]  = 0;
        mVals[7]  = 0;
        mVals[8]  = 0;
        mVals[9]  = 0;
        mVals[10]  = farClipPlane / ( nearClipPlane - farClipPlane );
        mVals[11] = -1;
        mVals[12] = 0;
        mVals[13] = 0;
        mVals[14] = nearClipPlane * farClipPlane / ( nearClipPlane - farClipPlane );
        mVals[15] = 0;
    }
    
    template<typename T>
    inline void cMatrix4<T>::PerspectiveOffCenterRH( float left, float right, float bottom, float top, float nearClipPlane, float farClipPlane )
    {
        /*
          2*zn/(r-l)   0            0                0
          0            2*zn/(t-b)   0                0
          (l+r)/(r-l)  (t+b)/(t-b)  zf/(zn-zf)      -1
          0            0            zn*zf/(zn-zf)    0
        */

        mVals[0] = 2 * nearClipPlane / ( right - left );
        mVals[1] = 0;
        mVals[2] = 0;
        mVals[3] = 0;
        mVals[4] = 0;
        mVals[5] = 2 * nearClipPlane / ( top - bottom );
        mVals[6] = 0;
        mVals[7] = 0;
        mVals[8] = ( left + right ) / ( right - left );
        mVals[9] = ( top + bottom ) / ( top - bottom );
        mVals[10] = farClipPlane / ( nearClipPlane - farClipPlane );
        mVals[11] = -1;
        mVals[12] = 0;
        mVals[13] = 0;
        mVals[14] = nearClipPlane * farClipPlane / ( nearClipPlane - farClipPlane );
        mVals[15] = 0;
    }

    template<typename T>
    inline void cMatrix4<T>::LookAtRH( const cVector3<T> &eye, const cVector3<T> &lookAt, const cVector3<T> &up )
    {
        /*
        zaxis = normal(Eye - At)
        xaxis = normal(cross(Up, zaxis))
        yaxis = cross(zaxis, xaxis)

        xaxis.x           yaxis.x           zaxis.x          0
        xaxis.y           yaxis.y           zaxis.y          0
        xaxis.z           yaxis.z           zaxis.z          0
        dot(xaxis, eye)   dot(yaxis, eye)   dot(zaxis, eye)  1
        */

        cVector3<T> zAxis( eye - lookAt );
        zAxis.Normalize();

        cVector3<T> xAxis;
        xAxis = up.Cross( zAxis );
        xAxis.Normalize();

        cVector3<T> yAxis;
        yAxis = zAxis.Cross( xAxis );
        yAxis.Normalize();

        mVals[0]  = xAxis.X();
        mVals[1]  = yAxis.X();
        mVals[2]  = zAxis.X();
        mVals[3]  = 0;
        mVals[4]  = xAxis.Y();
        mVals[5]  = yAxis.Y();
        mVals[6]  = zAxis.Y();
        mVals[7]  = 0;
        mVals[8]  = xAxis.Z();
        mVals[9]  = yAxis.Z();
        mVals[10] = zAxis.Z();
        mVals[11] = 0;
        mVals[12] = xAxis.Dot( eye );
        mVals[13] = yAxis.Dot( eye );
        mVals[14] = zAxis.Dot( eye );
        mVals[15] = 1;
    }

    template<typename T>
    inline void cMatrix4<T>::OrthoRH( float w, float h, float nearClipPlane, float farClipPlane )
    {
        /*
        2/w  0    0           0
        0    2/h  0           0
        0    0    1/(zn-zf)   0
        0    0    zn/(zn-zf)  1
        */

        mVals[0]  = 2 / w;
        mVals[1]  = 0;
        mVals[2]  = 0;
        mVals[3]  = 0;
        mVals[4]  = 0;
        mVals[5]  = 2 / h;
        mVals[6]  = 0;
        mVals[7]  = 0;
        mVals[8]  = 0;
        mVals[9]  = 0;
        mVals[10] = 1 / ( nearClipPlane - farClipPlane );
        mVals[11] = 0;
        mVals[12] = 0;
        mVals[13] = 0;
        mVals[14] = nearClipPlane / ( nearClipPlane - farClipPlane );
        mVals[15] = 1;
    }

    template<typename T>
    inline void cMatrix4<T>::OrthoOffCenterRH( float left, float right, float bottom, float top, float nearClipPlane, float farClipPlane )
    {
        /*
        2/(r-l)      0            0           0
        0            2/(t-b)      0           0
        0            0            1/(zn-zf)   0
        (l+r)/(l-r)  (t+b)/(b-t)  zn/(zn-zf)  1
        */

        mVals[0]  = 2 / ( right - left );
        mVals[1]  = 0;
        mVals[2]  = 0;
        mVals[3]  = 0;
        mVals[4]  = 0;
        mVals[5]  = 2 / ( top - bottom );
        mVals[6]  = 0;
        mVals[7]  = 0;
        mVals[8]  = 0;
        mVals[9]  = 0;
        mVals[10] = 1 / ( nearClipPlane - farClipPlane );
        mVals[11] = 0;
        mVals[12] = ( left + right ) / ( left - right );
        mVals[13] = ( top + bottom ) / ( bottom - top );
        mVals[14] = nearClipPlane / ( nearClipPlane - farClipPlane );
        mVals[15] = 1;
    }

    //====================================================================================
    // Operators
    //====================================================================================

    template<typename T>
    inline bool cMatrix4<T>::operator==( const cMatrix4<T>& rhs ) const
    {
        return ( ::memcmp( mVals, rhs.mVals, 16 * sizeof( T ) ) == 0 );
    }

    template<typename T>
    inline bool cMatrix4<T>::operator!=( const cMatrix4<T>& rhs ) const 
    { 
        return !( *this == rhs ); 
    }

    template<typename T>
    inline cMatrix4<T>& cMatrix4<T>::operator*=( const cMatrix4<T>& rhs )
    {
        SetToProduct( *this, rhs );
        return *this;
    }

    template<typename T>
    inline cMatrix4<T> &cMatrix4<T>::operator+=( const cMatrix4<T> &rhs )
    {
        const T *v2 = rhs.Data();
        for( int i = 0; i < 16; ++i ) mVals[i] += v2[i];
        return *this;
    }

    template<typename T>
    inline cMatrix4<T> &cMatrix4<T>::operator-=( const cMatrix4<T> &rhs )
    {
        const T *v2 = rhs.Data();
        for( int i = 0; i < 16; ++i ) mVals[i] -= v2[i];
        return *this;
    }

    template<typename T>
    inline cVector4<T> cMatrix4<T>::operator*( const cVector4<T> &rhs ) const
    {
        cVector4<T> ret;
        const T *v1 = Data();
        const T x0( rhs[0] ), x1( rhs[1] ), x2( rhs[2] ), x3( rhs[3] );
        return cVector4<T>(
            v1[0] * x0 + v1[1] * x1 + v1[2] * x2 + v1[3] * x3,
            v1[4] * x0 + v1[5] * x1 + v1[6] * x2 + v1[7] * x3,
            v1[8] * x0 + v1[9] * x1 + v1[10] * x2 + v1[11] * x3,
            v1[12] * x0 + v1[13] * x1 + v1[14] * x2 + v1[15] * x3 );
    }

}

#endif // __PLATFORM__LINUX__

