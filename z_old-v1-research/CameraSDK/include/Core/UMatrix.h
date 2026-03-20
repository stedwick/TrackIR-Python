//======================================================================================================
// Copyright 2020, NaturalPoint Inc.
//======================================================================================================

#pragma once

#include "Core/DebugSystem.h"

#include <vector>
#include <map>
#include <algorithm>
#include <numeric> // iota
#include <memory> // fill, copy
#include <string>

// Avoid Windows' definition of max, which overrides std::max
#pragma push_macro("min")
#undef min
#pragma push_macro("max")
#undef max

#ifdef _DEBUG
#define DEBUG_BOUNDS_CHECK
#endif

// Raise exception on out-of-bounds error
#ifdef DEBUG_BOUNDS_CHECK
#define DEBUG_BOUNDS {if (i < 0 || i >= size()) throw;}
#else
#define DEBUG_BOUNDS
#endif

namespace Core
{
    template<class U>
    class cMat;

    template<class T>
    struct sIndexDataPair
    {
        sIndexDataPair() : index( -1 ) { }
        sIndexDataPair( int _index, const T &_data ) : index( _index ), data( _data ) { }
        bool operator<( const sIndexDataPair& that ) const { return data < that.data || ( !( that.data < data ) && index < that.index ); }
        bool operator==( const sIndexDataPair& that ) const { return ( data == that.data && index == that.index ); }
        bool Valid() const { return index != -1; }
        int index;
        T   data;
    };
    using sIndexFloat = sIndexDataPair<float>;

    template<class T> class cVec;
    template<typename R, typename VT>
    void ReadVector( R& reader, std::vector<VT>& v );
    template<typename W, typename VT>
    void WriteVector( W& writer, const cVec<const VT> v );
    template<class R, class VT>
    void ReadVector( R& reader, std::vector<std::wstring>& v );
    template<typename W, class VT>
    void WriteVector( W& writer, const cVec<const std::wstring>& v );

    /// <summary> cVec behaves like a std::vector but does not own the data (and never makes copies). </summary>
    template<class T>
    class cVec
    {
    public:
        using const_iterator = const T*;
        using iterator = T*;
        using value_type = T;

        cVec( const std::vector<typename std::remove_const<T>::type>& v ) : mBegin( v.data() ), mEnd( mBegin + v.size() ) { }
        cVec( std::vector<T>& v ) : mBegin( v.data() ), mEnd( mBegin + v.size() ) { }
        cVec() : mBegin( nullptr ), mEnd( nullptr ) { }
        cVec( iterator begin, iterator end ) : mBegin( begin ), mEnd( end ) { ASSERT( mEnd >= mBegin ); }
        cVec( iterator begin, iterator end, const T& v ) : mBegin( begin ), mEnd( end ) { ASSERT( mEnd >= mBegin ); Fill( v ); }
        cVec( const typename std::vector<typename std::remove_const<T>::type>::const_iterator& begin,
            const typename std::vector<typename std::remove_const<T>::type>::const_iterator& end )
        {
            size_t size = std::distance( begin, end );
            mBegin = size ? &*begin : nullptr; // NOTE this avoids debug build crash if begin==nullptr or begin==end
            mEnd = mBegin + size;
        }

        //operator const cVec<typename std::remove_const<T>::type>&() { return *reinterpret_cast<const cVec<typename std::remove_const<T>::type>*>(this); }
        operator const cVec<const T>&() const { return *( const cVec<const T>* )( this ); }

        ~cVec() { mBegin = nullptr; mEnd = nullptr; }

        const_iterator data() const { return mBegin; }

        iterator begin() { return mBegin; }
        const_iterator begin() const { return mBegin; }

        iterator end() { return mEnd; }
        const_iterator end() const { return mEnd; }

        T& operator[]( size_t i ) { DEBUG_BOUNDS; return mBegin[i]; }
        const T& operator[]( size_t i ) const { DEBUG_BOUNDS; return mBegin[i]; }

        bool empty() const { return mEnd == mBegin; }
        size_t size() const { return size_t( mEnd - mBegin ); }

        T& back() { return mEnd[-1]; }
        const T& back() const { return mEnd[-1]; }

        void pop_back() { ASSERT( mEnd > mBegin ); --mEnd; }

        std::vector<typename std::remove_const<T>::type>& CopyTo( std::vector<typename std::remove_const<T>::type>& v ) const
        {
            v.assign( mBegin, mEnd );
            return v;
        }

        void CopyFrom( const cVec<const T> v )
        {
            ASSERT( size() == v.size() );
            if( size() )
            {
                std::copy( v.data(), v.data() + size(), mBegin );
            }
        }

