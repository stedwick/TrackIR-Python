//======================================================================================================
// Copyright 2010, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include "cameracommonglobals.h"
#include "cameralibraryglobals.h"

class CLAPI ThreadInfo
{
public:
    ThreadInfo();
    ~ThreadInfo() = default;

#ifdef WIN32
    void StartThread( void *threadProc, void *param );
#else
    void StartThread( void( *proc )( void* ), void *param );
#endif
    void StartShutdown();
    void StopThread();

    void SetNormalPropriority();
    void SetHighPropriority();
    void SetHighestPropriority();
    void SetTimeCritical();

    bool IsThreadRunning() const { return mThreadRunning; }
    bool IsSteadyState() const { return !mShuttingDown; }
    bool IsShuttingDown() const { return (mShuttingDown || !mThreadRunning); }


    void* mParam;
    bool mThreadRunning;

private:
    bool mShuttingDown;

#ifdef WIN32
    void* mThreadProc;
#else
    void( *mThreadProc )( void* );
#endif    
    void* mThreadHandle;
    unsigned long mThreadID;
};
