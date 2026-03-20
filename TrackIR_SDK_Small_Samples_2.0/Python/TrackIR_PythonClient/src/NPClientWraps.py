#*******************************************************************************
#* Copyright 2023, NaturalPoint Inc.
#*******************************************************************************
#* Description:
#*
#*    TrackIR Game Client API function wrappers. This should serve as a starting point
#*    to use on Python projects. It contains all needed code for initializing connection,
#*    polling data from, and shutting down connection to TrackIR.
#
#     It attempts to expose as little ctypes as possible for
#*    those that are unfamiliar with it. If you wish to understand this on a deeper level,
#*    you will need some ctypes knowledge.
#* 
#*******************************************************************************

import ctypes
from ctypes import wintypes
import struct
from re import L

import NPClient
from NPClient import *


# <summary>
# Register the application window for liveness checks. This allows the TrackIR software to
# detect situations where e.g. your application crashes and fails to shut down cleanly.
# </summary>
# <remarks>
# TrackIR will only communicate with one application at a time.
# You must unregister you're window handle after you are done using TrackIR.
# </remarks>
# <param name="hWnd">Window Handle</param>
# <returns>
# If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
# Otherwise, returns NP_OK if the window handle was successfully registered.
# </returns>
def RegisterWindowHandle(hWnd: int) -> NPRESULT:
    
    # try the function
    result = NP_RegisterWindowHandle(wintypes.HWND(hWnd))
    
    # check if function has been properly loaded
    if(result == None):
        # issue with dll
        return NPRESULT.NP_ERR_DLL_NOT_FOUND
    
    return result

# <summary>
# Unregisters a given window handle
# </summary>
# <returns>
# If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
# Otherwise, returns NP_OK if the window handle was successfully unregistered.
# </returns>
def UnregisterWindowHandle() -> NPRESULT:
    result = NPRESULT.NP_ERR_DLL_NOT_FOUND
    
    if(NP_UnregisterWindowHandle != None):
        result = NP_UnregisterWindowHandle()
    
    return result



# <summary>
# Registers a Profile ID to TrackIR so that the application can use it's assigned profile on TrackIR.
# </summary>
# <remarks>
# Every game/application has its own unique Profile ID so that TrackIR can use the tracking profile
# when your game is loaded up for the user.
# </remarks>
# <param name="wPPID">Profile ID</param>
# /// <returns>
# If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
# Otherwise, returns NP_OK if the Profile ID was successfully registered.
# </returns>
def RegisterProgramProfileID(wPPID: int) -> NPRESULT:
    
    # python integer must be within the range of a c++ unsigned short
    if(wPPID < 0 or wPPID > 32767) or (not isinstance(wPPID, int)):
        return NPRESULT.NP_ERR_INVALID_ARG
    
    result = NP_RegisterProgramProfileID(wPPID)
    
    if(result == None):
        return NPRESULT.NP_ERR_DLL_NOT_FOUND
    
    return result


# <summary>
# Gets the current TrackIR version.
# </summary>
# <param name="pwVersion">Reference to an unsigned short to store the version</param>
# /// <returns>
# If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
# Otherwise, returns NP_OK if the version was successfully queried.
# </returns>
def QueryVersion() -> NPRESULT | int | int:
    
    # create a unsigned short pointer
    shrt_ptr = ctypes.pointer(ctypes.c_ushort(5))
    
    # outputs a result status and changes the short pointer
    result = NP_QueryVersion(shrt_ptr)
    
    if(result == None):
        return NPRESULT.NP_ERR_DLL_NOT_FOUND
    
    # return result status and decode major and minor versions
    return result, shrt_ptr.contents.value >> 8, shrt_ptr.contents.value & 0x00FF

# <summary>
# Tell TrackIR what kinds of data we are using. 
# </summary>
# <remarks>
# Historically this function was necessary to signal TrackIR to send different types of data. However, 
# This is not necessary but good practice and for bookkeeping on TrackIR. This function won't change anything TrackIR sends.
# TrackIR will send Roll, Pitch, Yaw, X, Y, Z regardless.
# </remarks>
# <param name="wDataReq">unsigned short whose bytes tell us what kind of data want</param>
# <returns>
# If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
# Otherwise, returns NP_OK if the data was requested successfully.
# </returns>
def RequestData(wDataReq: int) -> NPRESULT:
    
    if(wDataReq < 0 or wDataReq > 32767) or (not isinstance(wDataReq, int)):
        return NPRESULT.NP_ERR_INVALID_ARG
    
    result = NP_RequestData(wDataReq)
    
    if(result == None):
        return NPRESULT.NP_ERR_DLL_NOT_FOUND
    
    return result

# <summary>
# Gets TrackIR's signature from TrackIR.
# </summary>
# <remarks>
# Necessary step to verify that we are receiving data from TrackIR and that the DLL was loaded properly. 
# </remarks>
# <returns>
# If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
# Otherwise, returns NP_OK if the version was successfully queried.
# </returns>
def GetSignature() -> NPRESULT | SIGNATUREDATA:

    newsig = SIGNATUREDATA(b"",b"") 
    result = NP_GetSignature(ctypes.pointer(newsig))
    
    if(result == None):
        return NPRESULT.NP_ERR_DLL_NOT_FOUND, None
    
    return result, newsig

