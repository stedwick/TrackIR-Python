//======================================================================================================
// Copyright 2016, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <algorithm>

// Local includes
#include "Core/Vector3.h"

// Avoid Windows' definition of max, which overrides std::max
#undef min
#undef max

namespace Core
{
    /// <summary>A cubic 3D region.</summary>
    template<typename T>
    class cRegion3
    {
    public:
        /// <summary>A 3D region.</summary>
        cRegion3() : mLowerBound( 0, 0, 0 ), mUpperBound( 0, 0, 0 ) { }

        /// <summary>A 3D region bound over range x = { minX->maxX }, y = { minY, maxY }, z = { minZ, maxZ }.</summary>
        cRegion3( T minX, T minY, T minZ, T maxX, T maxY, T maxZ ) : mLowerBound( minX, minY, minZ ), mUpperBound( maxX, maxY, maxZ ) { }

        /// <summary>A 3D region bound over range x = { min.X()->max.X() }, y = { min.Y(), max.Y() }, z = { min.Z(), max.Z() }.</summary>
        cRegion3( const cVector3<T>& min, const cVector3<T>& max ) : mLowerBound( min ), mUpperBound( max ) { }

        /// <summary>A 3D region of zero size at location.</summary>
        cRegion3( const cVector3<T>& location ) : mLowerBound( location ), mUpperBound( location ) { }

        /// <summary>Set the center point of the cuboid.</summary>
        void SetCenterAndWidths( const cVector3<T>& center, const cVector3<T>& widths )
        {
            cVector3<T> halfWidths( widths / 2 );

            mUpperBound = center + halfWidths;
            mLowerBound = center - halfWidths;
        }

        /// <summary>Set the center point of the cuboid.</summary>
        void SetCenter( const cVector3<T>& center )
        {
            cVector3<T> halfWidths( Widths() / 2 );

            mUpperBound = center + halfWidths;
            mLowerBound = center - halfWidths;
        }

        /// <summary>Set the center point of the cuboid.</summary>
        cVector3<T> Center() const
        {
            return cVector3<T>( ( mUpperBound + mLowerBound ) / 2 );
        }

        /// <summary>Set the widths along each axis.</summary>
        void SetWidths( const cVector3<T>& widths )
        {
            cVector3<T> center( Center() );
            cVector3<T> halfWidths( widths / 2 );

            mUpperBound = center + halfWidths;
            mLowerBound = center - halfWidths;
        }

        /// <summary>Fetch the widths along each axis.</summary>
        cVector3<T> Widths() const
        {
            return cVector3<T>( mUpperBound - mLowerBound );
        }

        /// <summary>Fetch the lower bound.</summary>
        const cVector3<T>& LowerBound() const { return mLowerBound; }

        /// <summary>Fetch the upper bound.</summary>
        const cVector3<T>& UpperBound() const { return mUpperBound; }

        /// <summary>Expand region by amount.</summary>
        void ExpandRegion( T amount )
        {
            if( amount < 0 )
            {
                ContractRegion( -amount );
            }
            else
            {
                mLowerBound = mLowerBound - cVector3<T>( amount, amount, amount );
                mUpperBound = mUpperBound + cVector3<T>( amount, amount, amount );
            }
        }

        /// <summary>Contract region by amount.</summary>
        void ContractRegion( T amount )
        {
            if( amount < 0 )
            {
                ExpandRegion( -amount );
                return;
            }

            if( mLowerBound.X() + amount > mUpperBound.X() - amount )
            {
                mLowerBound.X() = mUpperBound.X() = ( mLowerBound.X() + mUpperBound.X() ) / 2;
            }
            else
            {
                mLowerBound.X() = mLowerBound.X() + amount;
                mUpperBound.X() = mUpperBound.X() + amount;
            }

            if( mLowerBound.Y() + amount > mUpperBound.Y() - amount )
            {
                mLowerBound.Y() = mUpperBound.Y() = ( mLowerBound.Y() + mUpperBound.Y() ) / 2;
            }
            else
            {
                mLowerBound.Y() = mLowerBound.Y() + amount;
                mUpperBound.Y() = mUpperBound.Y() + amount;
            }

            if( mLowerBound.Z() + amount > mUpperBound.Z() - amount )
            {
                mLowerBound.Z() = mUpperBound.Z() = ( mLowerBound.Z() + mUpperBound.Z() ) / 2;
            }
            else
            {
                mLowerBound.Z() = mLowerBound.Z() + amount;
                mUpperBound.Z() = mUpperBound.Z() + amount;
            }
        }

