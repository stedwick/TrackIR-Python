//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

// Added Thread Synchronization....2008-09-03............Douglas Beck

//! The CSingleton class is a template class for creating singleton objects.
/*!
When the static Instance() method is called for the first time, the singleton
object is created. Every sequential call returns a reference to this instance.
*/

#ifdef _DEBUG
// If this is defined, elements are includes to help with leak detection (i.e. to do explicit tear-down of
// all singleton objects and associated objects at shutdown).
//#define SINGLETON_LEAK_DETECTION
#endif

#ifdef SINGLETON_LEAK_DETECTION
#include <set>
#include <algorithm>
#endif

#include "cameralibraryglobals.h"
#include "Core/ThreadLock.h"

namespace CameraLibrary
{
#ifdef SINGLETON_LEAK_DETECTION
    class CLAPI SingletonBase
    {
    public:
        virtual ~SingletonBase() { }

        static void Add( SingletonBase *ptr )
        {
            sSingletons.insert( ptr );
        }
        static void Add( Core::cThreadLock*lock )
        {
            sLocks.insert( lock );
        }
        static void Remove( SingletonBase *ptr )
        {
            std::set<SingletonBase*>::iterator it = std::find( sSingletons.begin(), sSingletons.end(), ptr );
            if( it != sSingletons.end() )
            {
                sSingletons.erase( it );
            }
        }
        static void DeleteAll()
        {
            std::set<SingletonBase*>::reverse_iterator it;
            for( it = sSingletons.rbegin(); it != sSingletons.rend(); ++it )
            {
                _CrtCheckMemory();
                delete *it;
            }
            sSingletons.clear();

            std::set<Core::cThreadLock*>::reverse_iterator lockIt;
            for( lockIt = sLocks.rbegin(); lockIt != sLocks.rend(); ++lockIt )
            {
                delete *lockIt;
            }
            sLocks.clear();
        }
    private:
        static std::set<SingletonBase*> sSingletons;
        static std::set<Core::cThreadLock*> sLocks;
    };

    template <typename T>
    class Singleton : public SingletonBase
#else
    template <typename T>
    class Singleton
#endif
    {
    public:
        static bool IsActive()
        {
            return ( m_instance != 0 );
        }
        static T* Ptr()
        {
            if( m_instance == 0 )
            {
                m_Lock->Lock();
                if( m_instance == 0 )
                {
                    m_instance = new T;
#ifdef SINGLETON_LEAK_DETECTION
                    SingletonBase::Add( m_Lock );
#endif
                }
                m_Lock->Unlock();
            }

            //FAILURE(m_instance == 0, "Singleton Not Initialized");

            return m_instance;
        }

        static T& X()
        {
            if( m_instance == 0 )
            {
                if( m_Lock == 0 )
                {
                    m_Lock = new Core::cThreadLock();
                }

                m_Lock->Lock();
                if( m_instance == 0 )
                {
                    // If you are crashing here, it's likely that your solution's runtime
                    // does not match the runtime compiled against in the Camera SDK library you're
                    // linking into your solution.
                    //
                    // link the following to your project:
                    //
                    //     cameralibrary.lib    into your solution when you are using the static runtime (/MT)
                    //     cameralibrarydrt.lib into your solution when you are using the dynamic runtime (/MD)

                    m_instance = new T;
#ifdef SINGLETON_LEAK_DETECTION
                    SingletonBase::Add( m_Lock );
#endif
                }
                m_Lock->Unlock();
            }

            return *m_instance;
        }

        static void RefreshInstance()
        {
            m_Lock->Lock();
#ifdef SINGLETON_LEAK_DETECTION
            SingletonBase::Remove( m_instance );
#endif
            delete m_instance;
            m_instance = new T;
            m_Lock->Unlock();
        }

        static void DestroyInstance()
        {
            m_Lock->Lock();
            if( m_instance )
            {
#ifdef SINGLETON_LEAK_DETECTION
                SingletonBase::Remove( m_instance );
#endif
                delete m_instance;
                m_instance = 0;
            }
            m_Lock->Unlock();
        }

    protected:
        // shield the constructor and destructor to prevent outside sources
        // from creating or destroying a CSingleton instance.

        // Default constructor.
        Singleton()
        {
#ifdef SINGLETON_LEAK_DETECTION
            SingletonBase::Add( this );
#endif
        }

        //! Destructor.
        virtual ~Singleton() { }

    private:
        //! Copy constructor.
        Singleton( const Singleton& source ) { }
        static Core::cThreadLock* m_Lock;    // Thread synchronization
        static T* m_instance;                // singleton class instance
    };
}

//! static class member initialization.
template <typename T> T* CameraLibrary::Singleton<T>::m_instance = 0;
template <typename T> Core::cThreadLock* CameraLibrary::Singleton<T>::m_Lock = new Core::cThreadLock();
