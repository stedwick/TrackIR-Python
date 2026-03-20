//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once

// Local includes
#include "Core/DebugSystem.h"

namespace Core
{
    /// <summary>
    /// The cSingleton class is a template class for creating singleton objects.
    /// When the static Instance() method is called for the first time, the singleton
    /// object is created. Every sequential call returns a reference to this instance.
    /// </summary>
    template <typename T>
    class cSingleton
    {
    public:
        static bool IsActive() { return ( sInstance != nullptr ); }

        /// <summary>Get a reference to the singleton.</summary>
        static T& Ref()
        {
            ASSERT( sInstance != nullptr );

            return *sInstance;
        }

        /// <summary>Get a const reference to the singleton.</summary>
        static const T& ConstRef()
        {
            return Ref();
        }

        /// <summary>Get a pointer to the singleton.</summary>
        static T* Ptr()
        {
            ASSERT( sInstance != nullptr );

            return sInstance;
        }

        static void CreateInstance()
        {
            if( sInstance == nullptr )
            {
                sInstance = new T;
            }
        }

        static void DestroyInstance()
        {
            delete sInstance;
            sInstance = nullptr;
        }

    protected:
        // Shield the constructor and destructor to prevent outside sources
        // from creating or destroying a cSingleton instance.

        // Default constructor.
        cSingleton() = default;

        // Destructor.
        virtual ~cSingleton() = default;

    private:
        // Copy constructor.
        cSingleton( const cSingleton &source ) = delete;

        static T *sInstance;    //!< singleton class instance ==--
    };
}

// Static class member initialization.
template <typename T> T* Core::cSingleton<T>::sInstance = nullptr;