        /// <summary>Expand region to include location.</summary>
        void ExpandRegion( const cVector3<T>& location )
        {
            if( mLowerBound.X() > location.X() )
            {
                mLowerBound.X() = location.X();
            }
            else if( mUpperBound.X() < location.X() )
            {
                mUpperBound.X() = location.X();
            }

            if( mLowerBound.Y() > location.Y() )
            {
                mLowerBound.Y() = location.Y();
            }
            else if( mUpperBound.Y() < location.Y() )
            {
                mUpperBound.Y() = location.Y();
            }

            if( mLowerBound.Z() > location.Z() )
            {
                mLowerBound.Z() = location.Z();
            }
            else if( mUpperBound.Z() < location.Z() )
            {
                mUpperBound.Z() = location.Z();
            }
        }

        /// <summary>Returns the longest axis distance</summary>
        T LargestAxialDistance() const
        {
            cVector3<T> distances( mUpperBound - mLowerBound );

            T largest = std::max( distances.X(), distances.Y() );
            largest = std::max( largest, distances.Z() );

            return largest;
        }

        /// <summary>Returns the shortest axis distance</summary>
        T SmallestAxialDistance() const
        {
            cVector3<T> distances( mUpperBound - mLowerBound );

            T smallest = std::min( distances.X(), distances.Y() );
            smallest = std::min( smallest, distances.Z() );

            return smallest;
        }

        /// <summary>Is location within region.</summary>
        bool Contains( const cVector3<T>& location ) const
        {
            return ( mLowerBound.X() <= location.X() && mLowerBound.Y() <= location.Y() && mLowerBound.Z() <= location.Z()
                && mUpperBound.X() >= location.X() && mUpperBound.Y() >= location.Y() && mUpperBound.Z() >= location.Z() );
        }

        static const cRegion3 kZero;

    private:
        cVector3<T> mLowerBound;
        cVector3<T> mUpperBound;
    };

    template <typename T>
    const cRegion3<T> cRegion3<T>::kZero;

    using cRegion3i = cRegion3<int>;
    using cRegion3f = cRegion3<float>;
    using cRegion3d = cRegion3<double>;

    /// <summary>A spherical 3D region centered at mCenter, with radius mRadius.</summary>

    template<typename T>
    class cSphericalRegion3
    {
    public:
        /// <summary>A spherical 3D region.</summary>
        cSphericalRegion3() : mCenter( 0, 0, 0 ), mRadius( 0 ), mRadiusSquared( 0 ) { }

        /// <summary>A spherical 3D region centered around { center }.</summary>
        cSphericalRegion3( const cVector3<T>& center ) : mCenter( center ), mRadius( 0 ), mRadiusSquared( 0 ) { }

        /// <summary>A spherical 3D region centered around { center } and including up to distance radius.</summary>
        cSphericalRegion3( const cVector3<T>& center, T radius ) : mCenter( center )
            , mRadius( radius ), mRadiusSquared( radius* radius ) { }

        /// <summary>A spherical 3D region centered around { centerX, centerY }.</summary>
        cSphericalRegion3( T centerX, T centerY, T centerZ ) : mCenter( centerX, centerY, centerZ )
            , mRadius( 0 ), mRadiusSquared( 0 ){ }

        /// <summary>A spherical 3D region centered around { centerX, centerY, centerZ } and including up to distance radius.</summary>
        cSphericalRegion3( T centerX, T centerY, T centerZ, T radius ) : mCenter( centerX, centerY, centerZ )
            , mRadius( radius ), mRadiusSquared( radius * radius ) { }

        /// <summary>Set the spherical region.</summary>
        void SetRegion( T centerX, T centerY, T centerZ, T radius )
        {
            mCenter.SetValues( centerX, centerY, centerZ );
            mRadius = radius;
            mRadiusSquared = radius * radius;
        }

        /// <summary>Set the spherical region.</summary>
        void SetRegion( const cVector3<T>& center, T radius )
        {
            mCenter = center;
            mRadius = radius;
            mRadiusSquared = radius * radius;
        }

        /// <summary>Set the center of the spherical 3D region.</summary>
        void SetCenter( T centerX, T centerY, T centerZ ) { mCenter.SetValues( centerX, centerY, centerZ ); }

        /// <summary>Set the center of the spherical 3D region.</summary>
        void SetCenter( const cVector3<T>& center ) { mCenter = center; }

        /// <summary>Set the radius of the spherical 3D region.</summary>
        void SetRadius( T radius )
        {
            mRadius = radius;
            mRadiusSquared = radius * radius;
        }

        /// <summary>Fetch the center of the spherical 3D region.</summary>
        const cVector3<T>& Center() const { return mCenter; }

