//======================================================================================================
// Copyright 2015, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include <iosfwd>

// Local includes
#include "Core/BuildConfig.h"
#include "Core/UID.h"
#include "Core/Vector3.h"
#include "Core/Quaternion.h"
#include "Core/Marker.h"

#pragma warning( push )
#pragma warning( disable: 4251 ) // Warning about "needs to have dll-interface" on STL types

namespace Core
{
    class CORE_API cRigidBody
    {
    public:
        static const int kMaxRigidBodyMarkers = 20;

        cRigidBody();

        cUID ID;             //== Marker ID/Label
        bool Selected;       //== Selection state

        //== rigid body tracked / untracked information ==--

        bool Tracked;
        int FramesUntracked;
		int TrackingAlgorithm;

        //== position and orientation ==--

        void SetPosition( const cVector3f& pos ) { mPosition = pos; }
        const cVector3f& Position() const { return mPosition; }

        void SetRotation( const cQuaternionf& rot ) { mOrientation = rot; }
        const cQuaternionf& Rotation() const { return mOrientation; }

        //== Marker information ==--

        int MarkerCount;
        Core::cMarker Markers[kMaxRigidBodyMarkers];      // World-space calculated marker locations
        bool MarkerTracked[kMaxRigidBodyMarkers];
        float MarkerQuality[kMaxRigidBodyMarkers];

        //== additional information ==--

        float ErrorPerMarker;

    private:
        cVector3f mPosition;
        cQuaternionf mOrientation;
    };

    //=========================================================================
    // Stream I/O operators
    //=========================================================================
    CORE_API std::ostream& operator<<( std::ostream& os, const Core::cRigidBody& rb );
    CORE_API std::istream& operator>>( std::istream& is, Core::cRigidBody& rb );
}

#pragma warning( pop )