//======================================================================================================
// Copyright 2010, NaturalPoint Inc.
//======================================================================================================
#pragma once

#include "cameracommonglobals.h"

namespace CameraLibrary
{
    class cSegment;
    class Frame;
    class Camera;

	using ObjectDim = unsigned long long;

    class CLAPI cObject
    {
    public:
        cObject();
        ~cObject() = default;

        int Left() const;
        int Right() const;
        int Top() const;
        int Bottom() const;
        float Area() const;

        float Radius() const;
        float Aspect() const;
        float Roundness() const;

        int Width() const;
        int Height() const;

        float X() const;
        float Y() const;

        // The Segments method gives you optimized access to the shape of a 2D object from a
        // camera that is in cSegment or Precision Mode.  This method returns the first 
        // segment in a linked list of segments.  To access the next segment, you traverse
        // the linked list by something like cSegment* nextSegment = segment->Next;
        // Each segment contains the start X,Y coordinate as well as the length of the segment.
        cSegment* Segments();

        // The Camera SDK offers pseudo-labels for each object.  However, this takes a non-
        // trivial amount of CPU time.  As a result, the PseudoLabel() method will return 0 unless
        // the cModuleActiveLabel is attached to the camera.  The easiest way to accomplish this
        // is:  camera->AttachModule(new cModuleActiveLabel());
        int PseudoLabel() const;

        // Objects can have additional flags to signify additional information.  Currently
        // flags are only used in conjunction with cModuleActiveLabel for active marker
        // tracking.
        enum eObjectFlags
        {
            ActiveMarker = 1 << 0,
            Predicted = 1 << 1,
            Acquiring = 1 << 2
        };
        bool IsFlag( eObjectFlags Flag ) const;

    };

    class CLAPI ObjectLink
    {
    public:
        ObjectLink() : mMaster( nullptr ), mSlave( nullptr )
        {
        }

        void SetLink( cObject* master, cObject* slave )
        {
            mMaster = master;
            mSlave = slave;
        }

        const cObject* GetMaster() const { return mMaster; }
        const cObject* GetSlave() const { return mSlave; }

    private:
        cObject* mMaster;
        cObject* mSlave;
    };

    class cTinyObject
    {
    public:
        void PopulateFrom( const cObject& other );
        void WriteTo( cObject* object );

        // NOTE : Do not add, remove, or rearrange members of this class. It will break serialization.
        unsigned short X;
        unsigned char  XMantissa;
        unsigned short Y;
        unsigned char  YMantissa;
        unsigned char  Roundness;
        unsigned char  Area;
    };
}