        /// <summary>Fetch the radius of the spherical 3D region.</summary>
        const T& Radius() const { return mRadius; }

        /// <summary>Expand region to include location x,y,z.</summary>
        void ExpandRegion( T x, T y, T z )
        {
            T distanceSquared = DistanceSquared( x, y, z );

            if( distanceSquared > mRadiusSquared )
            {
                mRadius = sqrt( distanceSquared );
                mRadiusSquared = distanceSquared;
            }
        }

        /// <summary>Expand region to include location.</summary>
        void ExpandRegion( const cVector3<T>& location )
        {
            ExpandRegion( location.X(), location.Y(), location.Z() );
        }

        /// <summary>Expand region by amount.</summary>
        void ExpandRegion( T amount )
        {
            if( amount < 0 )
            {
                ContractRegion( -amount );
            }
            else
            {
                mRadius = mRadius + amount;
                mRadiusSquared = mRadius * mRadius;
            }
        }

        /// <summary>Contract region by amount.</summary>
        void ContractRegion( T amount )
        {
            if( amount < 0 )
            {
                ExpandRegion( -amount );
            }
            else
            {
                mRadius = mRadius - amount;

                if( mRadius < 0 )
                {
                    mRadius = 0;
                }
                mRadiusSquared = mRadius * mRadius;
            }
        }

        /// <summary>Distance from location x,y,z to center of spherical region.</summary>
        T Distance( T x, T y, T z ) const
        {
            T squared = DistanceSquared( x, y, z );

            return sqrt( squared );
        }
        T DistanceSquared( T x, T y, T z ) const
        {
            return ( ( x - mCenter.X() ) * ( x - mCenter.X() ) + ( y - mCenter.Y() ) * ( y - mCenter.Y() ) + ( z - mCenter.Z() ) * ( z - mCenter.Z() ) );
        }

        /// <summary>Distance from location  to center of spherical region.</summary>
        T Distance( const cVector3<T>& location ) const
        {
            return Distance( location.X(), location.Y(), location.Z() );
        }
        T DistanceSquared( const cVector3<T>& location ) const
        {
            return DistanceSquared( location.X(), location.Y(), location.Z() );
        }

        /// <summary>Is location within region.</summary>
        bool Contains( T x, T y, T z ) const
        {
            if( DistanceSquared( x, y, z ) < mRadiusSquared )
            {
                return true;
            }

            return false;
        }

        /// <summary>Is location within region.</summary>
        bool Contains( const cVector3<T>& location ) const
        {
            return Contains( location.X(), location.Y(), location.Z() );
        }

        static const cSphericalRegion3 kZero;

    private:
        cVector3<T> mCenter;
        T mRadius;
        T mRadiusSquared;
    };

    template <typename T>
    const cSphericalRegion3<T> cSphericalRegion3<T>::kZero;

    using cSphericalRegion3i = cSphericalRegion3<int>;
    using cSphericalRegion3f = cSphericalRegion3<float>;
    using cSphericalRegion3d = cSphericalRegion3<double>;

    /// <summary>An ellipsoid 3D region centered at mCenter, with half-widths on each major axis defined by mAxisLength.</summary>
    template<typename T>
    class cEllipsoidRegion3
    {
    public:
        /// <summary>An elliptical 3D region.</summary>
        cEllipsoidRegion3() : mCenter( 0, 0, 0 ), mAxisLength( 0, 0, 0 ) { }

        /// <summary>An elliptical 3D region centered around { center }.</summary>
        cEllipsoidRegion3( const cVector3<T>& center ) : mCenter( center ), mAxisLength( 0, 0, 0 ) { }

        /// <summary>An elliptical 3D region centered around { center } and including up to distance radius.</summary>
        cEllipsoidRegion3( const cVector3<T>& center, const cVector3<T>& axisLengths ) : mCenter( center ), mAxisLength( axisLengths ) { }

        /// <summary>Set the region.</summary>
        void SetRegion( cVector3<T> center, cVector3<T> axisLengths ) { mCenter = center; mAxisLength = axisLengths; }

        /// <summary>Set the center of the 3D region.</summary>
        void SetCenter( const cVector3<T>& center ) { mCenter = center; }

        /// <summary>Set the length of each axis of the ellipsoid. The ellipsoid will fit perfectly in a box centered
        /// at center, with box sides each of lengths axisLength * 2.</summary>
        void SetAxisLengths( const cVector3<T>& axisLengths ) { mAxisLength = axisLengths; }

        /// <summary>Fetch the center of the 3D region.</summary>
        const cVector3<T>& Center() const { return mCenter; }

