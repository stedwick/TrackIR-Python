//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
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
    /// <summary>
    /// A platform-neutral thread. Derive from this class and override the Process() method to define functionality
    /// that should run each time the thread wakes up. This class manages a separate thread, but is not thread-safe
    /// so methods on this class should be called from a single thread for each cThread instance.
    /// </summary>
    class CORE_API cThread
    {
    public:
        enum eThreadPriority
        {
            Lowest,
            BelowNormal,
            Normal,
            AboveNormal,
            Highest,
            TimeCritical
        };

        cThread( const std::string& name = "" );
        virtual ~cThread();

        /// <summary>Set the thread priority. May be called at any time.</summary>
        void SetPriority( eThreadPriority priority );

        /// <summary>Set the amount of time to wait between calls to Process() when running continuously.</summary>
        void SetLoopWait( int milliseconds );

        /// <summary>Run the processing method continuously until Pause is called or the thread is
        /// destroyed.</summary>
        void Run();

        /// <summary>Run the processing method once (asynchronously) on this thread, then suspend the thread.
        /// The thread will remain active in between calls to RunOnce(), but be in a suspended state. To terminate
        /// the thread, call Stop().</summary>
        void RunOnce();

        /// <summary>Pause continuous running of the thread, placing the thread in a suspended state. Run() or 
        /// RunOnce() can be called after this.</summary>
        void Pause( bool waitForCompletion = true );

        /// <summary>Stop the thread (i.e. kill the thread). This effectively destroys the internal thread and calling
        /// Run() or RunOnce() can be called after this to recreate and restart the thread.</summary>
        void Stop();

        /// <summary>Returns true if the thread is running continuously (i.e. not in RunOnce mode).</summary>
        bool IsRunning() const { return mRunContinuous; }

        /// <summary>Get the current thread priority.</summary>
        eThreadPriority Priority() const { return mPriority; }

        /// <summary>Get the current wait time between Process() calls when running.</summary>
        int LoopWait() const { return mLoopWait; }

    protected:
        /// <summary>
        /// Override this method to define the functionality that should run each time the thread wakes up and runs
        /// either through continuous running, or via a call to RunOnce().
        /// </summary>
        virtual void Process() = 0;

    private:
        static const int kMaxLoopWait; // In milliseconds

        bool mShuttingDown;
        bool mActive;
        bool mPausing;
        bool mRunContinuous;
        int mLoopWait;
        eThreadPriority mPriority;

        void *mThreadHandle;
        unsigned int mThreadID;
        void *mEvent;

        std::string mName;

        void SetName( const std::string& name );
        void SetPriority();
        void Start();
        void SetEvent();
        void ResetEvent();

        static int ThreadProc( void * owner );
    };

    /// <summary>A collection of useful thread-related functions.</summary>
    class CORE_API cThreadHelpers
    {
    public:
        /// <summary>Relinquish the CPU and wait for the given time on the caller's thread.</summary>
        static void Sleep( int milliseconds );

        /// <summary>Returns a unique ID for the calling thread.</summary>
        static unsigned int ThreadID();
    };
}

#pragma warning( pop )
