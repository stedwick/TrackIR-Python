//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include "cameracommonglobals.h"

class CLAPI cPrecisionTimeBase
{
public:
    cPrecisionTimeBase();
    ~cPrecisionTimeBase();

    void  CatchUp(void);       // reset elapsed time
#ifdef __PLATFORM__LINUX__
    float Elapsed(void) const;       // returns the elapsed time in seconds
    static unsigned long long Ticks();
#elif defined WIN32
    double  Elapsed() const;              //== Return Elapsed Milliseconds =============--------
    static unsigned long long Ticks();
#endif

private:
#ifdef __PLATFORM__LINUX__
    long  GetRawTime() const;
    long  start;
#elif defined WIN32
    __int64	mStart;
    __int64	mFrequency;
#endif
};
