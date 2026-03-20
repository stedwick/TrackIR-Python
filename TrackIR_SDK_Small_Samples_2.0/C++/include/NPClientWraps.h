// *******************************************************************************
// * Copyright 2023, NaturalPoint Inc.
// *******************************************************************************
// *
// * Description:
// *   This module implements the wrapper code for interfacing to the TrackIR
// *   Game Client API.  Developers of client apps can include this module into
// *   their projects to simplify communication with the TrackIR software.
// *
// *   This is necessary since the NPClient DLL is run-time linked rather than
// *   load-time linked, avoiding the need to link a static library into the
// *   client program (only this module is needed, and can be supplied in source
// *   form.)
// *
// *******************************************************************************



#pragma once

// check if we are using afx/mfc CString and library loading
#if defined(_AFXDLL)
#include "StdAfx.h"
#else
// otherwise, just import library importing from windows api
#include <windows.h>
#endif

// standard includes
#include <string>
#include "string.h"

// local include
#include "NPClient.h"


// Check if this is 32bit or 64bit
#if _WIN32 || _WIN64
#if _WIN64
#define ENV64BIT
#else
#define ENV32BIT
#endif
#endif


// addresses of function imported from DLL
PF_NP_REGISTERWINDOWHANDLE       gpfNP_RegisterWindowHandle = NULL;
PF_NP_UNREGISTERWINDOWHANDLE     gpfNP_UnregisterWindowHandle = NULL;
PF_NP_REGISTERPROGRAMPROFILEID   gpfNP_RegisterProgramProfileID = NULL;
PF_NP_QUERYVERSION               gpfNP_QueryVersion = NULL;
PF_NP_REQUESTDATA                gpfNP_RequestData = NULL;
PF_NP_GETSIGNATURE               gpfNP_GetSignature = NULL;
PF_NP_GETDATA                    gpfNP_GetData = NULL;
PF_NP_GETDATAEX					 gpfNP_GetDataEX = NULL;
PF_NP_STARTCURSOR                gpfNP_StartCursor = NULL;
PF_NP_STOPCURSOR                 gpfNP_StopCursor = NULL;
PF_NP_RECENTER	                 gpfNP_ReCenter = NULL;
PF_NP_STARTDATATRANSMISSION      gpfNP_StartDataTransmission = NULL;
PF_NP_STOPDATATRANSMISSION       gpfNP_StopDataTransmission = NULL;



HMODULE ghNPClientDLL = (HMODULE)NULL;

/// <summary>
/// All relevant TrackIR SDK functions
/// </summary>
namespace NPClient
{
	
	/// <summary>
	/// Register the application window for liveness checks. This allows the TrackIR software to
	/// detect situations where e.g. your application crashes and fails to shut down cleanly.
	/// </summary>
	/// <remarks>
	/// TrackIR will only communicate with one application at a time.
	/// You must unregister you're window handle after you are done using TrackIR.
	/// </remarks>
	/// <param name="hWnd">Window Handle</param>
	/// <returns>
	/// If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
	/// Otherwise, returns NP_OK if the window handle was successfully registered.
	/// </returns>
	NPRESULT __stdcall NP_RegisterWindowHandle( HWND hWnd )
	{
		NPRESULT result = NP_ERR_DLL_NOT_FOUND;

		if (NULL != gpfNP_RegisterWindowHandle)
			result = ( *gpfNP_RegisterWindowHandle )( hWnd );

		return result;
	} // NP_RegisterWindowHandle()

	/// <summary>
	/// Unregisters a given window handle
	/// </summary>
	/// <returns>
	/// If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
	/// Otherwise, returns NP_OK if the window handle was successfully unregistered.
	/// </returns>
	NPRESULT __stdcall NP_UnregisterWindowHandle()
	{
		NPRESULT result = NP_ERR_DLL_NOT_FOUND;

		if (NULL != gpfNP_UnregisterWindowHandle)
			result = ( *gpfNP_UnregisterWindowHandle )( );

		return result;
	} // NP_UnregisterWindowHandle()

