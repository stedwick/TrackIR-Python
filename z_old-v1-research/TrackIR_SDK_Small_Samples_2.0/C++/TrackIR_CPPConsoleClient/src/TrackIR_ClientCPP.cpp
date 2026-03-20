// *******************************************************************************
// * Copyright 2023, NaturalPoint Inc.
// *******************************************************************************
// * Description:
// *
// *   Console application that displays tracking information coming from TrackIR
// *   and uses keyboard input for different TrackIR function calls. This acts as 
// *   a simple example of how to connect, interact with, and close your connection to TrackIR.
// * 
// *******************************************************************************

#include <Windows.h>
#include <iostream>
#include <string>
#include "string.h"
#include <cstdio>

#include "NPClientWraps.h"
#include "TrackIR_ClientCPP.h"


using namespace NPClient;

///<summary>
/// The developer ID provided to you by NaturalPoint.
/// The default is the SDK ID, which is available to everyone.
///</summary>
#define NP_DEVELOPER_ID 1000


/// <summary>
/// Registry location for locating NPClient.dll
/// </summary>
std::wstring dLLRegKeyLoc = L"Software\\NaturalPoint\\NATURALPOINT\\NPClient Location\\";

unsigned long	NPFrameSignature; // keep track of last frame to see if anything has changed
bool readNPData = true; // keep track of whether or not to try to read data from TrackIR

/// <summary>
/// Display welcome message for the TrackIR SDK console application.
/// </summary>
void TrackIR_ClientCPP::DisplayWelcomeMessage()
{
    std::cout << "======================================================================================================\n";
    std::cout << "Welcome to the TrackIR SDK! \n";
    std::cout << "======================================================================================================\n";
    std::cout << "In this sample, you will be able to see how to implement TrackIR into a basic C++ application. \n\n";
	std::cout << "Keyboard Controls: \n";
	std::cout << "\t 1: Initialize application (find and initialize DLL, register window handle, get TrackIR version, select data to receive, start data transmission)\n";
	std::cout << "\t 2: Unregister window handle and stop receiving data\n";
	std::cout << "\t 3: Start receiving data\n";
	std::cout << "\t 4: Stop receiving data\n";
	std::cout << "\t 5: Recenter TrackIR headset\n";
	std::cout << "\t ESC: Quit application\n\n";

	// let user read before moving on
	Sleep( 1000 );
}

/// <summary>
/// Uses windows api to find the location of the NPClient.dll.
/// </summary>
/// <remarks>
/// You can find the registry value at Software\NaturalPoint\NATURALPOINT\NPClient Locations in the windows registry.
/// </remarks>
/// <param name="regKeyLoc">The registry key location for the DLL.</param>
/// <returns>The location of the NPClient.dll.</returns>
std::wstring TrackIR_ClientCPP::FindDllLocation(std::wstring regKeyLoc)
{
	//find path to NPClient.dll
	HKEY pKey = NULL;
	//open the registry key 
	if (RegOpenKeyEx( HKEY_CURRENT_USER, (LPCWSTR) regKeyLoc.c_str(), 0, KEY_READ, &pKey) != ERROR_SUCCESS)
	{
		//error condition
		return L"Error: DLL Location key not present\n";
	}

	//get the value from the key
	wchar_t szValue[250];
	DWORD dwSize;

	//first discover the size of the value
	if (RegQueryValueEx( pKey, L"Path", NULL, NULL, NULL, &dwSize ) == ERROR_SUCCESS)
	{
		//now get the value
		if (RegQueryValueEx( pKey, L"Path", NULL, NULL, reinterpret_cast<LPBYTE>( szValue ), &dwSize ) == ERROR_SUCCESS)
		{
			//everything worked
			RegCloseKey( pKey );
			std::wstring lValue(szValue);
			lValue.pop_back();
			return lValue;
		}
		else//error
		{
			return L"Error reading location key!";
		}

	}
	RegCloseKey( pKey );
	return L"Error";

}


