//======================================================================================================
// Copyright 2019, NaturalPoint Inc.
//======================================================================================================
#pragma once

// System includes
#include <functional>
#include <thread>

// Local includes
#include "Core/BuildConfig.h"
#include "Core/DebugSystem.h"
#include "Core/ThreadLock.h"
#include "Core/Timer.h"

#pragma warning( push )
#pragma warning( disable: 4251 ) // Warning about "needs to have dll-interface" on STL types

namespace Core
{
    /// <summary>
    /// A threaded job queue for carrying out tasks that are to be delayed by at least a certain amount.
    /// </summary>
    template<typename T>
    class CORE_API cJobQueue
    {
    public:
        struct sJobItem
        {
            sJobItem( T* data, std::function<void( T* )> function ) : mData( data ), mFunction( function ) { }

            T* mData;
            std::function<void( T* )> mFunction;
        };

        cJobQueue() = default;

        virtual ~cJobQueue()
        {
            mThreadEnabled = false;

            // wait for it to finish
            try
            {
                if( mThread )
                {
                    mThread->join();
                }
            }
            catch( const std::exception& )
            {
                // ok - thread has already terminated
            }
        }

        /// <summary>Add a job to the queue. Thread-safe.</summary>
        void AddJob( double delayTime /* in ms */, T* data, std::function<void( T* )> func )
        {
            sJobItem newItem( data, func );
            ASSERT( delayTime < 10000.0 ); // Keep delay times to something semi-reasonable.
            double expiryTime = mTimer.Elapsed() + delayTime / 1000.0;

            // Only create the thread if/when it is needed.
            if( mThread == nullptr )
            {
                mThread = std::make_unique<std::thread>( &cJobQueue::ProcessJobs, this );
            }

            mQueueMutex.Lock();
            mJobQueue.insert( std::make_pair( expiryTime, newItem ) );
            mQueueMutex.Unlock();
        }

    private:
        Core::cTimer mTimer;
        cThreadLock mQueueMutex;
        std::unique_ptr<std::thread> mThread;
        std::map<double, sJobItem> mJobQueue;
        bool mThreadEnabled = false;

        void ProcessJobs()
        {
            mThreadEnabled = true;
            std::vector<sJobItem> jobsToExecute;
            unsigned int    sleepTime = 10; // ms

            while( mThreadEnabled || !mJobQueue.empty() )
            {
                double currentTime = mTimer.Elapsed();

                // Pull out any jobs that have timed out.
                {
                    mQueueMutex.Lock();
                    if( !mJobQueue.empty() )
                    {
                        auto endIt = mJobQueue.upper_bound( currentTime );

                        for( auto jobIt = mJobQueue.begin(); jobIt != endIt; ++jobIt )
                        {
                            jobsToExecute.push_back( jobIt->second );
                        }
                        mJobQueue.erase( mJobQueue.begin(), endIt );
                    }

                    if( mJobQueue.empty() )
                    {
                        sleepTime = 200;
                    }
                    else
                    {
                        sleepTime = 20;
                    }
                    mQueueMutex.Unlock();
                }

                // Process the jobs that are ready.
                for( auto it = jobsToExecute.begin(); it != jobsToExecute.end(); ++it )
                {
                    it->mFunction( it->mData );
                }
                jobsToExecute.clear();

                // Go to sleep for specified time.
                std::this_thread::sleep_for( std::chrono::milliseconds( sleepTime ) );
            }
        }
    };
}

#pragma warning( pop )