	/// <summary>
	/// Registers a Profile ID to TrackIR so that the application can use it's assigned profile on TrackIR.
	/// </summary>
	/// <remarks>
	/// Every game/application has its own unique Profile ID so that TrackIR can use the tracking profile
	/// when your game is loaded up for the user.
	/// </remarks>
	/// <param name="wPPID">Profile ID</param>
	/// /// <returns>
	/// If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
	/// Otherwise, returns NP_OK if the Profile ID was successfully registered.
	/// </returns>
	NPRESULT __stdcall NP_RegisterProgramProfileID( unsigned short wPPID )
	{
		NPRESULT result = NP_ERR_DLL_NOT_FOUND;

		if (NULL != gpfNP_RegisterProgramProfileID)
			result = ( *gpfNP_RegisterProgramProfileID )( wPPID );

		return result;
	} // NP_RegisterProgramProfileID()

	/// <summary>
	/// Gets the current TrackIR version.
	/// </summary>
	/// <param name="pwVersion">Reference to an unsigned short to store the version</param>
	/// /// <returns>
	/// If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
	/// Otherwise, returns NP_OK if the version was successfully queried.
	/// </returns>
	NPRESULT __stdcall NP_QueryVersion( unsigned short* pwVersion )
	{
		NPRESULT result = NP_ERR_DLL_NOT_FOUND;

		if (NULL != gpfNP_QueryVersion)
			result = ( *gpfNP_QueryVersion )( pwVersion );

		return result;
	} // NP_QueryVersion()

	/// <summary>
	/// Tell TrackIR what kinds of data we are using. 
	/// </summary>
	/// <remarks>
	/// Historically this function was necessary to signal TrackIR to send different types of data. However, 
	/// This is not necessary but good practice and for bookkeeping on TrackIR. This function won't change anything TrackIR sends.
	/// TrackIR will send Roll, Pitch, Yaw, X, Y, Z regardless.
	/// </remarks>
	/// <param name="wDataReq">unsigned short whose bytes tell us what kind of data want</param>
	/// <returns>
	/// If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
	/// Otherwise, returns NP_OK if the data was requested successfully.
	/// </returns>
	NPRESULT __stdcall NP_RequestData( unsigned short wDataReq )
	{
		NPRESULT result = NP_ERR_DLL_NOT_FOUND;

		if (NULL != gpfNP_RequestData)
			result = ( *gpfNP_RequestData )( wDataReq );

		return result;
	} // NP_RequestData()

	/// <summary>
	/// Gets TrackIR's signature from TrackIR.
	/// </summary>
	/// <remarks>
	/// Necessary step to verify that we are receiving data from TrackIR and that the DLL was loaded properly. 
	/// </remarks>
	/// <param name="pSignature">Reference to a TRACKIRSIGNATURE to store the signature given from TrackIR and the DLL</param>
	/// <returns>
	/// If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
	/// Otherwise, returns NP_OK if the version was successfully queried.
	/// </returns>
	NPRESULT __stdcall NP_GetSignature( LPTRACKIRSIGNATURE pSignature )
	{
		NPRESULT result = NP_ERR_DLL_NOT_FOUND;

		if (NULL != gpfNP_GetSignature)
			result = ( *gpfNP_GetSignature )( pSignature );

		return result;
	} // NP_GetSignature()

	/// <summary>
	/// Gets tracking information sent from TrackIR.
	/// </summary>
	/// <param name="pTID">Reference to a TRACKIRDATA to store the tracking information given from TrackIR</param>
	/// <returns>	
	/// If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
	/// Otherwise, returns NP_OK if the tracking data was successfully gathered
	/// </returns>
	NPRESULT __stdcall NP_GetData( LPTRACKIRDATA pTID )
	{
		NPRESULT result = NP_ERR_DLL_NOT_FOUND;

		if (NULL != gpfNP_GetData)
		{
			result = ( *gpfNP_GetData )( pTID );
			if (result == NP_OK)
			{
				return result;
			}
			else{
				memset( pTID, 0, sizeof( TRACKIRDATA ) );
			}
		}

		return result;
	}

	/// <summary>
	/// Gets tracking information sent from TrackIR. This function is only used if you were provided an app key from NaturalPoint.
	/// </summary>
	/// <remarks>
	/// Only a few games were given an app key for sending encrypted data over. It is very likely that you will not need to use this.
	/// </remarks>
	/// <param name="pTID">Reference to a TRACKIRDATA to store the tracking information given from TrackIR</param>
	/// <returns>
	/// If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
	/// Otherwise, returns NP_OK if the tracking data was successfully gathered and decrypted.
	/// </returns>
	NPRESULT __stdcall NP_GetDataEX( LPTRACKIRDATA pTID, long AppKeyHigh, long AppKeyLow )
	{
		NPRESULT result = NP_ERR_DLL_NOT_FOUND;

		if (NULL != gpfNP_GetDataEX)
		{
			result = ( *gpfNP_GetDataEX )( pTID, AppKeyHigh, AppKeyLow );
			if (result == NP_OK)
			{
				return NP_OK;
			}
			else
			{
				memset( pTID, 0, sizeof( TRACKIRDATA ) );
			}
		}

		return result;
	} // NP_GetData()

