//======================================================================================================
// Copyright 2023, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include "Core/BuildConfig.h"

namespace Core
{
    /// <summary> A thin wrapper around std::recursive_mutex, without actually using recursive_mutex.
    /// That's because NMotive uses clr and can't compile code using std::recursive_mutex directly.
    /// Ensure single-thread entry into blocks of code surrounded by Lock()/TryLock() and Unlock() pairs.
    /// Every call to Lock()/TryLock() must be matched with a call to Unlock()
    /// </summary>
    class CORE_API cThreadLock
    {
    public:
        cThreadLock();
        ~cThreadLock();

    	/// <summary>Try to acquire a lock. Declared const so it can be used in const methods.</summary>
    	/// <returns>True if the lock was engaged.</returns>
        bool TryLock() const;

		/// <summary>Acquire a lock. Declared const so it can be used in const methods.</summary>
        void Lock() const;

		/// <summary>
		/// Release the lock. The lock must have been previously acquired with a call to Lock() or a
		/// successful call to TryLock(). Declared const so it can be used in const methods.
		/// </summary>
        void Unlock() const;

    private:

        mutable void *mLock;

        // Disallow copy construction and assignment;
        cThreadLock( const cThreadLock& other ) = delete;
        cThreadLock& operator=( const cThreadLock& other ) = delete;
    };
}