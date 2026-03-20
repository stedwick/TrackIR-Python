//======================================================================================================
// Copyright 2013, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <string>

#include "Core/BuildConfig.h"

#if defined(__unix__)
    #if !defined(__PLATFORM__LINUX__)
        #define __PLATFORM__LINUX__
    #endif
#endif

#ifdef __PLATFORM__LINUX__
#define sprintf_s snprintf
// #define byte unsigned char
#endif

namespace Core
{
	///<summary>Query the number of cores available on this machine.</summary>
    CORE_API int CoreCount();

    CORE_API void SleepMilliseconds( int milliseconds );

    class CORE_API cUniqueMachineID
    {
    public:
        cUniqueMachineID();
        ~cUniqueMachineID() = default;

        std::string MachineID() const;

    private:
        unsigned short mMachineIDLow = 0;
        unsigned short mMachineIDHigh = 0;

        void Initialize();
        unsigned int Hash( unsigned int value1, unsigned int value2 );
    };
}


