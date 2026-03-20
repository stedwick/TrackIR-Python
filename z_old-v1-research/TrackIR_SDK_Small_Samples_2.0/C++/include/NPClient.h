// *******************************************************************************
// * Copyright 2023, NaturalPoint Inc.
// *******************************************************************************
// * Description:
// *
// *   Definitions for all structs, functions, constants, and enums that are used
// *   in the TrackIR Game Client API.
// * 
// *******************************************************************************

#pragma once

#pragma pack( push, npclient_h ) // Save current pack value
#pragma pack(1)

//////////////////
// Defines //////////////////////////////////////////////////////////////////////
/////////////////


/// <summary>
/// possible wNPStatus values.
/// REMOTIVE_ACTIVE signals that this device is ready for use in game.
/// NPSTATUS_REMOTEDISABLED signals that this device is using the mouse emulation software (cursor control).
/// </summary>
# define	NPSTATUS_REMOTEACTIVE	0

#define	NPSTATUS_REMOTEDISABLED	1

//////////////
// DATA FIELDS
//////////////

/// <summary>
/// Definitions useful for converting between TIR (TrackIR) units
/// and angles (Tait-Bryan) and centimeters.
/// </summary>
#define NP_MAX_ROTATION			180.0f		///< +/- 180.0 degrees
#define NP_MAX_TRANSLATION		50.0f		///< +/- 50.0 cms
#define NP_MAX_VALUE            16383.0f    ///< upper bound of TIR units
#define NP_MIN_VALUE            -16383.0f   ///< lower bound of TIR units


/// <summary>
/// Definitions, in bytes, to use for requesting types of TrackIR tracking data.
/// </summary>
/// <remarks>
/// This is used for 
/// </remarks>
#define	NPRoll		1
#define	NPPitch		2
#define	NPYaw		4
		
#define	NPY			32
#define	NPX			16
#define	NPZ			64


//////////////////
// Typedefs /////////////////////////////////////////////////////////////////////
/////////////////

/// <summary>
/// Return codes used by TrackIR functions.
/// </summary>
typedef enum tagNPResult
{
	NP_OK = 0,
	NP_ERR_DEVICE_NOT_PRESENT,
	NP_ERR_UNSUPPORTED_OS,
	NP_ERR_INVALID_ARG,
	NP_ERR_DLL_NOT_FOUND,
	NP_ERR_NO_DATA,
	NP_ERR_INTERNAL_DATA,
	NP_ERR_ALREADY_REGISTERED, 
	NP_ERR_UNKNOWN_ID, 
	NP_ERR_READ_ONLY,           // parameter is read only
	NP_ERR_FAILED = 100,
	NP_ERR_INVALID_KEY
 
} NPRESULT;

/// <summary>
/// Different states the TrackIR camera could be in. Read-only.
/// </summary>
typedef enum tagNPParameter
{	

	NP_PARAM_TRACKING,			///< Tracking state for the camera 
	
	NP_PARAM_PAUSED,			///< Paused state for the camera (F9 in the TrackIR app).
	
	NP_PARAM_CENTERED,         	///< centered (F12 in the TrackIR app). // TrackIR app sets this flag, client can read and clear once handled
    
} NPPARAMETER;

/// <summary>
/// This structure contains strings set by both the TrackIR Enhanced DLL and the TrackIR application itself. Used
/// for integrity checking.
/// </summary>
typedef struct tagTrackIRSignature
{
	
	char DllSignature[200];	///< Signature set by DLL once initialized.

	char AppSignature[200];	///< Signature set by TrackIR application once initialized.

} SIGNATUREDATA, *LPTRACKIRSIGNATURE;

/// <summary>
/// This structure represents a single frame of head tracking data returned by the NP_GetData function.
/// </summary>
/// <remarks>
/// For historical reasons, the TrackIR SDK uses a non-standard, "hybrid" coordinate system, with
/// left-handed basis vectors - from the tracked user's perspective, facing the display + TrackIR device:
/// +X = Left
/// +Y = Up
/// +Z = Back
/// And right-hand rule rotations about those vectors:
/// Pitch = Rotation about X, increasing as the user looks down.
/// Yaw = Rotation about Y, increasing as the user turns left.
/// Roll = Rotation about Z, increasing as the user tilts left.
/// The X, Y, and Z members define the position.
/// 
/// The values range from -16383.0f to 16383.0f, and map to a range
/// of -50 cm to +50 cm.
/// The Roll, Pitch, and Yaw members represent orientation as an intrinsic rotation using Tait–Bryan angles.
/// To be consistent with the TrackIR 5 software view port, they should be applied in the order z-y'-x'' (R-Y-P).
/// The values range from -16383.0f to 16383.0f, and map to a range of -180 degrees to +180 degrees.
/// 
/// </remarks>   
typedef struct tagTrackIRData
{

	unsigned short Status;	///< Tells us whether or not the TrackIR camera is in mouse-emulation mode or not

	unsigned short FrameSignature; ///< Incrementing frame number coming from the TrackIR app used to verify if the incoming frame is new. Ranges from 1 to 32766 (will loop)

	unsigned long  IOData;	///< only used for hash key encryption on a few games. It is likely you won't need to use this.


	float Roll;		///< Roll component of head pose in TIR rotation units.
	float Pitch;	///< Pitch component of head pose in TIR rotation units.
	float Yaw;		///< Yaw component of head pose in TIR rotation units	


	float X; ///< X component of head position in TIR units
	float Y; ///< Y component of head position in TIR units	
	float Z; ///< Z component of head position in TIR units


	float reserved1; ///< Unused field 
	float reserved2; ///< Unused field 
	float reserved3; ///< Unused field 
	float reserved4; ///< Unused field
	float reserved5; ///< Unused field
	float reserved6; ///< Unused field
	float reserved7; ///< Unused field
	float reserved8; ///< Unused field
	float reserved9; ///< Unused field

} TRACKIRDATA, *LPTRACKIRDATA;