        template<class P> cVec& RemoveIf( P p )
        {
            for( iterator it = mBegin; it != mEnd; ++it )
            {
                if( p( *it ) )
                {
                    std::swap( *it--, *--mEnd );
                }
            }
            return *this;
        }

        void Fill( const T& v ) { std::fill( mBegin, mEnd, v ); }
        size_t IndexOf( const T& v ) const { size_t i = size_t( &v - mBegin ); DEBUG_BOUNDS; return i; }
    private:
        iterator mBegin, mEnd;
    };

    template<class U, class P = std::less<> >
    struct cUSorter
    {
        const cVec<const U> mData;
        cUSorter( const std::vector<U>& data ) : mData( data ) { }
        bool operator()( int i, int j ) const
        {
            return P()( mData[i], mData[j] );
        }
        bool operator()( int i, const U& u ) const
        {
            return P()( mData[i], u );
        }
    };

    template<typename U, typename P = std::less<> >
    class cUIndex
    {
    public:
        cUIndex() = default;
        cUIndex( const cUIndex<U, P>& that )
        {
            *this = that;
        }
        cUIndex( const cVec<const U>& data ) : mUData( data.begin(), data.end() )
        {
            mArgSort.resize( mUData.size() );
            std::iota( mArgSort.begin(), mArgSort.end(), 0 );
            BuildIndex();
        }
        cUIndex& operator=( const cUIndex<U, P>& that )
        {
            mUData = that.mUData;
            mArgSort = that.mArgSort;
            return *this;
        }
        const cVec<const U> UData() const { return mUData; }
        const cVec<const int> IndexData() const { return mArgSort; }
        void Swap( std::vector<U>& data )
        {
            mUData.swap( data );
            mArgSort.resize( mUData.size() );
            std::iota( mArgSort.begin(), mArgSort.end(), 0 );
            BuildIndex();
        }
        size_t size() const { return mUData.size(); }
        void clear()
        {
            mUData.clear();
            mArgSort.clear();
        }
        //! must BuildIndex after this
        void AddU( const U& u )
        {
            mUData.push_back( u );
            mArgSort.push_back( (int) mArgSort.size() );
        }
        const U& operator[]( size_t i ) const { return mUData[i]; }
        template<class V>
        bool HasU( const V& u ) const
        {
            return UIndex( u ) != -1;
        }
        void BuildIndex()
        {
            std::sort( mArgSort.begin(), mArgSort.end(), cUSorter<U, P>( mUData ) );
            //std::sort( mArgSort.begin(), mArgSort.end(), [&]( int i, int j )->bool { return P()(mUData[i], mUData[j]); } );
        }
        template<class V>
        size_t UIndex( const V& u ) const
        {
            std::vector<int>::const_iterator it = std::lower_bound( mArgSort.begin(), mArgSort.end(), u, cUSorter<V, P>( mUData ) );
            //std::vector<int>::const_iterator it = std::lower_bound( mArgSort.begin(), mArgSort.end(), u, [&]( int i, const V &j )->bool { return P()(mUData[i], j); } );
            return ( it == mArgSort.end() || P()( u, mUData[*it] ) ) ? -1 : *it;
        }
        template<class V>
        cVec<const int> UIndices( const V &u ) const
        {
            const int *begin = nullptr, *end = nullptr;
            std::vector<int>::const_iterator it = std::lower_bound( mArgSort.begin(), mArgSort.end(), u, cUSorter<V, P>( mUData ) );
            //std::vector<int>::const_iterator it = std::lower_bound( mArgSort.begin(), mArgSort.end(), u, [&]( int i, const V &j )->bool { return P()(mUData[i], j); } );
            if( it != mArgSort.end() )
                begin = end = &*it;
            while( it != mArgSort.end() && !P()( u, mUData[*it] ) )
            {
                ++it;
                ++end;
            }
            return cVec<const int>(begin, end);
        }
        template<typename W>
        void Save( W &writer ) const
        {
            WriteVector<W, U>( writer, mUData );
        }

        template<typename R>
        void Load( R& reader )
        {
            std::vector<U> data;
            ReadVector<R, U>( reader, data );
            Swap( data );
        }

    private:
        std::vector<U>    mUData;
        std::vector<int>  mArgSort;
    };

    // TODO consider moving this to BinaryStreamReader, change R to cIReader
    template<typename R, typename VT>
    void ReadVector( R& reader, std::vector<VT>& v )
    {
        int tmp = reader.ReadInt();
        ASSERT( tmp == sizeof( VT ) );
        v.resize( reader.ReadLongLong() );
        if( v.size() )
        {
            reader.ReadData( reinterpret_cast<unsigned char*>( v.data() ), ( tmp * v.size() ) );
        }
    }