/// <summary> Initialize the NPClient interface to start recieving data. </summary>
/// <remarks>
/// This function demonstrates how to communicate with TrackIR to start receiving tracking data.
/// 1. We locate and load the code from NPClient(64).dll. See \ref NPClient::NPClient_Init and \ref FindDllLocation.
/// 2. Then, we register the console's window handle so that TrackIR can start communicating with the console. \ref NPClient::NP_RegisterWindowHandle
/// and \ref GetConsoleHwnd.
/// 3. Next, we get the TrackIR version that is being used, and request the types of data we want to receive.
///  See \ref NPClient::NP_RequestData
/// 4. Lastly, we tell TrackIR we are ready to start receiving data and start processing tracking data.
/// </remarks>
void TrackIR_ClientCPP::TrackIR_EnhancedInit()
{


	NPRESULT result;
	//
	// Search registry for DLL location
	//
	std::wstring pathToDll = TrackIR_ClientCPP::FindDllLocation( dLLRegKeyLoc ) + L"\\";
	if (pathToDll.find( L"Error" ) == -1)
	{
		std::wcout << L"Found DLL in " << pathToDll << "\n";
	}
	else
	{
		std::cout << "Error: Cannot find DLL location";
		readNPData = false;
		return;
	}


	//
	// Initialize functions from DLL
	//
	result = NPClient_Init( pathToDll ) ;
	if (NP_OK == result)
	{
		std::cout << "NPClient interface -- initialize OK.\n";
	}
	else
	{
		std::cout << "Error initializing NPClient interface. Make sure TrackIR is running!\n";
		readNPData = false;
		return;
	}

	//
	// Get the current console window handle 
	//
	HWND handle = GetConsoleHwnd();
	// Try again, just in case it didn't get the handle
	if (handle == nullptr)
	{
		handle = GetConsoleHwnd();
	}

	//
	// Register window handle to communicate between TrackIR and application
	//
	result = NP_RegisterWindowHandle( handle );
	if (NP_OK == result)
	{
		std::cout << "NPClient : Window handle registration successful.\n";
	}
	else
	{
		std::cout << "NPCLient : Error Registering window handle.\n";
		readNPData = false;
		return;
	}


	//
	// Query the NaturalPoint software version
	
	// request version information using 2 messages, they cannot be expected to arrive in a specific order - so always parse using the High byte
	// the messages have a NPCONTROL byte in the first parameter, and the second parameter has packed bytes.

	unsigned short wNPClientVer;
	result = NP_QueryVersion( &wNPClientVer );
	if (NP_OK == result)
	{
		char csMajorVer[250], csMinorVer[250], csVerMsg[250];
		sprintf_s(csMajorVer, "%d", ( wNPClientVer >> 8 ) ); // right shift by 1 byte to get high byte (major version)
		sprintf_s(csMinorVer, "%02d", ( wNPClientVer & 0x00FF ) ); // mask remaining to get the lower byte (minor version)
		sprintf_s(csVerMsg, "NaturalPoint software version is %s.%s \n", csMajorVer, csMinorVer );
		printf(csVerMsg);
	}
	else{
		std::cout << "NPClient : Error querying NaturalPoint software version.\n";
	}

	//
	// Register program profile ID
	//
	result = NP_RegisterProgramProfileID( NP_DEVELOPER_ID );
	if (result == NP_OK)
		std::cout << "Registered Developer ID\n";
	else
	{
		std::cout << "NPClient : Error Registering Developer ID.\n";
		readNPData = false;
		return;
	}

	//[requestDataSnippet]
	// Tell TrackIR that we are using Pitch, Yaw, Roll, and X,Y,Z.
	unsigned int DataFields = 0;
	DataFields |= NPPitch;
	DataFields |= NPYaw;
	DataFields |= NPRoll;
	DataFields |= NPX;
	DataFields |= NPY;
	DataFields |= NPZ;

	NP_RequestData( DataFields );
	//[requestDataSnippet]


	//
	// Tell TrackIR we are ready to receive data
	//
	result = NP_StartDataTransmission();
	if (result == NP_OK)
		std::cout << "Data transmission started\n";
	else
	{
		std::cout << "NPClient : Error starting data transmission.\n";
		readNPData = false;
		return;
	}


	readNPData = true;

}

