/*
    Based on:

    md5.h and md5.c
    reference implementation of RFC 1321

    Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
    rights reserved.

    License to copy and use this software is granted provided that it
    is identified as the "RSA Data Security, Inc. cMD5 Message-Digest
    Algorithm" in all material mentioning or referencing this software
    or this function.

    License is also granted to make and use derivative works provided
    that such works are identified as "derived from the RSA Data
    Security, Inc. cMD5 Message-Digest Algorithm" in all material
    mentioning or referencing the derived work.

    RSA Data Security, Inc. makes no representations concerning either
    the merchantability of this software or the suitability of this
    software for any particular purpose. It is provided "as is"
    without express or implied warranty of any kind.

    These notices must be retained in any copies of any part of this
    documentation and/or software.
*/

#pragma once

#include <string>

// A small class for calculating cMD5 hashes of strings or byte arrays
//
// usage: 1) feed it blocks of uchars with update()
//      2) finalize()
//      3) get hexdigest() string
//      or
//      cMD5(std::string).hexdigest()

namespace Core
{
    class cUID;

    class cMD5
    {
    public:
        typedef unsigned int size_type; // must be 32bit

        cMD5();
        cMD5( const std::string& text );

        void Update( const unsigned char *input, size_type length );
        void Update( const char *buf, size_type length );
        cMD5& Finalize();
        std::string HexDigest() const;

        cUID Hash() const;

    private:
        void Init();

        typedef unsigned char uint1; //  8bit
        typedef unsigned int uint4;  // 32bit

        static const unsigned int kBlockSize = 64;

        void Transform( const uint1 block[kBlockSize] );
        static void Decode( uint4 output[], const uint1 input[], size_type len );
        static void Encode( uint1 output[], const uint4 input[], size_type len );

        bool mFinalized;
        uint1 mBuffer[kBlockSize]; // bytes that didn't fit in last 64 byte chunk
        uint4 mCount[2];   // 64bit counter for number of bits (lo, hi)
        uint4 mState[4];   // digest so far
        uint1 mDigest[16]; // the result

        // Low level logic operations
        static inline uint4 F( uint4 x, uint4 y, uint4 z );
        static inline uint4 G( uint4 x, uint4 y, uint4 z );
        static inline uint4 H( uint4 x, uint4 y, uint4 z );
        static inline uint4 I( uint4 x, uint4 y, uint4 z );
        static inline uint4 RotateLeft( uint4 x, int n );
        static inline void FF( uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac );
        static inline void GG( uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac );
        static inline void HH( uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac );
        static inline void II( uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac );
    };
}