    // TODO consider moving this to BinaryStreamWriter, change W to cIWriter
    template<typename W, typename VT>
    void WriteVector( W& writer, const cVec<const VT> v )
    {
        int tmp = sizeof( VT );
        writer.WriteInt( tmp );
        writer.WriteLongLong( v.size() );
        if( v.size() )
        {
            writer.WriteData( reinterpret_cast<const unsigned char*>( v.data() ), ( tmp * v.size() ) );
        }
    }

    // TODO consider moving this to BinaryStreamReader, change R to cIReader
    template<class R, class VT=std::wstring>
    void ReadVector( R &reader, std::vector<std::wstring> &v )
    {
        for( size_t size = reader.ReadLongLong(); size; --size)
        {
            v.push_back( reader.ReadWString() );
        }
    }

    // TODO consider moving this to BinaryStreamWriter, change W to cIWriter
    template<typename W, class VT=std::wstring>
    void WriteVector( W& writer, const cVec<const std::wstring>& v )
    {
        writer.WriteLongLong( v.size() );
        for( const std::wstring &s : v )
        {
            writer.WriteWString( s );
        }
    }

    /// <summary>cMatrix is a useful vector of vectors. </summary>
    template<class T>
    class cMatrix
    {
        template<class U, class S> friend class cUMatrix;
        template<class U> friend class cMatrix;
        template<class U> friend class cMat;

    public:
        // These methods are meant to behave similarly to the STL.
        cMatrix() { mRowOffsets.push_back( 0 ); }

        void reserve( size_t s, size_t s2 ) { mRowOffsets.reserve( s + 1 ); mData.reserve( s2 ); }
        void clear() { mRowOffsets.clear(); mData.clear(); mRowOffsets.push_back( 0 ); }
        void release() { mRowOffsets.swap( std::vector<int>{} ); mData.swap( std::vector<T>{} ); mRowOffsets.push_back( 0 ); }

        size_t size() const { return mRowOffsets.size() - 1; }
        bool empty() const { return mData.empty(); }

        // these helpers allow code using cMatrix<T> to look just like std::vector<std::vector<T>>
        cVec<T> operator[]( size_t i )
        {
            DEBUG_BOUNDS;
            return cVec<T>( mData.data() + mRowOffsets[i], mData.data() + mRowOffsets[i + 1] );
        }
        const cVec<const T> operator[]( size_t i ) const
        {
            DEBUG_BOUNDS;
            return cVec<const T>( mData.data() + mRowOffsets[i], mData.data() + mRowOffsets[i + 1] );
        }
        
        bool IsEmptyRow( size_t i ) const { return mRowOffsets[i] == mRowOffsets[i + 1]; }
        int GetRowOffset( size_t i ) const { return mRowOffsets[i]; }

        cMatrix<T>& CopyTo( cMatrix<T>& m ) const
        {
            m.mRowOffsets = mRowOffsets;
            m.mData = mData;
            return m;
        }

        void AddRowItem( const T& v ) { mData.push_back( v ); }
        void EndRow() { mRowOffsets.push_back( (int) mData.size() ); }

        const cVec<const T> FlatData() const { return mData; } // the matrix as a flat vector
        std::vector<T>& EditFlatData() { return mData; } // the matrix as a flat vector

        void SortColumns()
        {
            const size_t imax = size();
            for( size_t i = 0; i < imax; ++i )
            {
                std::sort( mData.begin() + mRowOffsets[i], mData.begin() + mRowOffsets[i + 1] );
            }
        }

        void ClipRows( size_t limit )
        {
            int head = 0, tail = 0;
            const size_t imax = size();
            for( size_t i = 0; i < imax; ++i )
            {
                for( size_t j = 0; j < limit && head < mRowOffsets[i + 1]; ++j )
                {
                    mData[tail++] = mData[head++];
                }
                head = mRowOffsets[i + 1];
                mRowOffsets[i + 1] = tail;
            }
            mData.resize( tail );
        }

        ///<summary> This only works with T = int. Each row of this matrix is made to reference a column of a sparse data matrix, ignoring -1 indices.
        /// conceptually, this is an argsort via bucket sorting.
        ///</summary>
        template<class U>
        void MakeRef( const cVec<const U> data, const size_t size )
        {
            // count each U::index in a bucket (ignoring unlabelled == -1)
            clear();
            mRowOffsets.resize( size + 1, 0 );
            for( const U* it = data.begin(); it != data.end(); ++it )
            {
                if( size_t( it->index ) < size )
                {
                    mRowOffsets[it->index]++;
                }
            }
            // cumsum to convert to offsets, marking the end of each span
            int sum = 0;
            for( int& i : mRowOffsets )
            {
                sum = ( i += sum );
            }
            // count backwards so that offsets end up at the start of each span
            mData.resize( sum );
            for( const U* it = data.end(); it != data.begin(); )
            {
                --it;
                if( size_t( it->index ) < size )
                {
                    mData[--mRowOffsets[it->index]] = int( it - data.data() );
                }
            }
        }

