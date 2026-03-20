//======================================================================================================
// Copyright 2025, NaturalPoint Inc.
//======================================================================================================
#pragma once

// System includes
#include <string>

// Local includes
#include "Core/BuildConfig.h"

#pragma warning( push )
#pragma warning( disable: 4251 ) // Warning about "needs to have dll-interface" on STL types

namespace Core
{
    class CORE_API cBuildStamp
    {
    public:
        static const std::wstring kDateTimeString;

        static const int kYear;
        static const int kMonth;
        static const int kDay;
        static const int kHour;
        static const int kMinute;
        static const int kSecond;

        static const int kChangelist;
        static const std::string kHash;
    };
}

#define CORE_CHANGELIST_STRING "0"

#pragma warning( pop )
