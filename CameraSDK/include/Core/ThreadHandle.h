//======================================================================================================
// Copyright 2015, NaturalPoint Inc.
//======================================================================================================
#pragma once

// System includes
#include <string>

// Local includes
#include "Core/BuildConfig.h"
#include "Core/Event.h"

namespace Core
{
    class cThreadProc;

    /// <summary>
    /// A platform-neutral thread handler.
    /// </summary>
    class CORE_API cThreadHandle
    {
    public:
        cThreadHandle();
        virtual ~cThreadHandle() = default;
        
        void StartThread( cThreadProc* threadProc, void* param = nullptr );
        void StopThread();

        enum eThreadPriority
        {
            Lowest,
            BelowNormal,
            Normal,
            AboveNormal,
            Highest,
            TimeCritical
        };

        /// <summary>Set the thread priority. May be called at any time.</summary>
        void SetPriority( eThreadPriority priority );

        bool IsThreadRunning() const;
        bool IsSteadyState() const;

        //== internal for thread proc access from entry point ==--
        cThreadProc& ThreadProcObject();
        void SetComplete();

        const Core::cEvent& ThreadEvent() const;

        void SetUserData( void * param ) { mParam = param; }
        void* UserData() const { return mParam; }
    private:
        Core::cThreadProc* mThreadProcObject;
#ifdef WIN32
        void* mThreadProc;
#else
        void (*mThreadProc)(void*);
#endif    
        void* mThreadHandle;

        unsigned long mThreadID;
        mutable void* mParam;
        bool mThreadRunning;
        bool mShuttingDown;
        
        void SetPriority();
        eThreadPriority mPriority;

        Core::cEvent mThreadEvent;
    };

    class CORE_API cThreadProc
    {
    public:
        cThreadProc() = default;
        virtual ~cThreadProc() = default;

        /// <summary>
        ///   Inherit this class and override this function to provide a entry point for a new thread.
        /// </summary>

        virtual void ThreadProc( cThreadHandle& threadHandle ) = 0;
    };
}