/// <summary>
/// Typedef for pointer to the notify callback function that is implemented within
/// the client -- this function receives head tracker reports from the game client API
/// </summary>
typedef NPRESULT (__stdcall *PF_NOTIFYCALLBACK)( unsigned short, unsigned short );


// Typedefs for client functions (useful for declaring pointers to these
// functions within the client for use during GetProcAddress() ops)
typedef NPRESULT (__stdcall *PF_NP_REGISTERWINDOWHANDLE)( HWND );
typedef NPRESULT (__stdcall *PF_NP_UNREGISTERWINDOWHANDLE)( void );
typedef NPRESULT (__stdcall *PF_NP_REGISTERPROGRAMPROFILEID)( unsigned short );
typedef NPRESULT (__stdcall *PF_NP_QUERYVERSION)( unsigned short* );
typedef NPRESULT (__stdcall *PF_NP_REQUESTDATA)( unsigned short );
typedef NPRESULT (__stdcall *PF_NP_GETSIGNATURE)( LPTRACKIRSIGNATURE);
typedef NPRESULT( __stdcall* PF_NP_GETDATA )( LPTRACKIRDATA );
typedef NPRESULT( __stdcall* PF_NP_GETDATAEX )( LPTRACKIRDATA, long, long );
typedef NPRESULT (__stdcall *PF_NP_REGISTERNOTIFY)( PF_NOTIFYCALLBACK );
typedef NPRESULT (__stdcall *PF_NP_UNREGISTERNOTIFY)( void );
typedef NPRESULT (__stdcall *PF_NP_STARTCURSOR)( void );
typedef NPRESULT (__stdcall *PF_NP_STOPCURSOR)( void );
typedef NPRESULT (__stdcall *PF_NP_RECENTER)( void );
typedef NPRESULT (__stdcall *PF_NP_STARTDATATRANSMISSION)( void );
typedef NPRESULT (__stdcall *PF_NP_STOPDATATRANSMISSION)( void );
typedef NPRESULT (__stdcall *PF_NP_GETPARAMETER)( NPPARAMETER, DWORD * );
typedef NPRESULT (__stdcall *PF_NP_SETPARAMETER)( NPPARAMETER, DWORD );

/// Function Prototypes ///////////////////////////////////////////////
/// <summary>
///  Functions exported from NPClient DLL 
/// </summary>
/// __stdcall calling convention is used for ease of interface to clients of differing implementations including
/// C, C++, Pascal (Delphi) and VB. )
namespace NPClient
{

	NPRESULT __stdcall NP_RegisterWindowHandle( HWND hWnd );
	NPRESULT __stdcall NP_UnregisterWindowHandle( void );
	NPRESULT __stdcall NP_RegisterProgramProfileID( unsigned short wPPID );
	NPRESULT __stdcall NP_QueryVersion( unsigned short* pwVersion );
	NPRESULT __stdcall NP_RequestData( unsigned short wDataReq );
	NPRESULT __stdcall NP_GetSignature( LPTRACKIRSIGNATURE pSignature );
	NPRESULT __stdcall NP_GetData( LPTRACKIRDATA pTID );
	NPRESULT __stdcall NP_GetDataEX( LPTRACKIRDATA pTID, long AppKeyHigh, long AppKeyLow );
	NPRESULT __stdcall NP_StartCursor( void );
	NPRESULT __stdcall NP_StopCursor( void );
	NPRESULT __stdcall NP_ReCenter( void );
	NPRESULT __stdcall NP_StartDataTransmission( void );
	NPRESULT __stdcall NP_StopDataTransmission( void );

	NPRESULT __stdcall NP_RegisterNotify( PF_NOTIFYCALLBACK pfNotify );
	NPRESULT __stdcall NP_UnregisterNotify( void );
	NPRESULT __stdcall NP_GetParameter( NPPARAMETER param, DWORD* pdwValue );
	NPRESULT __stdcall NP_SetParameter( NPPARAMETER param, DWORD dwValue );
}

#pragma pack( pop, npclient_h ) // Ensure previous pack value is restored

//
// *** End of file: NPClient.h ***
//