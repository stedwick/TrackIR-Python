//======================================================================================================
//======================================================================================================
// Copyright 2022, NaturalPoint Inc.
//======================================================================================================
#pragma once
namespace CameraLibrary
{
	class cLabelSegment
	{
    public:
        cLabelSegment() = default;
        cLabelSegment(int x, int y, unsigned int length);
        ~cLabelSegment() = default;

        // Public Segment Interface
        int X() const;
        int Y() const;
        int Length() const;
        int StopX() const;
        int Label() const;

        bool AddSegment(int x, int y, unsigned int length);
        bool AddSegment(const cLabelSegment & segment);
        void AddToLength(const unsigned int addLength);
        void SetLabel(int newLabel) { mLabel = newLabel; };
        void Clear();
        cLabelSegment& operator++() { ++mLength; return *this; };
    private:
        int mX=0;
        int mY=0;
        int mLength=0;
        int mLabel=0;
	};
};
