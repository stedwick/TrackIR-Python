//======================================================================================================
// Copyright 2008, NaturalPoint Inc.
//======================================================================================================
#pragma once

//== Wired Cinepuck 2.0 with primeX13 Main Board<<<

#include "camerarev57.h"
#include "camerarev11.h"


namespace CameraLibrary
{

	struct sActiveTagSettings;
    class CameraRev58 : public CameraRev57
    {
    public:

        CameraRev58();

		bool IsWiredTag() const override { return true; }	// this is a wired active tag

		sActiveTagSettings GetTagSettings();
		int ActiveTagCount() const override { return 1; }


	};
}

