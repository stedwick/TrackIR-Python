//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once

// Local includes
#include "Core/BuildConfig.h"

namespace Core
{
    /// <summary>
    /// A collection of static methods for debug output, failure detection, and logging.
    /// </summary>
    class CORE_API cDebugSystem
    {
    public:
        enum eDebugSystemName
        {
            General = 0,
            Camera,
            CameraManager,
            InputManager,
            Input,
            Frame,
            Thread,
            Network,
            USB,
            Pipeline,
            UI,
            Skeleton,
            Calibration,
            RigidBody,
            PluginDevice,
            DebugSystemCount
        };

        static bool SystemVisibility[DebugSystemCount];

        static void Failure( bool failure, const char *file, int line, const char *fmt, ... );
        static void ReportDebug( eDebugSystemName system, const char *fmt, ... );
        static void ReportDebug( eDebugSystemName system, const wchar_t *fmt, ... );
        static void ReportDebug( const char *fmt, ... );
        static void ReportDebug( const wchar_t *fmt, ... );

        static void ReportLog( eDebugSystemName system, const char *filename, const char *fmt, ... );
        static void Assert( bool assertion, const char *file, int line );

        static const char* SystemName( eDebugSystemName system );
    };
}

#ifdef CORE_DEBUGSYSTEM

#ifndef OUTPUT
#define OUTPUT(...)     Core::cDebugSystem::ReportDebug( __VA_ARGS__ )
#endif

#ifndef FAILURE
#define FAILURE(x,...)  Core::cDebugSystem::Failure( x, __FILE__, __LINE__, __VA_ARGS__ )
#endif

#ifndef LOGOUTPUT
#define LOGOUTPUT(x,y,...)  Core::cDebugSystem::ReportLog( x, y, __VA_ARGS__ )
#endif

#ifndef ASSERT
#define ASSERT(x)       Core::cDebugSystem::Assert( x, __FILE__, __LINE__ )
#endif

#ifndef VERIFY
#define VERIFY(x)       Core::cDebugSystem::Assert( x, __FILE__, __LINE__ )
#endif

#else

#ifndef OUTPUT
#define OUTPUT(...)    {}
#endif

#ifndef FAILURE
#define FAILURE(x,...) {}
#endif

#ifndef LOGOUTPUT
#define LOGOUTPUT(...) {}
#endif

#ifndef ASSERT
#define ASSERT(x)  {}
#endif

#ifndef VERIFY
#define VERIFY(x)  { x; }
#endif

#endif // CORE_DEBUGSYSTEM

