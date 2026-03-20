//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <iosfwd>
#include <functional>

#include "Core/BuildConfig.h"

#ifdef __PLATFORM__LINUX__
#include "Core/Platform.h"
#endif

namespace Core
{
    /// <summary>
    /// A platform-neutral 128-bit universal identifier. It is essentially guaranteed to never
    /// generate the same ID twice.
    /// </summary>
    class CORE_API cUID
    {
    public:
        using uint64 = unsigned long long int;

        /// <summary>
        /// Create a default UID. In order to create a UID that has a valid unique identifier you
        /// must call Generate().
        /// </summary>
        cUID() : mHighBits( 0 ), mLowBits( 0 ) { }

        cUID( uint64 high, uint64 low ) : mHighBits( high ), mLowBits( low ) { }

        /// <summary>
        /// Set the value of the UID from two long integer values. It is up to the caller to ensure that
        /// the resulting UID is unique.
        /// </summary>
        void SetValue( uint64 highBits, uint64 lowBits )
        {
            mHighBits = highBits;
            mLowBits = lowBits;
        }

        /// <summary>Get the low 64 bits of the UID.</summary>
        uint64 LowBits() const
        {
            return mLowBits;
        }

        /// <summary>Get the high 64 bits of the UID.</summary>
        uint64 HighBits() const
        {
            return mHighBits;
        }

        /// <summary>Returns true if the ID is valid (i.e. not equal to kInvalid).</summary>
        bool Valid() const
        {
            return !( mHighBits == 0 && mLowBits == 0 );
        }

        /// <summary>Generate a new UID value.</summary>
        static cUID Generate();

        //==============================================================================================
        // Comparison operators
        //==============================================================================================

        bool operator<( const cUID & rhs ) const
        {
            return ( ( mHighBits < rhs.mHighBits ) ? true : ( mHighBits == rhs.mHighBits ? ( mLowBits < rhs.mLowBits ) : false ) );
        }
        bool operator<=( const cUID & rhs ) const
        {
            return ( ( mHighBits < rhs.mHighBits ) ? true : ( mHighBits == rhs.mHighBits ? ( mLowBits <= rhs.mLowBits ) : false ) );
        }

        bool operator>( const cUID & rhs ) const
        {
            return !( *this <= rhs );
        }
        bool operator>=( const cUID & rhs ) const
        {
            return !( *this < rhs );
        }

        cUID operator^( const cUID &rhs ) const
        {
            return cUID( mHighBits ^ rhs.mHighBits, mLowBits ^ rhs.mLowBits );
        }

        // Inline these for performance.
        bool operator==( const cUID & rhs ) const
        {
            return ( ( mHighBits == rhs.mHighBits ) && ( mLowBits == rhs.mLowBits ) );
        }

        bool operator!=( const cUID & rhs ) const
        {
            return ( ( mHighBits != rhs.mHighBits ) || ( mLowBits != rhs.mLowBits ) );
        }

        explicit operator bool() const
        {
            return Valid();
        }

        //==============================================================================================
        // Constants
        //==============================================================================================

        static const cUID kInvalid;

    private:
        uint64 mHighBits;
        uint64 mLowBits;
    };

    //======================================================================================================
    // Operators
    //======================================================================================================
#if !defined(__PLATFORM__LINUX__) // The following methods are problematic to compile under Linux

    CORE_API std::ostream& operator<<( std::ostream& os, const Core::cUID& id );
    CORE_API std::istream& operator>>( std::istream& is, Core::cUID& id );

    CORE_API std::wostream& operator<<( std::wostream& os, const Core::cUID& id );
    CORE_API std::wistream& operator>>( std::wistream& is, Core::cUID& id );

#endif // !defined(__PLATFORM__LINUX__)
}

#if _MSC_VER > 1600 || defined(__PLATFORM__LINUX)
namespace std
{
    // Hash template specialization for cUID. Allows it to be used as a key for things like std::unordered_set.
    template<> struct hash<Core::cUID>
    {
        size_t operator()( const Core::cUID& s ) const
        {
            size_t h1( std::hash<unsigned long long>{}( s.HighBits() ) );
            size_t h2( std::hash<unsigned long long>{}( s.LowBits() ) );
            return ( h1 ^ ( h2 << 1 ) );
        }
    };
}
#endif