# <summary>
# Gets tracking information sent from TrackIR.
# </summary>
# <returns>	
# If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND and a TRACKIRDATA object as None.
# Otherwise, returns NP_OK if the tracking data was successfully gathered and the resulting TRACKIRDATA object.
# </returns>
def GetData() -> NPRESULT | TRACKIRDATA:
    tir = TRACKIRDATA()
    result = NP_GetData(ctypes.pointer(tir))
    if(result == None):
        return NPRESULT.NP_ERR_DLL_NOT_FOUND, None
    
    return result, tir


# <summary>
# Gets tracking information sent from TrackIR. This function is only used if you were provided an app key from NaturalPoint.
# </summary>
# <remarks>
# Only a few games were given an app key for sending encrypted data over. It is very likely that you will not need to use this.
# </remarks>
# <returns>
# If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND and a TRACKIRDATA object as None.
# Otherwise, returns NP_OK if the tracking data was successfully gathered and the resulting TRACKIRDATA object.
# </returns>
def GetDataEX(appKeyHigh: int, appKeyLow: int) -> NPRESULT | TRACKIRDATA:

    tir = TRACKIRDATA()
    pTID: ctypes.POINTER(tir)
    result = NP_GetDataEX(pTID, ctypes.c_long(appKeyHigh), ctypes.c_long(appKeyLow))

    if(result == None):
        return NPRESULT.NP_ERR_DLL_NOT_FOUND, None
    
    return result, tir

# <summary>
# Reports to TrackIR we have started tracking.
# </summary>
# <remarks>
# This function is only for book keeping purposes. This does not do anything on its own. 
# </remarks>
# <returns>
# If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
# Otherwise, returns NP_OK if the tracking cursor was started.
# </returns>
def StartCursor() -> NPRESULT:

    result = NP_StartCursor()
    
    if(result == None):
        return NPRESULT.NP_ERR_DLL_NOT_FOUND
    
    return result

# <summary>
# Reports to TrackIR we have stopped tracking.
# </summary>
# <remarks>
# This function is only for book keeping purposes. This does not do anything on its own. 
# </remarks>
# <returns>
# If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
# Otherwise, returns NP_OK if the tracking cursor was stopped.
# </returns>
def StopCursor() -> NPRESULT:
    
    result = NP_StopCursor()
    if(result == None):
        return NPRESULT.NP_ERR_DLL_NOT_FOUND
    
    return result

# <summary>
# Recenters tracking data.
# </summary>
# <returns>
# If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
# Otherwise, returns NP_OK if the tracking data was successfully recentered.
# </returns>
def ReCenter() -> NPRESULT:
    
    result = NP_ReCenter()
    
    if(result == None):
        return NPRESULT.NP_ERR_DLL_NOT_FOUND
    
    return result

# <summary>
# Signals TrackIR to start sending data to the application.
# </summary>
# <returns>
# If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
# Otherwise, returns NP_OK if TrackIR started sending data to the window handle.
# </returns>
def StartDataTransmission() -> NPRESULT:
    
    result = NP_StartDataTransmission()
    
    if(result == None):
        return NPRESULT.NP_ERR_DLL_NOT_FOUND
    
    return result

# <summary>
# Tells TrackIR to stop sending data to the window handle.
# </summary>
# <returns>
# If function was not loaded from DLL, it returns NP_ERR_DLL_NOT_FOUND.
# Otherwise, returns NP_OK if TrackIR started sending data to the window handle.
# </returns>
def StopDataTransmission() -> NPRESULT:
    
    result = NP_StopDataTransmission()
    
    if(result == None):
        return NPRESULT.NP_ERR_DLL_NOT_FOUND
    
    return result


# <summary>
# Initializes NPClient functions from DLL.
# </summary>
# <returns>
#  If DLL is not found and signature not TrackIR is verified, then return NP_ERR_DLL_NOT_FOUND,
# 	Otherwise, returns NP_OK if TrackIR started sending data to the window handle.
# <returns>
def NPClient_init():
    
    # check if system is 64bit 
    is64bit = True if ctypes.sizeof(ctypes.c_void_p) * 8 == 64 else False

    try:
        reg = winreg.ConnectRegistry(None, winreg.HKEY_CURRENT_USER)
        npLocKey = winreg.OpenKey(reg, r'Software\\NaturalPoint\\NATURALPOINT\\NPClient Location')
        npLocValue = winreg.QueryValueEx(npLocKey, "Path")[0]
        dllName = "\\NPClient64.dll" if is64bit else "\\NPClient.dll"
        NPClient.NP_DLL = ctypes.WinDLL(npLocValue + dllName)
    except:
        print("Could Not Find NPRESULT.NP_ERR_DLL_NOT_FOUND")

    # check if the dll is working 
    if(NPClient.NP_DLL == None):
        return NPRESULT.NP_ERR_DLL_NOT_FOUND
    
    # this is what the TrackIR application should be sending us 
    dllsig = b"precise head tracking\n put your head into the game\n now go look around\n\n Copyright EyeControl Technologies"
    appsig = b"hardware camera\n software processing data\n track user movement\n\n Copyright EyeControl Technologies"
    
    sig = SIGNATUREDATA(dllsig, appsig)


    result, newsig = GetSignature()
    
    # check if the function has been properly loaded or not
    if(result != NPRESULT.NP_OK):
        return NPRESULT.NP_ERR_DLL_NOT_FOUND
    
    # if signatures don't match, then the dll isn't valid
    if newsig != sig:
        return NPRESULT.NP_ERR_DLL_NOT_FOUND
    
    return NPRESULT.NP_OK

    
    

    
    
