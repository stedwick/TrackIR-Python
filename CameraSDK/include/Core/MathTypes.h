//======================================================================================================
// Copyright 2013, NaturalPoint Inc.
//======================================================================================================
#pragma once

namespace Core
{
    const double kPi = 3.1415926535897932384626433;
    const float  kPif = 3.14159265f;

    const double ke = 2.71828182845904523536;
    const float  kef = 2.7182818284f;

    ///<summary>Convert Degrees to Radians.</summary>
    template<typename T>
    inline T DegreesToRadians( T val )
    {
        return val * ( 0.017453292519943295769236907684 );
    }

    ///<summary>Convert Radians to Degrees.</summary>
    template<typename T>
    inline T RadiansToDegrees( T val )
    {
        return val * ( 57.29577951308232087679815481410 );
    }
}
