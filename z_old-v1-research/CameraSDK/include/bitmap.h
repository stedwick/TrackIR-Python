//======================================================================================================
// Copyright 2010, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include "cameracommonglobals.h"

#define GetRedCol(rgb)      (rgb&255)
#define GetGreenCol(rgb)    ((rgb>>8)&255)
#define GetBlueCol(rgb)     ((rgb>>16)&255)
#define GetAlphaCol(rgb)    ((rgb>>24)&255)

#define ABGR(a,b,g,r) (((a)<<24)|((b)<<16)|((g)<<8)|(r))

namespace CameraLibrary
{
    typedef int PIXEL;

    PIXEL Color( int r, int g, int b );

#define PIXELCOLOR(r,g,b) ((CameraLibrary::PIXEL)(((unsigned char)(r)|((int)((unsigned char)(g))<<8))|(((int)(unsigned char)(b))<<16)))
#define PIXELCOLORA(r,g,b,a) ((CameraLibrary::PIXEL)(((unsigned char)(r)|((int)((unsigned char)(g))<<8))|(((int)(unsigned char)(b))<<16))|(((int)(unsigned char)(a))<<24))

    class CLAPI Bitmap
    {
    public:
        enum ColorDepth
        {
            EightBit = 8,
            SixteenBit = 16,
            TwentyFourBit = 24,
            ThirtyTwoBit = 32
        };

        Bitmap( int pixelWidth, int pixelHeight, int byteSpan, ColorDepth cd, const unsigned char* bits = nullptr );
        virtual ~Bitmap();

        int PixelWidth() const;
        int PixelHeight() const;
        int ByteSpan() const;

        void HorizontalLine( int x, int y, int x2, PIXEL color );
        void HorizontalLineFrom8BitSource( int x, int y, int x2, unsigned char* buffer );
        void PutPixel( int X, int Y, PIXEL color );
        void PutPixelAlpha( int X, int Y, PIXEL color );
        void FillCircle( int X, int Y, int Radius, PIXEL Color = PIXELCOLOR( 255, 255, 255 ) );
        void Line( int X1, int Y1, int X2, int Y2, PIXEL Color = PIXELCOLOR( 255, 255, 255 ) );

        void Clear( unsigned char Intensity = 0 );
        void Print( int x, int y, const char* string, PIXEL foreColor = PIXELCOLOR( 255, 255, 255 ) );
        void PrintLarge( int x, int y, const char* string, PIXEL foreColor = PIXELCOLOR( 255, 255, 255 ) );
        void PrintMedium( int x, int y, const char* string, PIXEL foreColor = PIXELCOLOR( 255, 255, 255 ) );

        PIXEL GetPixel( int X, int Y ) const;

        // GUI uses this to draw.
        unsigned char* GetBits();

        int BufferSize() const;

        // Reset data to zero.
        void ClearBits();

        void Resize( Bitmap* source );

        void SetBuffer( unsigned char* buffer );

        // There are 8 bits / byte and the bytes / pixel is determined by the color depth.
        int GetBitsPerPixel() const;


    private:
        ColorDepth mColorDepth;
        int mBytesPerPixel;
        int mWidth;
        int mHeight;
        int mSpan;
        unsigned char* mFrameBuffer;
        int mBitArrayLength;
        bool mFreeBits;

        void CirclePointsFill( int cx, int cy, int x, int y, PIXEL BackColor );
        void HorizontalCharacterLine( int x, int y, int x2, PIXEL foreColor, unsigned char* buffer );
    };
}
