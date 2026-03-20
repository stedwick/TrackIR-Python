//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once

// System includes
#include <string>

// Local includes
#include "Core/BuildConfig.h"

namespace Core
{
    using FrameIndex = int;

    // A floating point frame number that can represent values in-between frames.
    using cFractionalFrame = double;

    /// <summary>Represents a relative time.</summary>
    class CORE_API cTime
    {
    public:
        cTime() = default;
        cTime( double seconds );
        cTime( time_t seconds, int milliseconds );
        cTime( double hours, double minutes, double seconds );

        /// <summary>Set the full time based on a number of seconds.</summary>
        void SetInSeconds( double seconds );

        /// <summary>Set the time based on real time measures.</summary>
        void SetHoursMinutesSeconds( double hours, double minutes, double seconds );

        /// <summary>Time in seconds only (i.e. hours, minutes, etc. all converted into seconds)</summary>
        double InSeconds() const;

        /// <summary>Calculate the average time between this and the given time.</summary>
        cTime Average( const cTime& tm ) const;

        /// <summary>Returns true if the contained time is valid (not invalid).</summary>
        bool Valid() const { return ( *this != kInvalid ); }

        /// <summary>Convert the internal time value to individual time components.</summary>
        void DateTime( int& year, int& month, int& dayOfMonth, int& hour, int& minute, int& second, int& millisecond ) const;

        /// <summary>Generate a string that contains this date in human-readable format.</summary>
        std::wstring DateString() const;

        /// <summary>Generate a string that contains this date and time in human-readable format.</summary>
        std::wstring DateTimeString( bool includeMilliseconds = false, wchar_t timeDelimiter = L'.' ) const;

        /// <summary>Generate a string that contains this date and time in human-readable format.</summary>
        std::wstring TimeString( bool includeMilliseconds = false, wchar_t timeDelimiter = L'.' ) const;

        /// <summary>Current local time.</summary>
        static cTime CurrentDateTime();

        //==============================================================================================
        // Mathematical and assignment operators
        //==============================================================================================

        const cTime operator+( const cTime & rhs ) const { return cTime( mTime + rhs.mTime, mMilliseconds + rhs.mMilliseconds ); }
        cTime& operator+=( const cTime& rhs );
        const cTime operator-( const cTime & rhs ) const;
        cTime& operator-=( const cTime& rhs );
        const cTime operator*( const cTime& rhs ) const { return cTime( mTime * rhs.mTime, mMilliseconds * rhs.mMilliseconds ); }
        cTime& operator*=( const cTime& rhs );

        //==============================================================================================
        // Comparison operators
        //==============================================================================================

        bool operator==( const cTime & tm ) const { return ( mTime == tm.mTime && mMilliseconds == tm.mMilliseconds ); }
        bool operator!=( const cTime & tm ) const { return ( mTime != tm.mTime || mMilliseconds != tm.mMilliseconds ); }
        bool operator<( const cTime & tm ) const { return ( mTime < tm.mTime || ( mTime == tm.mTime && mMilliseconds < tm.mMilliseconds ) ); }
        bool operator<=( const cTime & tm ) const { return ( *this == tm ) || ( *this < tm ); }
        bool operator>( const cTime & tm ) const { return ( mTime > tm.mTime || ( mTime == tm.mTime && mMilliseconds > tm.mMilliseconds ) ); }
        bool operator>=( const cTime & tm ) const { return ( *this == tm ) || ( *this > tm ); }

        //==============================================================================================
        // Helper constants
        //==============================================================================================
        
        static const cTime kZero;
        static const cTime kPositiveInfinity;
        static const cTime kNegativeInfinity;
        static const cTime kInvalid; // A time value that can be considered "invalid"

    private:
        time_t mTime = 0;
        int mMilliseconds = 0;

        void BalanceMilliseconds();
    };
}