/// <summary>
/// Processes data received from TrackIR.
/// </summary>
/// <remarks>
/// This is a simple loop controlled by a timer called once every 17ms or 60fps (although, TrackIR sends at a rate of 120fps). 
/// It consists of the following steps:
/// 1. Call NP_GetData and see if the function returned NP_OK
/// 2. Check if TrackIR is not paused, and check if we received a new frame based on the frame signature
/// 3. Convert data from TrackIR to degrees and cms in a left-handed coordinate basis, and finally output. 
/// </remarks>
/// <returns>If successful returns NP_OK, otherwise will return NP_ERR_NO_DATA.</returns>
NPRESULT TrackIR_ClientCPP::HandleTrackIRData()
{
	
	TRACKIRDATA tid;
	memset( &tid, 0, sizeof( TRACKIRDATA ) );

	// Go get the latest data
	NPRESULT result = NP_GetData( &tid );
	
	if (NP_OK == result)
	{
		// Got data to process ...

		// compare the last frame signature to the current one
		// if they are not the same then new data has arrived since then
		char buf[250];
		static int prevLength = 0;

		// check if we are not using mouse emulation mode.
		// TrackIR ships with mouse emulation software to control the mouse from head position
		// It is very likely this will cause weird things to happen in game.
		if (tid.Status == NPSTATUS_REMOTEACTIVE)
		{
			if (NPFrameSignature != tid.FrameSignature)
			{
				
				//[convertUnitsSnippit]
				// Negate right-hand rule rotations to be consistent with left-handed coordinate basis.

				// convert TIR translation values from TIR units to centimeters
				double x = ( tid.X / NP_MAX_VALUE ) * NP_MAX_TRANSLATION;
				double y = ( tid.Y / NP_MAX_VALUE ) * NP_MAX_TRANSLATION;
				double z = ( tid.Z / NP_MAX_VALUE ) * NP_MAX_TRANSLATION;

				// convert TIR rotation values from TIR units to degrees
				double yaw = ( tid.Yaw / NP_MAX_VALUE ) * NP_MAX_ROTATION;
				double pitch = ( tid.Pitch / NP_MAX_VALUE ) * NP_MAX_ROTATION;
				double roll = ( tid.Roll / NP_MAX_VALUE ) * NP_MAX_ROTATION;
				//[convertUnitsSnippit]

				// Process some NaturalPoint Tracking Data
				sprintf_s(buf, "Pitch = %03.02f, Yaw = %03.02f, Roll = %03.02f, X = %03.02f, Y = %03.02f, Z = %03.02f, Frame = %d", 
					pitch, yaw, roll, x, y, z, tid.FrameSignature);
				NPFrameSignature = tid.FrameSignature;

				//
				// All other data fields in TRACKIRDATA can be handled in a similar way.
				//
			}
			else
			{
				sprintf_s(buf, "No Data");
				result = NP_ERR_NO_DATA;
			}
		}
		else
		{
			sprintf_s(buf, "User Disabled");
			result = NP_ERR_NO_DATA;
		}

		std::string t_str( buf );
		// Calculate the number of spaces needed to overwrite any leftover characters from the previous line
		int numSpaces = max( 0, prevLength - static_cast<int>( t_str.size() ) );

		// print text
		std::cout << "\r" << t_str;

		// Print extra spaces to overwrite any leftover characters
		for (int i = 0; i < numSpaces; i++) {
			std::cout << " ";
		}

		std::cout << std::flush;

		prevLength = static_cast<int>( t_str.size() );

	}

	return result;


}

/// <summary>
/// Shutdown procedure for TrackIR consists of 2 steps.
/// 1. stop data transmission
/// 2. unregister window handle
/// </summary>
/// <remarks>
/// TrackIR will automatically detect through the window handle if you 
/// close the application. However, this step is still good practice 
/// just to make sure everything happens the way you want it to happen.
/// </remarks>
void TrackIR_ClientCPP::TrackIR_EnhancedShutdown()
{
	NP_StopDataTransmission();
	NP_UnregisterWindowHandle();
}


/// <summary>
/// Retrieves the current console's window handle.
/// </summary>
/// <returns>WINAPI window handle object</returns>
HWND TrackIR_ClientCPP::GetConsoleHwnd()
{
#define MY_BUFSIZE 1024 // Buffer size for console window titles.
	HWND hwndFound;         // This is what is returned to the caller.
	wchar_t pszNewWindowTitle[MY_BUFSIZE]; // Contains fabricated
	// WindowTitle.
	wchar_t pszOldWindowTitle[MY_BUFSIZE]; // Contains original
	// WindowTitle.

	// Fetch current window title.

	GetConsoleTitle( pszOldWindowTitle, MY_BUFSIZE );

	// Format a "unique" NewWindowTitle.

	wsprintf( pszNewWindowTitle, L"%d/%d",
		(ULONG)GetTickCount64(),
		GetCurrentProcessId() );

	// Change current window title.

	SetConsoleTitle( pszNewWindowTitle );

	// Ensure window title has been updated.

	Sleep( 50 );

	// Look for NewWindowTitle.

	hwndFound = FindWindow( NULL, pszNewWindowTitle );

	// Restore original window title.

	SetConsoleTitle( pszOldWindowTitle );

	return hwndFound;
}

