//======================================================================================================
// Copyright 2015 NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <ostream>
#include <istream>
#include <string>

// Local includes
#include "Core/BuildConfig.h"
#include "Core/UID.h"

namespace Core
{
    // For legacy use. Defined outside the class so that it can be excluded from the script wrappers.
    // Enums defined inside class scope cannot be excluded.
    enum eEntityType
    {
        NoLabels_Entity = 0,  // Means an entity that is NOT labeled
        MarkerSet_Entity,     // Labeled marker set markers
        Skeleton_Entity,      // Skeleton or skeleton markers
        RigidBody_Entity      // Rigid bodies or rigid body markers
    };

    /// <summary>A class that represents a marker label. Marker labels consist of two parts: The entity that the marker
    /// is associated with (e.g. skeleton, rigid body, etc.), and the (one-based) index into the label list for that entity.
    /// </summary>
    class CORE_API cLabel
    {
    public:
        cLabel();
        cLabel( const cUID& entityID, unsigned int memberLabelID );

        /// <summary>The node ID for the entity that this label belongs to.</summary>
        const cUID& EntityID() const;

        /// <summary>The label ID within the entity.</summary>
        unsigned int MemberID() const;

        /// <summary>True if the label has a non-null entity ID. Does not attempt to ensure that the entity ID
        /// is valid or that it belongs to an asset that has associated markers for labeling.</summary>
        bool Valid() const
        {
            return ( mEntityID != cUID::kInvalid );
        }

        /// <summary>Legacy method to help with encoding and decoding the previous definition of a label.</summary>
        /// <returns>True if the passed ID was a legacy label that could be decoded.</returns>
        static bool LegacyDecodeUID( const Core::cUID& uid, eEntityType& type, unsigned int& entityID, unsigned int& memberID );

        /// <summary>Comparison operators.</summary>
        bool operator==( const cLabel& other ) const
        {
            return ( mEntityID == other.mEntityID && mMemberLabelID == other.mMemberLabelID );
        }

        bool operator!=( const cLabel& other ) const
        {
            return ( mEntityID != other.mEntityID || mMemberLabelID != other.mMemberLabelID );
        }

        bool operator<( const cLabel & rhs ) const;

        // Convenience constants
        static const cLabel kInvalid;

        //=========================================================================
        // Stream I/O operators
        //=========================================================================
        friend std::ostream& operator<<( std::ostream& os, const Core::cLabel& id )
        {
            Core::cUID::uint64 highBits = id.EntityID().HighBits();
            Core::cUID::uint64 lowBits = id.EntityID().LowBits();
            unsigned int memberId = id.MemberID();
            os.write( (char *) ( &highBits ), sizeof( Core::cUID::uint64 ) );
            os.write( (char *) ( &lowBits ), sizeof( Core::cUID::uint64 ) );
            os.write( (char *) ( &memberId ), sizeof( unsigned int ) );
            return os;
        }

        friend std::istream& operator >> ( std::istream& is, Core::cLabel& id )
        {
            Core::cUID::uint64 highBits, lowBits;
            unsigned int memberId;
            is.read( (char *) ( &highBits ), sizeof( Core::cUID::uint64 ) );
            is.read( (char *) ( &lowBits ), sizeof( Core::cUID::uint64 ) );
            is.read( (char *) ( &memberId ), sizeof( unsigned int ) );

            id.mEntityID = Core::cUID( highBits, lowBits );
            id.mMemberLabelID = memberId;
            return is;
        }

    private:
        cUID mEntityID;
        unsigned int mMemberLabelID;

        // Legacy items for handling the old UID-based labels, mostly for deserializing older takes.
        static const long long klabelIdentifier;
        static const long long kTypeMask;
        static bool LegacyIsLabel( const Core::cUID& uid, bool checkForValidType = false );
    };
}
