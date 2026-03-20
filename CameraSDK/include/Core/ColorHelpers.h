//======================================================================================================
// Copyright 2013, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include "Core/BuildConfig.h"
#include "Core/Vector3.h"
#include "Core/Vector4.h"

namespace Core
{
    /// <summary>A platform-neutral color representation.</summary>
    struct CORE_API sHSVColor
    {
    public:
        /// <summary>Hue value [0-360)</summary>
        float H;

        /// <summary>Saturation value [0-1]</summary>
        float S;

        /// <summary>Color value [0-1]</summary>
        float V;

        /// <summary>Initialize an HSV color with RGB values (in 0-1 range).</summary>
        sHSVColor( float h, float s, float v ) : H( h ), S( s ), V( v ) { }

        /// <summary>Initialize an HSV color with RGB values (in 0-255 range).</summary>
        sHSVColor( int r, int g, int b );
    };

    /// <summary>A collection of platform-neutral color management and conversion routines.</summary>
    class CORE_API cColorHelpers
    {
    public:
        /// <summary>
        /// Convert from a packed int representation to an RGB 3-vector representation. Alpha values are lost
        /// in this conversion.
        /// </summary>
        /// <param name="color">Packed int32 color representation in AABBGGRR order.</param>
        /// <returns>3-vector color representation in RGB order with each component in the [0-1] range.</returns>
        static cVector3f PackedIntToVector3( unsigned int color );

        /// <summary>Convert floating point 3-vector (with color ranges in [0,1]) to a packed int.</summary> 
        static unsigned int Vector3ToPackedInt( const cVector3f& color );

        /// <summary>Convert floating point 4-vector (with color ranges in [0,1] and alpha range [0,1]) to a packed int.</summary> 
        static unsigned int Vector4ToPackedInt( const cVector4f& color );

        /// <summary>Convert RGBA to a packed int.</summary> 
        static unsigned int RGBAToPackedInt( unsigned char r, unsigned char g, unsigned char b, unsigned char a );

        /// <summary>Convert a packed int to unpacked RGB int values.</summary> 
        static void UnpackColor( unsigned int color, unsigned char &r, unsigned char &g, unsigned char &b );

        /// <summary>Convert a packed int to unpacked RGBA int values.</summary> 
        static void UnpackColor( unsigned int color, unsigned char &r, unsigned char &g, unsigned char &b, 
            unsigned char &a );

        /// <summary>Convert a packed int into unpacked normalized [0-1] RGB colors.</summary>
        static void UnpackNormalizedColor( int color, float &r, float &g, float &b );

        /// <summary>Convert a packed int into unpacked normalized [0-1] RGBA colors.</summary>
        static void UnpackNormalizedColor( int color, float &r, float &g, float &b, float &a );

        /// <summary>Convert an HSV color to floating-point RGB.</summary>
        static void ConvertHsvToRgb( const sHSVColor& hsvColor, float& r, float& g, float& b );

        /// <summary>Convert a floating-point RGB color to floating-point HSV.</summary>
        static sHSVColor ConvertRgbToHsv( float r, float g, float b );

        /// <summary> Convert a packed int to a string of "AABBGGRR".</summary>
        static std::wstring PackedIntToString( unsigned int color );
    };
}

