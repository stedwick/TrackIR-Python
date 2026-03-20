//======================================================================================================
// Copyright 2010, NaturalPoint Inc.
//======================================================================================================
#pragma once

//======================================================================================================
//== GLOBAL DEFINITIONS AND SETTINGS
//======================================================================================================
//== Static & Dynamic Linking Preprocessor Defines
#if defined(_MSC_VER)
    //  Microsoft 
    #define OPTITRACK_EXPORT __declspec(dllexport)
    #define OPTITRACK_IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    //  GCC
    #define OPTITRACK_EXPORT __attribute__((visibility("default")))
    #define OPTITRACK_IMPORT
#else
    //  do nothing and hope for the best?
    #define OPTITRACK_EXPORT
    #define OPTITRACK_IMPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif

#ifdef CAMERALIBRARY_EXPORTS
    #define CLAPI OPTITRACK_EXPORT
#elif defined CAMERALIBRARY_IMPORTS
    #define CLAPI OPTITRACK_IMPORT
#else
    #define CLAPI
#endif

//======================================================================================================
//== GLOBAL SYSTEM SELECTION

#if defined(_DEBUG) // DEBUG BUILD SETTINGS

#define ENABLE_DEBUGSYSTEM   1 // Enable/Disable Debug System for debug builds
#define ENABLE_DEBUGLOGGING  0 // Enable/Disable Debug System output c:\DebugLog.txt

#else // RELEASE BUILD SETTINGS

#define ENABLE_DEBUGSYSTEM   0 // Enable/Disable Debug System for debug builds
#define ENABLE_DEBUGLOGGING  0 // Enable/Disable Debug System output c:\DebugLog.txt

#endif


//======================================================================================================
//== Cross-Platform Glue

#if defined(__unix__)
#if !defined(__PLATFORM__LINUX__)
#define __PLATFORM__LINUX__
#endif
#endif

#ifdef __PLATFORM__LINUX__
#define sprintf_s snprintf
// #define byte unsigned char
#endif

//======================================================================================================
//== Application-Wide Global Values

namespace Core
{
    const int kMax3DMarkers = 2048;
    const double kPI        = 3.141592654;
}

//======================================================================================================
//== Enable Systems based on selection above

#if ENABLE_DEBUGSYSTEM==1
#define DEBUGSYSTEM
#else
#undef  DEBUGSYSTEM
#endif

#if ENABLE_DEBUGLOGGING==1
#define DEBUGLOGGING
#else
#undef  DEBUGLOGGING
#endif

#include "Core/DebugSystem.h"
