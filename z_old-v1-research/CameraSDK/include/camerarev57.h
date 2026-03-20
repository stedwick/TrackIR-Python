//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== This is the >>> Active BaseStation <<<
#include <vector>
#include <unordered_map>

#include "Core/ThreadLock.h"

#include "activetag.h"
#include "ActiveIOTypes.h"

#include "camerarev51.h"

namespace CameraLibrary
{
	class CameraRev57 : public CameraRev51
	{
	public:
		CameraRev57();

		bool IsCamera()		 const override	{ return false; }

		auto IsBaseStation() const -> bool override { return true; }  // this is a generic BaseStation device
		bool IsActive2()	 const override	{ return true; }  // this device uses a new BaseStation board
		bool IsWiredTag()	 const override	{ return false; } // this is not a wired active tag

		bool IsIRIlluminationAvailable() const override	  { return false; }
		bool IsHardwareTimeInfoSupported() const override { return false; }

		int ActiveTagCount() const override;
		bool ExpectFrameID( int frameID ) const override;
		bool IsCameraIDAssigned() override;

		eRinglightType RinglightType() const override;

	};
}