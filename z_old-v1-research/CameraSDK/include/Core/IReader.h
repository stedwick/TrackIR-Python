//======================================================================================================
// Copyright 2014, NaturalPoint Inc.
//======================================================================================================
#pragma once

// System includes
#include <string>

// Local includes
#include "Core/IBasicStream.h"

namespace Core
{
    class cUID;
    class cFilename;

    /// <summary>
    /// An interface class for reading from a stream, buffer, or file.
    /// </summary>
    class CORE_API cIReader : public cIBasicStream
    {
    public:
        virtual ~cIReader() = default;

        virtual unsigned long long ReadData( unsigned char *buffer, unsigned long long bufferSize ) = 0;

        virtual int ReadInt() = 0;
        virtual long long ReadLongLong() = 0;
        virtual short ReadShort() = 0;

        virtual double ReadDouble() = 0;
        virtual float ReadFloat() = 0;
        virtual bool ReadBool() = 0;
        virtual bool IsEOF() const = 0;

        // To be deprecated
        // Has a default implementation so that it can be removed from the scripting interface.
        virtual std::string ReadString() { return std::string(); }

        virtual std::wstring ReadWString() = 0;

        virtual cUID ReadUID() = 0;

        virtual unsigned char ReadByte() = 0;

        //==============================================================================================
        // Included interfaces
        //==============================================================================================

        // cIBasicStream
        unsigned long long Tell() const override = 0;
        bool Seek( unsigned long long pos ) override = 0;
        unsigned long long Size() const override = 0;
    };
}
