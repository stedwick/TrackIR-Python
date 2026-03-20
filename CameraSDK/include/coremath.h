//======================================================================================================
// Copyright 2009, NaturalPoint Inc.
//======================================================================================================

// NOTE: The comments immediately following seem misleading. Here's what I've found looking at this code:
//
//      1. Matrix::get_YRP() is consistent with Eberly's ZXY factorization (WITHOUT handling degenerate cases). [1]
//      2. Returned values: "Pitch" is rotation about X, "yaw" is rotation about Y, and "roll" is rotation about Z.
//      3. That IS consistent with Computer Graphics: Principles and Practice (Foley, van Dam, et al), which describes
//         a right-handed, Y-up, Z-forward coordinate system, but NOT with Essential Mathematics (Van Verth, et al).
//      4. That means that get_YRP() actually returns Tait-Bryan angles in the sequence z-x'-y'' (roll, pitch, yaw)!
//
// [1]: Eberly, David (1999). Euler Angle Formulas. Section 2.5, formula 13.
//      Retrieved from http://www.geometrictools.com/Documentation/EulerAngles.pdf
//
// - Zach Brockway, 2016-04-11
//
// Original comments follow.


// Core::Matrix uses CLASSIC coordinates. It matches the math shown in the following
// textbooks:
// Computer Graphics: Foley et al (seminal)
// Essential Mathematics for Games and Interactive Applications: Van Verth et al
//
// That means the distance from the viewer to the view is the X coordinate
// up and down is Z
// left and right is Y
// This module is backwards X & Y relative to these coordinates.
// Thus when we create matricies, we need to swap X and Y
// It is better to do the swap than to rewrite the math for the weird
// coordinate system.
// we also need to invert roll after using this.
// R_z = yaw
// R_y = pitch
// R_x = roll
#pragma once

#include <cmath>

#include "cameracommonglobals.h"

namespace Core
{
    class Matrix;

    class CLAPI Angle
    {
    public:
        Angle() : mAngle( 0.0 ) { }
        Angle( double angle ) : mAngle( angle ) { }
        ~Angle() = default;

        void operator/= ( double Value );
        void operator*= ( double Value );
        void operator-= ( double Value );
        void operator+= ( double Value );
        Angle &operator= ( double a );

        bool operator==( const Angle& a ) const { return mAngle == a.mAngle; }
        bool operator<( const Angle& a ) const { return mAngle < a.mAngle; }
        operator double();

        void FromXY( double X, double Y );

    private:
        void BoundsCheck();
        double mAngle;
    };

    class CLAPI Vector
    {
    public:
        Vector()
            : x( 0.0 )
            , y( 0.0 )
            , z( 0.0 )
        {
        }

        Vector( double X, double Y, double Z )
            : x( X )
            , y( Y )
            , z( Z )
        {
        }

        double X() const { return x; }
        double Y() const { return y; }
        double Z() const { return z; }

        void Set( double X, double Y, double Z )
        {
            x = X;
            y = Y;
            z = Z;
        }

        void operator/=( double d );
        void operator*=( double d );
        void operator-=( const Vector& vec );
        void operator+=( const Vector& vec );

        void operator*=( const Matrix *mat );

        double Dot( const Vector& vec ) const;
        void Cross( const Vector& a, const Vector& b );
        double Length() const;
        void Normalize();
        double Distance( const Vector &vec ) const;
        double SquareDistance( const Vector &vec ) const;

        double x, y, z;
    };

    inline float fastsqrt( float val )
    {
        union
        {
            int tmp;
            float val;
        } u;
        u.val = val;
        u.tmp -= 1 << 23; /* Remove last bit so 1.0 gives 1.0 */
                          /* tmp is now an approximation to logbase2(val) */
        u.tmp >>= 1; /* divide by 2 */
        u.tmp += 1 << 29; /* add 64 to exponent: (e+127)/2 =(e/2)+63, */
                          /* that represents (e/2)-64 but we want e/2 */
        return u.val;
    }

    struct VectorPtr
    {
        int Count;
        double* p;
        int increment;

        void operator++() { p += increment; }
        operator double() { return *p; }
    };

    class CLAPI Matrix
    {
    public:
        Matrix( int _R, int _C, double *p );
        Matrix( Vector vec );
        Matrix();

        int Rows;
        int Columns;
        int Count;
        double Data[4 * 4];

        void Row( VectorPtr& v, int r );
        void Column( VectorPtr& v, int c );
        double Norm() const;
        void Normalize();

        Matrix &operator/=( double d );
        Matrix &operator*=( double d );
        Matrix &operator-=( const Matrix &rhs );
        Matrix &operator+=( const Matrix &rhs );

        void Transpose();
        const Matrix& Multiply( Matrix& lhs, Matrix& rhs );
        void Cross( Matrix &lhs, Matrix &rhs );
        void set_Column( Matrix &rhs, int c );
        int offset( int r, int c );
        void get_YPR( double &yaw, double &pitch, double &roll );
        void get_YRP( double &yaw, double &pitch, double &roll );
        void CalculateOrthogonalMatrix( const Vector v1, const Vector v2, const Vector v3 );
        void CalculateOrthogonalMatrix( Matrix &p0, Matrix &p1, Matrix &p2 );
        void CreateFromYRP( double yaw, double pitch, double roll );
        void Identity();

        static double Dot( VectorPtr& lhs, VectorPtr& rhs );
    };

    struct CLAPI DistortionModel
    {
        DistortionModel();

        bool   Distort;                     //== Enable/Disable Distortion Model
        double LensCenterX;                 //== Primary point X (in pixels)
        double LensCenterY;                 //== Primary point Y (in pixels)
        double HorizontalFocalLength;       //== Horizontal Focal Length (in pixels)
        double VerticalFocalLength;         //== Vertical Focal Length {in pixels)
        double KC1;                         //== Distortion Parameter 1
        double KC2;                         //== Distortion Parameter 2
        double KC3;                         //== Distortion Parameter 3
        double Tangential0;                 //== Tangential 0
        double Tangential1;                 //== Tangential 1
    };

    CLAPI void Undistort2DPoint( DistortionModel &Model, float &X, float &Y );

    template <typename T>
    void Swap( T &A, T &B )
    {
        T temp = A;
        A = B;
        B = temp;
    }

    template <typename T>
    void LowToHigh( T& A, T& B )
    {
        if( A > B )
        {
            T temp = A;
            A = B;
            B = temp;
        }
    }


} // namespace Core

