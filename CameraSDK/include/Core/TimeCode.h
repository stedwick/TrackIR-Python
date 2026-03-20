//======================================================================================================
// Copyright 2014, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <string>

// Local includes
#include "Core/BuildConfig.h"

namespace Core
{
    class CORE_API cTimeCode
    {
    public:
        cTimeCode() : mHours( 0 ), mMinutes( 0 ), mSeconds( 0 ), mFrames( 0 ), mTimeCodeSubFrame( 0 ), mTimeCodeDropFrame( false ),
            mValid( false ) { }
        cTimeCode( unsigned int compositeTimecode, unsigned int subFrames = 0, bool isDropFrame = false, bool isValid = true );
        cTimeCode( int64_t compositeTimecodeWithSubFrame, bool isDropFrame = false, bool isValid = true );
        cTimeCode( unsigned int totalFrames, double frameRate );
        cTimeCode( unsigned int hours, unsigned int minutes, unsigned int seconds, unsigned int frames, unsigned int subFrames = 0, 
            bool isDropFrame = false, bool isValid = true );

        int Hours() const { return mHours; }
        int Minutes() const { return mMinutes; }
        int Seconds() const { return mSeconds; }
        int Frame() const { return mFrames; }
        int SubFrame() const { return mTimeCodeSubFrame; }
        bool IsDropFrame() const { return mTimeCodeDropFrame; }
        bool Valid() const { return mValid; }

        /// <summary>Composite, compact timecode value (hours/mins/secs/frames).</summary>
        unsigned int TimeCode() const;

        /// <summary>Composite, compact timecode value (hours/mins/secs/frames/subframes).</summary>
        int64_t TimeCodeWithSubFrame() const;

        std::wstring ToString( bool includeSubframes = true ) const;

    private:
        unsigned int mHours;
        unsigned int mMinutes;
        unsigned int mSeconds;
        unsigned int mFrames;
        unsigned int mTimeCodeSubFrame;
        bool mTimeCodeDropFrame;
        bool mValid;

        void ToString( wchar_t *buffer, int bufferSize, bool includeSubframes = true ) const;
    };
}

