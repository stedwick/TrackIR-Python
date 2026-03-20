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
    /// An interface class for writing to a stream, buffer, or file.
    /// </summary>
    class CORE_API cIWriter : public cIBasicStream
    {
    public:
        virtual ~cIWriter() = default;

        /// <summary>
        /// Write the given buffer.
        /// </summary>
        /// <param name="buffer">The data buffer.</param>
        /// <param name="bufferSize">The size of the data buffer.</param>
        /// <returns></returns>
        virtual unsigned long long WriteData( const unsigned char *buffer, unsigned long long bufferSize ) = 0;

        virtual void WriteInt( int val ) = 0;
        virtual void WriteLongLong( long long val ) = 0;
        virtual void WriteShort( short val ) = 0;

        virtual void WriteDouble( double val ) = 0;
        virtual void WriteFloat( float val ) = 0;
        virtual void WriteBool( bool val ) = 0;

        virtual void WriteWString( const std::wstring &str ) = 0;

        virtual void WriteUID( const cUID &id ) = 0;

        virtual void WriteByte( unsigned char val ) = 0;

        //==============================================================================================
        // Included interfaces
        //==============================================================================================

        // cIBasicStream
        unsigned long long Tell() const override = 0;
        bool Seek( unsigned long long pos ) override = 0;
        unsigned long long Size() const override = 0;
    };
}
