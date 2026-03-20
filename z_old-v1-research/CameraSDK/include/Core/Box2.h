//======================================================================================================
// Copyright 2014, NaturalPoint Inc.
//======================================================================================================
#pragma once

// Local includes
#include "Core/Vector2.h"

namespace Core
{
    /// <summary>
    /// A 2D box, with methods for comparing and intersecting boxes.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<typename T>
    class cBox2
    {
    public:
        cBox2() = default; // No initialization

        /// <summary>
        /// Initialize a new box. Coordinates are understood to increase from left to right and bottom to top.
        /// </summary>
        cBox2( const cVector2<T> &bottomLeft, const cVector2<T> &topRight )
        {
            // TODO : Put checks in place to ensure passed values are arranged correctly.
            SetValues( bottomLeft, topRight );
        }

        /// <summary>Set the vector values.</summary>
        void SetValues( const cVector2<T> &bottomLeft, const cVector2<T> &topRight )
        {
            mCorners[0] = bottomLeft;
            mCorners[1] = topRight;
        }

        /// <summary>Retrieve components of the box.</summary>
        cVector2<T> BottomLeft() const { return mCorners[0]; }
        cVector2<T>& BottomLeft() { return mCorners[0]; }
        cVector2<T> TopRight() const { return mCorners[1]; }
        cVector2<T>& TopRight() { return mCorners[1]; }
        cVector2<T> Center() const { return ( mCorners[0] + mCorners[1] ) / (T)2; }
        cVector2<T>& Center() { return ( mCorners[0] + mCorners[1] ) / (T)2; }

        T Left() const { return mCorners[0].X(); }
        T Right() const { return mCorners[1].X(); }
        T Bottom() const { return mCorners[0].Y(); }
        T Top() const { return mCorners[1].Y(); }
        T Width() const { return mCorners[1].X() - mCorners[0].X(); }
        T Height() const { return mCorners[1].Y() - mCorners[0].Y(); }

        /// <summary>Returns true if the box has no area.</summary>
        bool Empty() const
        {
            return ( Left() >= Right() || Bottom() >= Top() );
        }

        //==============================================================================================
        // Mathematical and assignment operators
        //==============================================================================================

        cBox2 operator-() const { return cBox2( -mCorners[0], -mCorners[1] ); }

        //==============================================================================================
        // Comparison operators
        //==============================================================================================

        bool operator==( const cBox2 &rhs ) const
        {
            return ( mCorners[0] == rhs.mCorners[0] && mCorners[1] == rhs.mCorners[1] );
        }

        bool operator!=( const cBox2 &rhs ) const { return !( *this == rhs ); }

        /// <summary>Compare two vectors to within a tolerance.</summary>
        bool Equals( const cBox2 &rhs, T tolerance ) const
        {
            tolerance *= tolerance;

            return ( ( mCorners[0] - rhs.mCorners[0] ).LengthSquared() < tolerance
                && ( mCorners[1] - rhs.mCorners[1] ).LengthSquared() < tolerance );
        }

        bool Intersects( const cBox2& rhs ) const
        {
            return Left() < rhs.Right() && Right() > rhs.Left() && Bottom() < rhs.Top() && Top() > rhs.Bottom();
        }

        //==============================================================================================
        // Helper constants
        //==============================================================================================
        
        static const cBox2<T> kZero;

    private:
        cVector2<T> mCorners[2];
    };

    template <typename T>
    const cBox2<T> cBox2<T>::kZero( cVector2<T>( 0, 0 ), cVector2<T>( 0, 0 ) );

    using cBox2f = cBox2<float>;
    using cBox2d = cBox2<double>;
    using cBox2i = cBox2<int>;
}

