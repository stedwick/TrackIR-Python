//======================================================================================================-----
// Copyright 2019, NaturalPoint Inc.
//======================================================================================================-----
#pragma once

// System includes
#include <string>
#include <vector>
#include <memory>
#include <mutex>

#include "Core/Filename.h"
#include "Core/BuildConfig.h"

namespace std
{
    class mutex;
}

namespace Core
{
    struct sStackFrame;

    /// <summary>
    /// Class to handle high performance logging, including the ability to output a full call stack as part of
    /// any message.
    /// </summary>
    class CORE_API cHighPerformanceLogger
    {
    public:
        cHighPerformanceLogger( const Core::cFilename& filename, bool autoAddNewline = true, bool removeExistingFile = true,
            size_t maxMemorySize = kLogMemoryDefaultSize );
        ~cHighPerformanceLogger();

        /// <summary>Output a pre-formatted message.</summary>
        void Trace( const std::string& message );

        /// <summary>Output a formatted message.</summary>
        void Trace( const char* format, ... );

        /// <summary>Output a stack trace with a message.</summary>
        void StackTrace( const char* format, ... );

        /// <summary>Force a flush of the memory buffers to file. Otherwise, this happens automatically
        /// when the memory buffers fill up.</summary>
        void Flush();

    private:
        static const size_t kLogMemoryMinimumSize = 4096;
        static const size_t kLogMemoryDefaultSize = 1024*1024;

        std::unique_ptr<std::recursive_mutex> mLock;
        std::string mFilename;
        std::string mMemoryBlock;
        size_t mMaxMemorySize;
        bool mAutoAddNewline;

        std::vector<sStackFrame> CurrentStack();
        void LogString( const char* str );

        static void* sProcessHandle;
        static unsigned int sInstanceCount;
    };
}
