//======================================================================================================
// Copyright 2015, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include "BuildConfig.h"

namespace Core
{
    /// <summary>
    ///   A cross-platform event object.  This can be used to signal one thread from another that an
    ///   event has occurred.
    /// </summary>

    class CORE_API cEvent
    {
    public:
        cEvent();
        ~cEvent();

        /// <summary> Trigger the event. </summary>
        void            Trigger() const;

        /// <summary> Wait, potentially indefinitely, for an event to trigger. </summary>
        bool            Wait() const;

        /// <summary> Wait for an event to trigger. Specify timeout in milliseconds.  If zero is specified,
        ///   a wait state will not be entered and will immediately return a boolean specifing if event has triggered. 
        /// </summary>
        bool            Wait( int timeout ) const;

    private:
        void*           mEvent;
    };
}
