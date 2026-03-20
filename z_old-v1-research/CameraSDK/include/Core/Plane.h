//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once

// System includes
#include <cstring> //== for memcpy
#include <iosfwd>

// Local includes
#include "Platform.h"
#include "Vector3.h"

#if !defined(__PLATFORM__LINUX__)
namespace Core
{
    template<typename T>
    class cPlane
    {
    public:
        cPlane() = default; // No initialization

        template <typename U>
        cPlane( const cPlane<U>& other )
        {
            mVals[0] = T( other.A() );
            mVals[1] = T( other.B() );
            mVals[2] = T( other.C() );
            mVals[3] = T( other.D() );
        }

        cPlane( T a, T b, T c, T d )
        {
            mVals[0] = a;
            mVals[1] = b;
            mVals[2] = c;
            mVals[3] = d;
        }
        
        template <typename U>
        cPlane( U a, U b, U c, U d )
        {
            mVals[0] = T( a );
            mVals[1] = T( b );
            mVals[2] = T( c );
            mVals[3] = T( d );
        }

        cPlane( const T vals[4] )
        {
            ::memcpy( mVals, vals, 4 * sizeof( T ) );
        }

        void FromPointNormal( const cVector3<T>& point, const cVector3<T>& normal, bool normalize )
        {
            cVector3<T> n( normal );
            if( normalize )
                n.Normalize();

            mVals[0] = n.X();
            mVals[1] = n.Y();
            mVals[2] = n.Z();
            mVals[3] = -n.Dot( point );
        }

        void FromThreePoints( const cVector3<T>& pt1, const cVector3<T>& pt2, const cVector3<T>& pt3 )
        {
            cVector3<T> cr( ( pt2 - pt1 ).Cross( pt3 - pt1 ) ), p;
            cr.Normalize();

            //cVectorUtilities<T>::ProjectPointOnRay( p, pt1, cVector3<T>::kZero, cr );

            T t = pt1.Dot( cr );
            p = cr * t;

            FromPointNormal( p, cr, false );
        }

        /// <summary>Set the vector values.</summary>
        void SetValues( T a, T b, T c, T d ) 
        { 
            mVals[0] = a; 
            mVals[1] = b; 
            mVals[2] = c; 
            mVals[3] = d; 
        }

        template <typename U>
        void SetValues( U a, U b, U c, U d ) 
        { 
            mVals[0] = T( a ); 
            mVals[1] = T( b ); 
            mVals[2] = T( c ); 
            mVals[3] = T( d ); 
        }
        
        void SetValues( const T vals[4] ) 
        { 
            memcpy( mVals, vals, 4 * sizeof( T ) ); 
        }

        T A() const { return mVals[0]; }
        T& A() { return mVals[0]; }
        T B() const { return mVals[1]; }
        T& B() { return mVals[1]; }
        T C() const { return mVals[2]; }
        T& C() { return mVals[2]; }
        T D() const { return mVals[3]; }
        T& D() { return mVals[3]; }

        T operator[]( int idx ) const { return mVals[idx]; }
        T& operator[]( int idx ) { return mVals[idx]; }

        /// <summary>Access to the data array.</summary>
        const float* Data() const { return mVals; }

        /// <summary>Normalize the vector (in place) to unit length.</summary>
        void Normalize()
        {
            T   len = sqrt( mVals[0] * mVals[0] + mVals[1] * mVals[1] + mVals[2] * mVals[2] );

            if( len > 0 )
            {
                T   scale = 1 / len;
                mVals[0] *= scale;
                mVals[1] *= scale;
                mVals[2] *= scale;
                mVals[3] *= scale;
            }
        }

        /// <summary>Returns the distance from plane to the point. Negative if the point is on the other side of plane.</summary>
        T DistanceTo( const cVector3<T>& point ) const
        {
            T dx = point.X() + A() * D();
            T dy = point.Y() + B() * D();
            T dz = point.Z() + C() * D();

            return A() * dx + B() * dy + C() * dz;
        }

        /// <summary>Returns the position projected onto the plane.</summary>
        cVector3<T> ProjectedPosition( const cVector3<T>& point ) const
        {
            T distance = DistanceTo( point );
            return point - cVector3<T>( A(), B(), C() ) * distance;
        }

        //====================================================================================
        // Assignment operators
        //====================================================================================

        template <typename U>
        cPlane& operator=( const cPlane<U>& other )
        {
            mVals[0] = T( other.A() );
            mVals[1] = T( other.B() );
            mVals[2] = T( other.C() );
            mVals[3] = T( other.D() );

            return *this;
        }

        //====================================================================================
        // Type conversion helpers
        //====================================================================================

        inline Core::cPlane<float> ToFloat() const
        {
            return Core::cPlane<float>( (float) A(), (float) B(), (float) C(), (float) D() );
        }

        inline Core::cPlane<double> ToDouble() const
        {
            return Core::cPlane<double>( (double) A(), (double) B(), (double) C(), (double) D() );
        }

        //====================================================================================
        // Comparison operators
        //====================================================================================

        bool operator==( const cPlane &rhs ) const
        {
            return ( mVals[0] == rhs.mVals[0] && mVals[1] == rhs.mVals[1] && mVals[2] == rhs.mVals[2] && mVals[3] == rhs.mVals[3] );
        }

        bool operator!=( const cPlane &rhs ) const 
        { 
            return !( *this == rhs ); 
        }

        //====================================================================================
        // Helper constants
        //====================================================================================
        
        static const cPlane kZero;

    private:
        T mVals[4];
    };

    //=========================================================================
    // Stream I/O operators
    //=========================================================================
    template< typename T>
    std::wostream& operator<<( std::wostream& os, const cPlane<T>& vec )
    {
        os << L"(" << vec[0] << L"," << vec[1] << L"," << vec[2] << L"," << vec[3] << L")";
        return os;
    }

    template< typename T>
    std::ostream& operator<<( std::ostream& os, const cPlane<T>& vec )
    {
        os << "(" << vec[0] << "," << vec[1] << "," << vec[2] << "," << vec[3] << ")";
        return os;
    }

    template <typename T>
    const cPlane<T> cPlane<T>::kZero( 0, 0, 0, 0 );

    using cPlanef = cPlane<float>;
    using cPlaned = cPlane<double>;
}

#endif // __PLATFORM__LINUX__

