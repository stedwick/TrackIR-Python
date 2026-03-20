// *******************************************************************************
// * Copyright 2023, NaturalPoint Inc.
// *******************************************************************************
// * Description:
// *
// *   Header file containing class function definitions for the console application.
// * 
// *******************************************************************************

#pragma once

#include <string>
#include <NPClientWraps.h>

/// Class that manages the console for displaying tracking data.
class TrackIR_ClientCPP
{
public:
	static void DisplayWelcomeMessage();
	static std::wstring FindDllLocation(std::wstring regKeyLoc);
	static HWND GetConsoleHwnd();
	static void TrackIR_EnhancedInit();
	static NPRESULT HandleTrackIRData();

	static void TrackIR_EnhancedShutdown();
	static void HandleInput( int key );
};
