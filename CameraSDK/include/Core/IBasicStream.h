//======================================================================================================
// Copyright 2014, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include "Core/BuildConfig.h"

namespace Core
{
    /// <summary>
    /// Interface base class for any object that wishes to implement stream read/write operations.
    /// </summary>
    class CORE_API cIBasicStream
    {
    public:
        virtual ~cIBasicStream() = default;

        /// <summary>Returns the current position in the file stream.</summary>
        virtual unsigned long long Tell() const = 0;

        /// <summary>Seek to the requested (absolute) position in the stream.</summary>
        virtual bool Seek( unsigned long long pos ) = 0;

        /// <summary>Reports the total current size of the stream.</summary>
        virtual unsigned long long Size() const = 0;
    };
}