        ///<summary> This depends on T having an index member (such as sIndexDataPair). Each row of this matrix is made to reference a column of a sparse data matrix.
        /// conceptually, this is an argsort via bucket sorting.
        ///</summary>
        void MakeTranspose( const cMat<const T> mat, const size_t size )
        {
            // count each T::index in a bucket (ignoring unlabelled == -1)
            clear();
            mRowOffsets.resize( size + 1, 0 );
            for( const T &v : mat.FlatData() )
            {
                ASSERT( v.index+1 >= 0 && v.index < size );
                mRowOffsets[v.index+1]++;
            }
            // cumsum to convert to offsets, marking the beginning of each span
            int sum = 0;
            for( int& i : mRowOffsets )
            {
                std::swap( sum, i );
                sum += i;
            }
            // count forwards so that offsets end up at the end of each span
            mData.resize( sum );
            for( size_t i = 0; i < mat.size(); ++i )
            {
                for( const T &v : mat[i] )
                {
                    mData[mRowOffsets[v.index+1]++] = T( i, v.data );
                }
            }
        }

        template<class U>
        void SetShape( const cMatrix<U>& m, const T& d = T() )
        {
            mRowOffsets = m.mRowOffsets;
            mData.clear();
            mData.resize( mRowOffsets.back(), d );
        }

        template<class U>
        void SetShape( const cMat<U>& m, const T& d = T() )
        {
            m.mRowOffsets.CopyTo( mRowOffsets );
            mData.clear();
            mData.resize( m.mData.size(), d );
        }

        void SetShape( size_t rows, size_t cols, const T& d = T() )
        {
            mRowOffsets.resize( rows + 1 );
            for( size_t r = 0; r <= rows; ++r )
                mRowOffsets[r] = r * cols;
            mData.clear();
            mData.resize( rows * cols, d );
        }

        ///<summary>This method depends on the type having a index member (such as sIndexDataPair)</summary>
        int NumCols() const
        {
            int count = -1;
            for( const T& item : mData )
            {
                count = std::max( count, item.index );
            }
            return count + 1;
        }

        template<class W> void Save( W& writer ) const
        {
            writer.WriteByte( 5 ); // revision
            WriteVector<W, int>( writer, mRowOffsets );
            WriteVector<W, T>( writer, mData );
        }

        template<class R> bool Load( R& reader, int* _revision = nullptr )
        {
            unsigned char revision = reader.ReadByte();
            if( _revision != nullptr ) *_revision = revision;
            if( revision != 5 ) return false; // from the past or future: ignore
            ReadVector( reader, mRowOffsets );
            ReadVector( reader, mData );
            return true;
        }

    private:
        std::vector<int> mRowOffsets;
        std::vector<T> mData;
    };

    /// <summary> cMat behaves like a cMatrix, but does not own the data (and never makes copies). </summary>
    template<class T>
    class cMat
    {
        template<class U> friend class cMatrix;
        using c_int = typename std::conditional<std::is_const<T>::value, const int, int>::type;

    public:
        cMat() : mSize( 0 ) { }
        cMat( cMatrix<T>& mat ) : mSize( mat.size() ), mRowOffsets( mat.mRowOffsets ), mData( mat.mData ) { }
        cMat( const cMatrix<typename std::remove_const<T>::type>& mat ) : mSize( mat.size() ), mRowOffsets( mat.mRowOffsets ), mData( mat.mData ) { }

        size_t size() const { return mSize; }

        cVec<T> EditRow( size_t i )
        {
            DEBUG_BOUNDS;
            return cVec<T>( mData.begin() + mRowOffsets[i], mData.begin() + mRowOffsets[i + 1] );
        }

        const cVec<const T> operator[]( size_t i ) const
        {
            DEBUG_BOUNDS;
            return cVec<const T>( mData.begin() + mRowOffsets[i], mData.begin() + mRowOffsets[i + 1] );
        }

        cMatrix<T>& CopyTo( cMatrix<T>& m ) const
        {
            mRowOffsets.CopyTo( m.mRowOffsets );
            mData.CopyTo( m.mData );
            return m;
        }
        ///<summary> the matrix data as a flat vector </summary>
        const cVec<const T> FlatData() const { return mData; }

