//======================================================================================================
// Copyright 2014, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <cstdint>
#include <cstring>

// Local includes
#include "Core/ISerializer.h"

namespace Core
{
    const int kSerializerDefaultBlockSize = 16;  // Each successive block size will double
                                                 // till max block size is reached.
    const int kSerializerMaxBlockSize = 1024 * 1024; // 1MB block size

    /// <summary>
    /// Implementation of a serializer for reading and writing to/from a memory buffer, including the
    /// ability to save and load the memory buffer to/from a file.
    /// </summary>
    class CORE_API cSerializer : public cISerializer
    {
    protected:
        cSerializer( long Size = Core::kSerializerDefaultBlockSize );
        virtual ~cSerializer();

    public:
        static cSerializer* Create( long Size = Core::kSerializerDefaultBlockSize );
        static void Destroy( cSerializer* object );

        std::string HexEncodedString();
        void SetFromHexEncodedString( const std::string& str );
        std::wstring HexEncodedWString();
        void SetFromHexEncodedWString( const std::wstring &str );

        //==============================================================================================
        // Included interfaces
        //==============================================================================================

        // cISerializer
        void WriteData( const cISerializer& data ) override;
        virtual void Clear();

        // cIWriter
        unsigned long long WriteData( const unsigned char* buffer, unsigned long long bufferSize ) override;
        void WriteInt( int val ) override;
        void WriteLongLong( long long val ) override;
        void WriteShort( short val ) override;
        void WriteDouble( double val ) override;
        void WriteFloat( float val ) override;
        void WriteBool( bool val ) override;
        void WriteWString( const std::wstring& str ) override;
        void WriteUID( const cUID& id ) override;
        void WriteByte( unsigned char val ) override;

        // cIReader
        unsigned long long ReadData( unsigned char* Buffer, unsigned long long BufferSize ) override;
        int ReadInt() override;
        long long ReadLongLong() override;
        short ReadShort() override;
        double ReadDouble() override;
        float ReadFloat() override;
        bool ReadBool() override;
        bool IsEOF() const override;
        std::string ReadString() override;
        std::wstring ReadWString() override;
        cUID ReadUID() override;
        unsigned char ReadByte() override;

        // cIStream
        unsigned long long Tell() const override;
        bool Seek( unsigned long long pos ) override;
        unsigned long long Size() const override;

    };

    class CORE_API cSerializerFactory
    {
    public:
        static cISerializer* CreateInstance();
        static void DestroyInstance( cISerializer* instance );
    };
}

