#pragma once

//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#include <vector>

#include "Core/FrameRange.h"
#include "Core/Frame.h"

namespace Core
{
    //
    // Contiguous set of frame indexes (integers) which can be traversed forwards or backwards.
    class cFrameRangeSlice
    {
    public:
        enum eTraversalDirection
        {
            Forward,
            Backward
        };

        // Allow implicit conversion from cFrameRange (but not to!)
        cFrameRangeSlice( const cFrameRange& range );

        cFrameRangeSlice( FrameIndex start = 0, unsigned int length = 0, eTraversalDirection traversalDirection = Forward );

        unsigned int FrameCount() const { return mLength; }
        FrameIndex End() const { return mTraversalDirection == Forward ? mStart + (mLength - 1U) : mStart - (mLength - 1U); }
        FrameIndex Start() const { return mStart; }
        bool Empty() const { return mLength == 0U; }
        eTraversalDirection TraversalDirection() const { return mTraversalDirection; }
        FrameIndex operator[]( unsigned int i ) const;

    private:
        FrameIndex mStart;
        unsigned int mLength;
        eTraversalDirection mTraversalDirection;
    };

} // namespace Core