        ///<summary> the matrix data as an editable flat vector </summary>
        cVec<T> EditFlatData() { return mData; }

        ///<summary>This method depends on the type having an index member (such as sIndexDataPair)</summary>
        int NumCols() const
        {
            int count = -1;
            for( const T* it = mData.begin(); it != mData.end(); ++it )
            {
                count = std::max( count, it->index );
            }
            return count + 1;
        }

        bool IsEmptyRow( size_t i ) const { return mRowOffsets[i] == mRowOffsets[i + 1]; }
        int GetRowOffset( size_t i ) const { return mRowOffsets[i]; }

    private:
        size_t mSize; /// == mRowOffsets.size()-1 but useful to have here when debugging
        cVec<c_int> mRowOffsets;
        cVec<T> mData;
    };

    /// <summary> cUMatrix is a cMatrix which additionally supports assigning an ID (templated type U)
    /// to each index and converting back-and-forth between index and ID. </summary>
    template<class U, class T>
    class cUMatrix : public cMatrix<T>
    {
    public:
        void clear()
        {
            mUData.clear();
            cMatrix<T>::clear();
            mHash = 0;
        }

        bool HasU( const U& u ) const
        {
            return mUData.HasU( u );
        }

        //! must BuildUIndex after this
        void AddU( const U& u )
        {
            mUData.AddU( u );
            UpdateHash();
        }

        //! must BuildUIndex after this
        void AddFixedRow( const U& u, size_t n, const T& v )
        {
            AddU( u );
            cMatrix<T>::mData.insert( cMatrix<T>::mData.end(), n, v );
            cMatrix<T>::EndRow();
        }

        //! must BuildUIndex after this
        void AddFixedRow( const U &u, const cVec<const T> &v )
        {
            AddU( u );
            cMatrix<T>::mData.insert( cMatrix<T>::mData.end(), v.begin(), v.end() );
            cMatrix<T>::EndRow();
        }

        void BuildUIndex()
        {
            mUData.BuildIndex();
        }

        const cVec<const T> URow( const U& u ) const { size_t i = mUData.UIndex( u ); return i != -1 ? cMatrix<T>::operator[]( i ) : cVec<const T>(); }
        cVec<T> EditURow( const U& u ) { UpdateHash(); size_t i = mUData.UIndex( u ); return i != -1 ? cMatrix<T>::operator[]( i ) : cVec<T>(); }

        size_t U2Index( const U& u ) const { return mUData.UIndex( u ); }
        const U& Index2U( size_t i ) const { return mUData[i]; }

        const cVec<const U> GetIndex2U() const { return mUData.UData(); }

        template<class W>
        void Save( W& writer ) const
        {
            cMatrix<T>::Save( writer );
            mUData.Save( writer );
        }

        template<class R>
        bool Load( R& reader )
        {
            int revision;
            bool ok = cMatrix<T>::Load( reader, &revision );
            if( !ok ) return false;
            if( revision != 5 ) return false; // from the past or future: ignore
            mUData.Load( reader );
            UpdateHash();
            return true;
        }

        void CopyTo( cUMatrix &that ) const
        {
            cMatrix<T>::CopyTo( that );
            that.mHash = mHash;
            that.mUData = mUData;
        }

        void CopySubset( cUMatrix &that, cVec<U> subset ) const
        {
            std::sort( subset.begin(), subset.end(), [&]( const U &u1, const U &u2 )->bool { return U2Index( u1 ) < U2Index( u2 ); } );
            for( const U &key : subset )
            {
                that.AddU( key );
            }
            that.BuildUIndex();
            for( const U &key : subset )
            {
                if( HasU( key ) )
                {
                    for( const T &value : URow( key ) )
                    {
                        if( size_t( value.index ) < mUData.size() )
                        {
                            int index = that.U2Index( Index2U( value.index ) );
                            if( index != -1 )
                            {
                                that.AddRowItem( T( index, value.data ) );
                            }
                        }
                    }
                }
                that.EndRow();
            }
        }

        bool operator==( const cUMatrix& other ) const
        {
            return mHash == other.mHash;
        }

        bool operator!=( const cUMatrix& other ) const
        {
            return !( *this == other );
        }

        void UpdateHash()
        {
            mHash = ++sHash;
        }

    private:
        int mHash = 0;
        cUIndex<U> mUData;

        static int sHash;
    };

    template<class U, class T>
    int cUMatrix<U, T>::sHash = 0;
}

#pragma pop_macro("max")
#pragma pop_macro("min")
