//======================================================================================================
// Copyright 2016, NaturalPoint Inc.
//======================================================================================================
#pragma once

// Local includes
#include "Core/Vector2.h"

namespace Core
{
    /// <summary>A rectangular 2D region.</summary>
    template<typename T>
    class cRegion2
    {
    public:
        /// <summary>A rectangular 2D region.</summary>
        cRegion2() : mLowerBound( 0, 0 ), mUpperBound( 0, 0 ) { }

        /// <summary>A rectangular 2D region bound over range x = { minX->maxX }, y = { minY, maxY }.</summary>
        cRegion2( T minX, T minY, T maxX, T maxY ) : mLowerBound( minX, minY ), mUpperBound( maxX, maxY ) { }

        /// <summary>A rectangular 2D region bound over range x = { min.X()->max.X() }, y = { min.Y(), max.Y() }.</summary>
        cRegion2( cVector2<T> min, cVector2<T> max ) : mLowerBound( min ), mUpperBound( max ) { }

        /// <summary>A rectangular 2D region of zero size at location.</summary>
        cRegion2( cVector2<T> location ) : mLowerBound( location ), mUpperBound( location ) { }

        /// <summary>Set the bounds of the 2D region.</summary>
        void SetBounds( T minX, T minY, T maxX, T maxY )
        {
            mLowerBound.SetValues( minX, minY );
            mUpperBound.SetValues( maxX, maxY );
        }

        /// <summary>Set the lower bound of the 2D region.</summary>
        void SetLowerBound( T minX, T minY ) { mLowerBound.SetValues( minX, minY ); }

        /// <summary>Set the upper bound of the 2D region.</summary>
        void SetUpperBound( T maxX, T maxY ) { mUpperBound.SetValues( maxX, maxY ); }

        /// <summary>Set the lower bound of the 2D region.</summary>
        void SetLowerBound( cVector2<T> lowerBound ) { mLowerBound = lowerBound; }

        /// <summary>Set the upper bound of the 2D region.</summary>
        void SetUpperBound( cVector2<T> upperBound ) { mUpperBound = upperBound; }

        /// <summary>Fetch the lower bound.</summary>
        const cVector2<T>& LowerBound() const { return mLowerBound; }

        /// <summary>Fetch the upper bound.</summary>
        const cVector2<T>& UpperBound() const { return mUpperBound; }

        /// <summary>Convenience method to access the minimum X value.</summary>
        T MinX() const { return mLowerBound.X(); }

        /// <summary>Convenience method to access the minimum X value.</summary>
        T MaxX() const { return mUpperBound.X(); }

        /// <summary>Convenience method to access the minimum X value.</summary>
        T MinY() const { return mLowerBound.Y(); }

        /// <summary>Convenience method to access the minimum X value.</summary>
        T MaxY() const { return mUpperBound.Y(); }

        /// <summary>Expand region to include location x,y.</summary>
        void ExpandRegion( T x, T y )
        {
            if( mLowerBound.X() > x )
            {
                mLowerBound.X() = x;
            }
            if( mLowerBound.Y() > y )
            {
                mLowerBound.Y() = y;
            }
            if( mUpperBound.X() < x )
            {
                mUpperBound.X() = x;
            }
            if( mUpperBound.Y() < y )
            {
                mUpperBound.Y() = y;
            }
        }

        /// <summary>Expand region to include location.</summary>
        void ExpandRegion( cVector2<T> location )
        {
            ExpandRegion( location.X(), location.Y() );
        }

        /// <summary>Expand region by amount.</summary>
        void ExpandRegion( T amount )
        {
            if( amount < 0 )
            {
                ContractRegion( -amount );
                return;
            }

            mLowerBound = mLowerBound - Core::cVector2<T>( amount, amount );
            mUpperBound = mUpperBound + Core::cVector2<T>( amount, amount );
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
        }

        /// <summary>Region width</summary>
        T Width() const
        {
            return ( mUpperBound.X() - mLowerBound.X() );
        }

        /// <summary>Region height</summary>
        T Height() const
        {
            return ( mUpperBound.Y() - mLowerBound.Y() );
        }

        /// <summary>Is location within region.</summary>
        bool IsWithin( T x, T y ) const
        {
            if( mLowerBound.X() <= x && mLowerBound.Y() <= y && mUpperBound.X() >= x && mUpperBound.Y() >= y )
            {
                return true;
            }

            return false;
        }

        /// <summary>Is location within region.</summary>
        bool IsWithin( cVector2<T> location ) const
        {
            return IsWithin( location.X(), location.Y() );
        }

        static const cRegion2 kZero;

