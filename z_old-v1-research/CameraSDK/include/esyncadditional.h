//======================================================================================================
// Copyright 2023, NaturalPoint Inc.
//======================================================================================================
#pragma once

// This is additional data for the esync packet

namespace CameraLibrary
{
    const unsigned int kESYNC2_INPUT_COUNT_ADDITIONAL = 1; // 1 more for PTP
    const unsigned int kESYNC3_INPUT_COUNT_ADDITIONAL = 1; // 1 more for PTP

    enum eEsyncAdditional {
        EA_NONE=0,
        EA_PTP = 1,
        EA_MAX
    };
    // should be 4 bytes.  Hard code to avoid padding issues;
    struct sEsyncAdditionalVersion
    {
        uint32_t version;                   // data type
        static size_t GetSize() { return 4; };
    };

    // should be 16 bytes.  Hard code to avoid padding issues;
    struct sEsyncAdditionalPTP
    {
        int64_t ptpSeconds;
        uint32_t ptpNanoseconds;
        uint32_t inputRate;
        static size_t GetSize() { return 16; };
    };

}
