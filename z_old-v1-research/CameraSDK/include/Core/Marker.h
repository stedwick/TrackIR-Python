//======================================================================================================
// Copyright 2012, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <vector>

// Local includes
#include "Core/Label.h"
#include "Core/Vector3.h"
#include "Core/UMatrix.h"

#pragma warning( push )
#pragma warning( disable: 4251 ) // Warning about "needs to have dll-interface" on STL types

namespace Core
{
    enum eMarkerFlags
    {
        Occluded = 1 << 0,
        PointCloudSolved = 1 << 1,
        ModelFilled = 1 << 2,
        HasModel = 1 << 3,
        Unlabeled = 1 << 4,
        Active = 1 << 5,
        Established = 1 << 6,
        Measurement = 1 << 7,
        AnchorMarker = 1 << 8,
        Expected = 1 << 9
    };

    template <typename T>
    class cTMarker
    {
    public:
        cTMarker() : X( 0 ), Y( 0 ), Z( 0 ), ID( cUID::kInvalid ), ActiveID( 0 ), Size( 0 ), Label( cLabel::kInvalid ), Residual( 0 ), Selected( false ),
            Synthetic( false ), Flags( 0 ) { }
        cTMarker( T x, T y, T z ) : X( x ), Y( y ), Z( z ), ID( cUID::kInvalid ), ActiveID( 0 ), Size( 0 ), Label( cLabel::kInvalid ), Residual( 0 ), Selected( false ),
            Synthetic( false ), Flags( 0 ) { }
        cTMarker( const cVector3<T>& pos ) : X( pos.X() ), Y( pos.Y() ), Z( pos.Z() ), ID( cUID::kInvalid ), ActiveID( 0 ), Size( 0 ),
            Label( cLabel::kInvalid ), Residual( 0 ), Selected( false ), Synthetic( false ), Flags( 0 ) { }

        bool operator==( const cTMarker& other ) const { return ( Label == other.Label ); }
        bool operator!=( const cTMarker& other ) const { return ( Label != other.Label ); }

        /// <summary>Set the position.</summary>
        void SetPosition( T x, T y, T z ) { X = x; Y = y; Z = z; }
        void SetPosition( const cVector3<T>& pos ) { X = pos.X(); Y = pos.Y(); Z = pos.Z(); }

        /// <summary>Retrieve position as a vector.</summary>
        cVector3<T> Position() const { return cVector3<T>( X, Y, Z ); }

        /// <summary>Returns true if this was recorded from an active marker.</summary>
        bool IsActiveMarker() const { return ( ( Flags & Active ) != 0 ); }

        /// <summary>Returns true if this was an active marker in established state.</summary>
        bool IsEstablishedMarker() const { return ( ( Flags & Established ) != 0 ); }

        /// <summary>Returns true if this was recorded from a measurement (probe) point.</summary>
        bool IsMeasurement() const { return ( ( Flags & Measurement ) != 0 ); }

        /// <summary>Returns true if this is a defined anchor marker location.</summary>
        bool IsAnchorMarker() const { return ( ( Flags & AnchorMarker ) != 0 ); }

        /// <summary>Returns true if this is a real marker (i.e. not an anchor or measurement).</summary>
        bool IsRealMarker() const { return ( ( Flags & ( AnchorMarker | Measurement ) ) == 0 ); }

        /// <summary>Makes this an active marker with the given ID.</summary>
        void SetActiveID( unsigned int activeID ) { ActiveID = activeID; Flags = ( ActiveID ? ( Flags | Active ) : ( Flags & ( ~Active ) ) ); }

        /// <summary>Special method for generating a UID that is a little easier on the eyes when viewing the
        /// ID of unlabeled markers.
        static cUID GenerateUnlabeledMarkerUID()
        {
            return cUID( sNextMarkerID++, sMarkerIDLowBits );
        }

