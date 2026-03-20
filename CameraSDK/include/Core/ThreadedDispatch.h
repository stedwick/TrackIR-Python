//======================================================================================================
// Copyright 2015, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <vector>

#include "Core/BuildConfig.h"
#include "Core/Event.h"
#include "ThreadHandle.h"

#pragma warning( push )
#pragma warning( disable: 4251 ) // Warning about "needs to have dll-interface" on STL types

namespace Core
{
    /// <summary>Override cIThreadingTask to store pertinent information regarding
    ///    the threaded task to be performed.</summary>
    class CORE_API cIThreadingTask
    {
    public:
        virtual ~cIThreadingTask() { }
    };

    /// <summary>Inherit cThreadedDispatch to add threaded dispatching to your class.</summary>
    class CORE_API cThreadedDispatch : public cThreadProc
    {
    public:
        cThreadedDispatch();
        ~cThreadedDispatch();

        /// <summary>Dispatch a task to be scheduled and performed by one of the worker threads.</summary>
        void Dispatch( cIThreadingTask & task );

        /// <summary>Override PerformThreadedTask to receive tasks to perform. This
        ///    method will be called in parallel from a number of worker threads.</summary>
        virtual void PerformThreadedTask( cIThreadingTask & task ) = 0;
        
        /// <summary>Parallelism defaults to the number of cores on the processor. Override
        ///   this to limit the number of task running in parallel.</summary>
        virtual int MaximumThreadCount() const;

        /// <summary>Call WaitForThreadedCompletion() to block until all dispatched tasks are complete.</summary>
        void WaitForThreadedCompletion() const;

        /// <summary>Query IsAllThreadsIdle() to determine if all tasks are complete without blocking.</summary>
        bool IsAllThreadsIdle() const;

        /// <summary>ThreadProc method contains the default worker thread implementation for
        ///    performing tasks in parallel.  Needs to be public for thread entry point to call in.</summary>
        void ThreadProc( cThreadHandle & handle ) override;

    private:
        void PrepareThreading();
        void ShutdownThreads();

        cThreadHandle* AvailableThread();
        cThreadHandle* NextAvailableThread();

        void Dispatch( cIThreadingTask* task, cThreadHandle* thread );

        std::vector<cThreadHandle*> mThreadList;

        cEvent mThreadedDispatcherSignal;

        int mTargetThreadCount;
    };    
}

#pragma warning( pop )