	/// <summary>
	/// Reports to TrackIR we have started tracking.
	/// </summary>
	/// <remarks>
	/// This function is only for book keeping purposes. This does not do anything on its own. 
	/// </remarks>
	/// <returns>
	/// If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
	/// Otherwise, returns NP_OK if the tracking cursor was started.
	/// </returns>
	NPRESULT __stdcall NP_StartCursor()
	{
		NPRESULT result = NP_ERR_DLL_NOT_FOUND;

		if (NULL != gpfNP_StartCursor)
			result = ( *gpfNP_StartCursor )( );

		return result;
	} // NP_StartCursor()

	/// <summary>
	/// Reports to TrackIR we have stopped tracking.
	/// </summary>
	/// <remarks>
	/// This function is only for book keeping purposes. This does not do anything on its own. 
	/// </remarks>
	/// <returns>
	/// If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
	/// Otherwise, returns NP_OK if the tracking cursor was stopped.
	/// </returns>
	NPRESULT __stdcall NP_StopCursor()
	{
		NPRESULT result = NP_ERR_DLL_NOT_FOUND;

		if (NULL != gpfNP_StopCursor)
			result = ( *gpfNP_StopCursor )( );

		return result;
	} // NP_StopCursor()


	/// <summary>
	/// Re-centers tracking data.
	/// </summary>
	/// <returns>
	/// If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
	/// Otherwise, returns NP_OK if the tracking data was successfully re-centered.
	/// </returns>
	NPRESULT __stdcall NP_ReCenter()
	{
		NPRESULT result = NP_ERR_DLL_NOT_FOUND;

		if (NULL != gpfNP_ReCenter)
			result = ( *gpfNP_ReCenter )( );

		return result;
	} // NP_ReCenter()


	/// <summary>
	/// Signals TrackIR to start sending data to the application.
	/// </summary>
	/// <returns>
	/// If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
	/// Otherwise, returns NP_OK if TrackIR started sending data to the window handle.
	/// </returns>
	NPRESULT __stdcall NP_StartDataTransmission()
	{
		NPRESULT result = NP_ERR_DLL_NOT_FOUND;

		if (NULL != gpfNP_StartDataTransmission)
			result = ( *gpfNP_StartDataTransmission )( );

		return result;
	} // NP_StartDataTransmission()


	/// <summary>
	/// Tells TrackIR to stop sending data to the window handle.
	/// </summary>
	/// <returns>
	/// If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
	/// Otherwise, returns NP_OK if TrackIR started sending data to the window handle.
	/// </returns>
	NPRESULT __stdcall NP_StopDataTransmission()
	{
		NPRESULT result = NP_ERR_DLL_NOT_FOUND;

		if (NULL != gpfNP_StopDataTransmission)
			result = ( *gpfNP_StopDataTransmission )( );

		return result;
	} // NP_StopDataTransmission()



