//======================================================================================================
// Copyright 2014, NaturalPoint Inc.
//======================================================================================================
#pragma once

// Local includes
#include "Core/IWriter.h"
#include "Core/IReader.h"

namespace Core
{
    /// <summary>
    /// The base class for serializers that wish to implement both read and write functionality.
    /// </summary>
    class CORE_API cISerializer : public cIWriter, public cIReader
    {
    public:
        virtual ~cISerializer() = default;

        /// <summary>To Be Deprecated - Write the full contents of the given serializer into this one.</summary>
        virtual void    WriteData( const cISerializer &data ) = 0;

        //==============================================================================================
        // Included interfaces
        //==============================================================================================

        // cIWriter
        unsigned long long WriteData( const unsigned char *buffer, unsigned long long bufferSize ) override = 0;
        void WriteInt( int val ) override = 0;
        void WriteLongLong( long long val ) override = 0;
        void WriteShort( short val ) override = 0;
        void WriteDouble( double val ) override = 0;
        void WriteFloat( float val ) override = 0;
        void WriteBool( bool val ) override = 0;
        void WriteWString( const std::wstring &str ) override = 0;
        void WriteUID( const cUID &id ) override = 0;
        void WriteByte( unsigned char val ) override = 0;

        // cIReader
        unsigned long long ReadData( unsigned char *buffer, unsigned long long bufferSize ) override = 0;
        int ReadInt() override = 0;
        long long ReadLongLong() override = 0;
        short ReadShort() override = 0;
        double ReadDouble() override = 0;
        float ReadFloat() override = 0;
        bool ReadBool() override = 0;
        bool IsEOF() const override = 0;
        std::string ReadString() override = 0;
        std::wstring ReadWString() override = 0;
        cUID ReadUID() override = 0;
        unsigned char ReadByte() override = 0;

        // cIStream
        unsigned long long Tell() const override = 0;
        bool Seek( unsigned long long pos ) override = 0;
        unsigned long long Size() const override = 0;
    };
}

