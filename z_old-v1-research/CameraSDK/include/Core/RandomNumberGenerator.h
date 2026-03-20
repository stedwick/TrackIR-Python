//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include "Core/BuildConfig.h"

namespace Core
{
    class CORE_API cRandomNumberGenerator
    {
    public:
        cRandomNumberGenerator( int seed = 0 );

        /// <summary>Set the seed value to use.</summary>
        void SetSeed( int seed );

        /// <summary>Generate an integer within the full integer range.</summary>
        int GenerateInt() const;

        /// <summary>Generate a random 32-bit unsigned integer.</summary>
        unsigned int GenerateUnsignedInt() const;

        /// <summary>Generate a random 32-bit unsigned integer within the given range (inclusive).</summary>
        unsigned int GenerateUnsignedInt( unsigned int low, unsigned int high ) const;

        /// <summary>Fills the given array with random values.</summary>
        void GenerateVector( unsigned int v[], unsigned int size ) const;

        /// <summary>Fills the given buffer with random values.</summary>
        void GenerateBuffer( unsigned char* p, unsigned int n ) const;
    };
}