// Processes keyboard input by mapping key codes to functions
void TrackIR_ClientCPP::HandleInput( int key ) {
	NPRESULT result;

	if (readNPData){
		std::cout << "\n";
	}
	switch (key) {
	case '1':
		TrackIR_EnhancedInit();
		break;
	case '2':
		//
		// Unregister window handle to communicate between TrackIR and application
		//
		result = NP_UnregisterWindowHandle();
		if (NP_OK == result)
		{
			std::cout << "NPClient : Window handle unregister successful.\n";
			readNPData = false;
		}
		else
		{
			std::cout << "\nNPCLient : Error Unregistering window handle.\n";
		}
		break;
	case '3':
		//
		// Start reading data 
		//
		result = NP_StartCursor();
		if (NP_OK == result)
		{
			std::cout << "NPClient : Started cursor successful.\n";
			readNPData = true;
		}
		else
		{
			std::cout << "NPCLient : Error starting cursor.\n";
		}
		break;
	case '4':
		//
		// Stop reading data 
		//
		result = NP_StopCursor();
		if (NP_OK == result)
		{
			std::cout << "NPClient : Stopped cursor successful.\n";
			readNPData = false;
		}
		else
		{
			std::cout << "NPCLient : Error stopping cursor.\n";
		}
		break;
	case '5':
		//
		// Recenter
		//
		result = NP_ReCenter();
		if (NP_OK == result)
		{
			std::cout << "NPClient : Recenter successful.\n";
		}
		else
		{
			std::cout << "NPCLient : Error Recentering. \n";
		}
		break;
		
	default:
		break;
	}
}


// Callback function that is called on by the timer to process keyboard input and head-tracking data.
void CALLBACK OnTimer( HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime )
{

	// variables to keep track of status of key press
	// we only want one actions performed when a key is pressed and held down.
	static bool isKeyPressed[5] = { false };
	int keyCodes[5] = { '0' + 1, '0' + 2, '0' + 3, '0' + 4, '0' + 5 };

	// Check for keyboard input 1-5
	for (int i = 0; i < 5; i++)
	{
		if (GetAsyncKeyState( keyCodes[i] ) & 0x8000)
		{
			if (!isKeyPressed[i])
			{
				// Do assigned action for the key that is pressed once
				TrackIR_ClientCPP::HandleInput( keyCodes[i] );
				isKeyPressed[i] = true;
			}
		}
		else{
			isKeyPressed[i] = false;
		}
	}

	// Check if ESC key is pressed (non-blocking)
	if (GetAsyncKeyState( VK_ESCAPE ) & 0x8000)
	{
		PostQuitMessage( 0 );  // Post a quit message to break out of the main message loop
	}


	if (!readNPData)
	{
		return;
	}

	NPRESULT result;
	std::string csTimerMsg;
	
	// get tracking data from TrackIR
	result = TrackIR_ClientCPP::HandleTrackIRData();
	
}


// Go through TrackIR shutdown process before we shutdown the console.
BOOL WINAPI ConsoleHandler( DWORD dwCtrlType )
{
	// if the program is shutdown in some kind of way
	// we want to unregister the window handle
	switch (dwCtrlType)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		TrackIR_ClientCPP::TrackIR_EnhancedShutdown();
		return TRUE;
	}

	return FALSE;
}

int main()
{

    TrackIR_ClientCPP::DisplayWelcomeMessage();
	TrackIR_ClientCPP::TrackIR_EnhancedInit();
	std::cout << std::flush;

	// Set up console control handler to unregister the window handle from trackir before closing 
	if (!SetConsoleCtrlHandler( ConsoleHandler, TRUE ))
	{
		std::cerr << "Failed to set console control handler." << std::endl;
		return 1;
	}

	// Start the timer routine, and set the call interval at 17 milliseconds (~ 60 times per second)
	// at this frame rate there may be a few premature or late reads (resulting in skipped or lack of new data)
	// but it is best to try and match the naturalpoint application as closely as possible.
	// 
	// It should be noted that new TrackIR devices send at a rate of 120 times per second.
	MSG msg;
	UINT_PTR id = SetTimer( NULL, 0, 17, (TIMERPROC)&OnTimer);
	while (GetMessage( &msg, NULL, 0, 0 ))
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
	

	// end of program
	KillTimer( NULL, id );
	NP_UnregisterWindowHandle();

	return 0;
}
