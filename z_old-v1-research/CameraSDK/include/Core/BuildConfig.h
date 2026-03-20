//======================================================================================================
// Copyright 2013, NaturalPoint Inc.
//======================================================================================================
#pragma once

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

#ifdef CORE_EXPORTS
    #define CORE_API OPTITRACK_EXPORT
#elif defined CORE_IMPORTS
    #define CORE_API OPTITRACK_IMPORT
#else
    #define CORE_API
#endif

//== Global System Selection

#ifdef _DEBUG           //=== DEBUG BUILD SETTINGS =====================================================

#define ENABLE_CORE_DEBUGSYSTEM   1 //=== Enable/Disable Debug System for debug builds
#define ENABLE_CORE_DEBUGLOGGING  0 //=== Enable/Disable Debug System output C:\\Users\\Public\\Documents\\DebugLog.txt

#else                   //=== RELEASE BUILD SETTINGS ===================================================

#define ENABLE_CORE_DEBUGSYSTEM   0 //=== Enable/Disable Debug System for debug builds
#define ENABLE_CORE_DEBUGLOGGING  0 //=== Enable/Disable Debug System output C:\\Users\\Public\\Documents\\DebugLog.txt

#endif

//== Enable Systems based on selection above

#if ENABLE_CORE_DEBUGSYSTEM==1
#define CORE_DEBUGSYSTEM
#else
#undef  CORE_DEBUGSYSTEM
#endif

#if ENABLE_CORE_DEBUGLOGGING==1
#define CORE_DEBUGLOGGING
#else
#undef  CORE_DEBUGLOGGING
#endif