    private:
        cVector2<T>  mLowerBound;
        cVector2<T>  mUpperBound;
    };

    template <typename T>
    const cRegion2<T> cRegion2<T>::kZero( 0, 0, 0, 0 );

    typedef cRegion2<int> cRegion2i;
    typedef cRegion2<float> cRegion2f;
    typedef cRegion2<double> cRegion2d;

    /// <summary>A circular 2D region.</summary>
    template<typename T>
    class cCircularRegion2
    {
    public:
        /// <summary>A circular 2D region.</summary>
        cCircularRegion2() : mCenter( 0, 0 ), mRadius( 0 ) { }

        /// <summary>A circular 2D region centered around { center }.</summary>
        cCircularRegion2( cVector2<T> center ) : mCenter( center ), mRadius( 0 ) { }

        /// <summary>A circular 2D region centered around { center } and including up to distance radius.</summary>
        cCircularRegion2( cVector2<T> center, T radius ) : mCenter( center ), mRadius( radius ) { }

        /// <summary>A circular 2D region centered around { centerX, centerY }.</summary>
        cCircularRegion2( T centerX, T centerY ) : mCenter( centerX, centerY ), mRadius( 0 ) { }

        /// <summary>A circular 2D region centered around { centerX, centerY } and including up to distance radius.</summary>
        cCircularRegion2( T centerX, T centerY, T radius ) : mCenter( centerX, centerY ), mRadius( radius ) { }

        /// <summary>Set the circular region.</summary>
        void SetRegion( T centerX, T centerY, T radius ) { mCenter.SetValues( centerX, centerY ); mRadius = radius; }

        /// <summary>Set the circular region.</summary>
        void SetRegion( cVector2<T> center, T radius ) { mCenter = center; mRadius = radius; }

        /// <summary>Set the center of the circular 2D region.</summary>
        void SetCenter( T centerX, T centerY ) { mCenter.SetValues( centerX, centerY ); }

        /// <summary>Set the center of the circular 2D region.</summary>
        void SetCenter( cVector2<T> center ) { mCenter = center; }

        /// <summary>Set the radius of the circular 2D region.</summary>
        void SetRadius( T radius ) { mRadius = radius; }

        /// <summary>Fetch the center of the circular 2D region.</summary>
        const cVector2<T>& Center() const { return mCenter; }

        /// <summary>Fetch the radius of the circular 2D region.</summary>
        const T& Radius() const { return mRadius; }

        /// <summary>Expand region to include location x,y.</summary>
        void ExpandRegion( T x, T y )
        {
            T distance = Distance( x, y );

            if( distance > mRadius )
            {
                mRadius = distance;
            }
        }

        /// <summary>Expand region to include location.</summary>
        void ExpandRegion( cVector2<T> location )
        {
            ExpandRegion( location.X(), location.Y() );
        }

        /// <summary>Expand region by amount.</summary>
        void ExpandRegion( T amount )
        {
            if( amount < 0 )
            {
                ContractRegion( -amount );
                return;
            }

            mRadius = mRadius + amount;
        }

        /// <summary>Contract region by amount.</summary>
        void ContractRegion( T amount )
        {
            if( amount < 0 )
            {
                ExpandRegion( -amount );
                return;
            }

            mRadius = mRadius - amount;

            if( mRadius < 0 )
            {
                mRadius = 0;
            }
        }

        /// <summary>Distance from location x,y to center of circular region.</summary>
        T Distance( T x, T y ) const
        {
            T squared = ( ( ( x - mCenter.X() ) * ( x - mCenter.X() ) ) + ( ( y - mCenter.Y() ) * ( y - mCenter.Y() ) ) );

            return sqrt( squared );
        }

        /// <summary>Distance from location  to center of circular region.</summary>
        T Distance( cVector2<T> location ) const
        {
            return Distance( location.X(), location.Y() );
        }

        /// <summary>Is location within region.</summary>
        bool IsWithin( T x, T y ) const
        {
            if( Distance( x, y ) < mRadius )
            {
                return true;
            }

            return false;
        }

        /// <summary>Is location within region.</summary>
        bool IsWithin( cVector2<T> location ) const
        {
            return IsWithin( location.X(), location.Y() );
        }

        static const cCircularRegion2 kZero;

    private:
        cVector2<T>  mCenter;
        T mRadius;
    };

    template <typename T>
    const cCircularRegion2<T> cCircularRegion2<T>::kZero( 0, 0, 0 );

    typedef cCircularRegion2<int> cCircularRegion2i;
    typedef cCircularRegion2<float> cCircularRegion2f;
    typedef cCircularRegion2<double> cCircularRegion2d;
}