	/// <summary>
	/// Initializes NPClient functions from DLL. Compatible with standard library strings and AFX CStrings.
	/// </summary>
	/// <param name="csDLLPath"> csDLLPath Path to directory containing NPClient(64).dll </param>
	/// <returns>
	///  If DLL is not found and signature not TrackIR is verified, then return NP_ERR_DLL_NOT_FOUND,
	///		Otherwise, returns NP_OK if TrackIR started sending data to the window handle.
	/// <returns>
	// if using MFC/AFX strings
#if defined(_AFXDLL)
	NPRESULT NPClient_Init( CString csDLLPath )
	{
		NPRESULT result = NP_OK;
		CString csNPClientDLLFullPath;
		if (0 != csDLLPath.GetLength())
		{
			csNPClientDLLFullPath = csDLLPath;
		}
#else
	NPRESULT NPClient_Init( std::wstring csDLLPath )
	{
		NPRESULT result = NP_OK;
		std::wstring csNPClientDLLFullPath;
		if (0 != csDLLPath.size())
		{
			csNPClientDLLFullPath = csDLLPath;
		}
#endif


#if defined ENV32BIT
		csNPClientDLLFullPath += L"NPClient.dll";	// 32bit dll

#endif
#if defined ENV64BIT
		csNPClientDLLFullPath += L"NPClient64.dll";	// 64bit dll

#endif

		//[loadLibrarySnippet]
#if defined(_AFXDLL)
		ghNPClientDLL = ::LoadLibrary( (LPCSTR)csNPClientDLLFullPath );
#else
		ghNPClientDLL = ::LoadLibrary( (LPCWSTR)csNPClientDLLFullPath.c_str() );
#endif


		if (NULL != ghNPClientDLL)
		{
			//...
			//[loadLibrarySnippet]


			// [verifySignatureSnippet]
			// verify the dll signature
			gpfNP_GetSignature = ( PF_NP_GETSIGNATURE )::GetProcAddress( ghNPClientDLL, "NP_GetSignature" );

			SIGNATUREDATA pSignature;
			SIGNATUREDATA verifySignature;

			// init the signatures
			strcpy_s( verifySignature.DllSignature, 200, "precise head tracking\n put your head into the game\n now go look around\n\n Copyright EyeControl Technologies" );
			strcpy_s( verifySignature.AppSignature, 200, "hardware camera\n software processing data\n track user movement\n\n Copyright EyeControl Technologies" );
			// query the dll and compare the results
			NPRESULT vresult = NP_GetSignature( &pSignature );
			if (vresult == NP_OK)
			{
				if (( strcmp( verifySignature.DllSignature, pSignature.DllSignature ) == 0 )
					&& ( strcmp( verifySignature.AppSignature, pSignature.AppSignature ) == 0 ))
				{
					// ...
					// [verifySignatureSnippet]
					result = NP_OK;

					// [loadFunctionsSnippet]
					// Get addresses of all exported functions
					gpfNP_RegisterWindowHandle = ( PF_NP_REGISTERWINDOWHANDLE )::GetProcAddress( ghNPClientDLL, "NP_RegisterWindowHandle" );
					gpfNP_UnregisterWindowHandle = ( PF_NP_UNREGISTERWINDOWHANDLE )::GetProcAddress( ghNPClientDLL, "NP_UnregisterWindowHandle" );
					gpfNP_RegisterProgramProfileID = ( PF_NP_REGISTERPROGRAMPROFILEID )::GetProcAddress( ghNPClientDLL, "NP_RegisterProgramProfileID" );
					gpfNP_QueryVersion = ( PF_NP_QUERYVERSION )::GetProcAddress( ghNPClientDLL, "NP_QueryVersion" );
					gpfNP_RequestData = ( PF_NP_REQUESTDATA )::GetProcAddress( ghNPClientDLL, "NP_RequestData" );
					gpfNP_GetData = ( PF_NP_GETDATA )::GetProcAddress( ghNPClientDLL, "NP_GetData" );
					gpfNP_GetDataEX = ( PF_NP_GETDATAEX )::GetProcAddress( ghNPClientDLL, "NP_GetDataEX" );
					gpfNP_StartCursor = ( PF_NP_STARTCURSOR )::GetProcAddress( ghNPClientDLL, "NP_StartCursor" );
					gpfNP_StopCursor = ( PF_NP_STOPCURSOR )::GetProcAddress( ghNPClientDLL, "NP_StopCursor" );
					gpfNP_ReCenter = ( PF_NP_RECENTER )::GetProcAddress( ghNPClientDLL, "NP_ReCenter" );
					gpfNP_StartDataTransmission = ( PF_NP_STARTDATATRANSMISSION )::GetProcAddress( ghNPClientDLL, "NP_StartDataTransmission" );
					gpfNP_StopDataTransmission = ( PF_NP_STOPDATATRANSMISSION )::GetProcAddress( ghNPClientDLL, "NP_StopDataTransmission" );
					// [loadFunctionsSnippet]
				}
				else
				{
					result = NP_ERR_DLL_NOT_FOUND;
				}
			}
			else
			{
				result = NP_ERR_DLL_NOT_FOUND;
			}
		}
		else
			result = NP_ERR_DLL_NOT_FOUND;

		return result;

	}// NPClient_Init()
}