        cUID ID;               // Marker ID (which may be assigned during reconstruction)
        unsigned int ActiveID; // ActiveID (read from the on/off sequence of historical frames)
        T X;                   // Position in meters
        T Y;                   // Position in meters
        T Z;                   // Position in meters
        T Size;                // Diameter in meters
        T Residual;            // Residual in mm/ray
        cLabel Label;          // Marker Label
        bool Selected;         // Selection state
        bool Synthetic;        // Synthetic markers created in pipeline such as virtual finger tip markers
        unsigned short Flags;  // bit-encoded marker flags (occluded, model-solved, active, unlabeled, etc)

        static unsigned long long sNextMarkerID;
        static unsigned long long sMarkerIDLowBits;
    };

    template <typename T>
    unsigned long long cTMarker<T>::sNextMarkerID = 1;
    template <typename T>
    unsigned long long cTMarker<T>::sMarkerIDLowBits = cUID::Generate().HighBits(); // HighBits is more random than LowBits.

    // Leave these as typedefs for the script wrapper system.
    using cMarker = cTMarker<float>;
    using cMarkerf = cTMarker<float>;
    using cMarkerd = cTMarker<double>;

    //=========================================================================
    // Stream I/O operators
    //=========================================================================
#if !defined(__PLATFORM__LINUX__) // gcc has no clue about stream operators.
    template<typename T>
    std::ostream& operator<<( std::ostream& os, const Core::cTMarker<T>& v )
    {
        Core::cUID::uint64 highBits = v.ID.HighBits();
        Core::cUID::uint64 lowBits = v.ID.LowBits();

        os.write( (char*) &v.X, sizeof( T ) );
        os.write( (char*) &v.Y, sizeof( T ) );
        os.write( (char*) &v.Z, sizeof( T ) );
        os.write( (char*) ( &highBits ), sizeof( Core::cUID::uint64 ) );
        os.write( (char*) ( &lowBits ), sizeof( Core::cUID::uint64 ) );
        os.write( (char*) &v.ActiveID, sizeof( unsigned int ) );
        os.write( (char*) &v.Size, sizeof( T ) );
        os << v.Label;
        os.write( (char*) &v.Selected, sizeof( bool ) );
        os.write( (char*) &v.Residual, sizeof( T ) );
        os.write( (char*) &v.Synthetic, sizeof( bool ) );
        os.write( (char*) &v.Flags, sizeof( short ) );

        return os;
    }

    template<typename T>
    std::istream& operator >> ( std::istream& is, Core::cTMarker<T>& v )
    {
        Core::cUID::uint64 highBits = v.ID.HighBits();
        Core::cUID::uint64 lowBits = v.ID.LowBits();

        is.read( (char*) &v.X, sizeof( T ) );
        is.read( (char*) &v.Y, sizeof( T ) );
        is.read( (char*) &v.Z, sizeof( T ) );
        is.read( (char*) ( &highBits ), sizeof( Core::cUID::uint64 ) );
        is.read( (char*) ( &lowBits ), sizeof( Core::cUID::uint64 ) );
        is.read( (char*) &v.ActiveID, sizeof( unsigned int ) );
        is.read( (char*) &v.Size, sizeof( T ) );
        is >> v.Label;
        is.read( (char*) &v.Selected, sizeof( bool ) );
        is.read( (char*) &v.Residual, sizeof( T ) );
        is.read( (char*) &v.Synthetic, sizeof( bool ) );
        is.read( (char*) &v.Flags, sizeof( short ) );

        v.ID = cUID( highBits, lowBits );
        return is;
    }
#endif // no stream operators for gcc

    class cMarkerStick
    {
    public:
        cMarkerStick() : mOrigin( cLabel::kInvalid ), mEnd( cLabel::kInvalid ), mColor( 1, 1, 1 ) { }
        cMarkerStick( const cLabel& origin, const cLabel& end, const cVector3f& color ) :
            mOrigin( origin ), mEnd( end ), mColor( color ) { }

        // Labels (currently represented as ID's) of the origin and endpoint markers.
        cLabel mOrigin;
        cLabel mEnd;

        // RGB line color, with color components in range of [0,1].
        cVector3f mColor;
    };
}

#pragma warning( pop )