        /// <summary>Fetch the axis lengths of the 3D region.</summary>
        const cVector3<T>& AxisLengths() const { return mAxisLength; }

        /// <summary>Is location within region.</summary>
        bool Contains( const cVector3<T>& location ) const
        {
            cVector3<T> pnt( location - mCenter );
            T x = pnt.X() / mAxisLength.X();
            T y = pnt.Y() / mAxisLength.Y();
            T z = pnt.Z() / mAxisLength.Z();
            return ( x * x + y * y + z * z ) <= 1;
        }

        static const cEllipsoidRegion3 kZero;

    private:
        cVector3<T> mCenter;
        cVector3<T> mAxisLength;
    };

    template <typename T>
    const cEllipsoidRegion3<T> cEllipsoidRegion3<T>::kZero;

    using cEllipsoidRegion3i = cEllipsoidRegion3<int>;
    using cEllipsoidRegion3f = cEllipsoidRegion3<float>;
    using cEllipsoidRegion3d = cEllipsoidRegion3<double>;

    /// <summary>A cylindrical 3D region, with the center of the base at mCenter, revolved around the Y axis
    /// with radius mRadius and with height mHeight.</summary>
    template<typename T>
    class cCylindricalRegion3
    {
    public:
        /// <summary>A cylindrical 3D region.</summary>
        cCylindricalRegion3() : mCenter( 0, 0, 0 ), mRadius( 0 ), mHeight( 0 ) { }

        /// <summary>A cylindrical 3D region centered around { center }.</summary>
        cCylindricalRegion3( const cVector3<T>& center ) : mCenter( center ), mRadius( 0 ), mHeight( 0 ) { }

        /// <summary>A cylindrical 3D region centered around { center } and including up to distance radius.</summary>
        cCylindricalRegion3( const cVector3<T>& center, T radius, T height ) : mCenter( center ), mRadius( radius ), mHeight( height ) { }

        /// <summary>Set the region.</summary>
        void SetRegion( cVector3<T> center, T radius, T height )
        {
            mCenter = center;
            mRadius = radius;
            mHeight = height;
        }

        /// <summary>Set the center of the 3D region.</summary>
        void SetCenter( const cVector3<T>& center ) { mCenter = center; }

        /// <summary>Set the radius of the base of the cylinder.</summary>
        void SetRadius( T radius ) { mRadius = radius; }

        /// <summary>Set the height of the cylinder.</summary>
        void SetHeight( T height ) { mHeight = height; }

        /// <summary>Fetch the center point of the base of the cylinder.</summary>
        const cVector3<T>& Center() const { return mCenter; }

        /// <summary>Fetch the radius of the cylinder base.</summary>
        T Radius() const { return mRadius; }

        /// <summary>Fetch the height of the cylinder.</summary>
        T Height() const { return mHeight; }

        /// <summary>Expand region to include location.</summary>
        void ExpandRegion( const cVector3<T>& location )
        {
            if( location.Y() < mCenter.Y() )
            {
                mHeight += mCenter.Y() - location.Y();  // Point was below the base
                mCenter.Y() = location.Y();
            }
            else if( ( location.Y() - mCenter.Y() ) < mHeight )
            {
                mHeight = ( location.Y() - mCenter.Y() ); // Point was above top of cylinder
            }

            T rSquared = mRadius * mRadius;
            T locationDistanceSquared = ( location.X() * location.X() + location.Z() * location.Z() );

            if( locationDistanceSquared > rSquared )
            {
                mRadius = sqrt( locationDistanceSquared );
            }
        }

        /// <summary>Is location within region.</summary>
        bool Contains( const cVector3<T>& location ) const
        {
            if( location.Y() < mCenter.Y() || location.Y() > mCenter.Y() + mHeight )
            {
                return false;
            }

            T rSquared = mRadius * mRadius;
            cVector3<T> relativeLocation( location - mCenter );
            T locationDistanceSquared = ( relativeLocation.X() * relativeLocation.X() + relativeLocation.Z() * relativeLocation.Z() );

            return ( locationDistanceSquared <= rSquared );
        }

        static const cCylindricalRegion3 kZero;

    private:
        cVector3<T> mCenter;
        T mRadius;
        T mHeight;
    };

    template <typename T>
    const cCylindricalRegion3<T> cCylindricalRegion3<T>::kZero;

    using cCylindricalRegion3i = cCylindricalRegion3<int>;
    using cCylindricalRegion3f = cCylindricalRegion3<float>;
    using cCylindricalRegion3d = cCylindricalRegion3<double>;
}


