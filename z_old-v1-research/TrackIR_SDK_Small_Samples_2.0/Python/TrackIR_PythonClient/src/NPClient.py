#*******************************************************************************
#* Copyright 2023, NaturalPoint Inc.
#*******************************************************************************
#* Description:
#*
#*   Contains TrackIR Game Client API function prototypes, data structures, and constants.
#*   Heavily utilizes the CTypes library to connect to the C++ code imported from the NPClient DLL.
#* 
#*******************************************************************************

import ctypes
from ctypes import wintypes
import winreg
from enum import Enum, auto


# possible values wNPStatus can be
NPSTATUS_REMOTEACTIVE = 0       # TrackIR remote enabled (device is ready for game use)
NPSTATUS_REMOTIVEISABLED = 1    # mouse emulation on (don't use in game camera, this is just for mouse cursor control)



# TrackIR globals  useful for converting between TIR (TrackIR) units

NP_MAX_ROTATION	 = 180		# +/- 180.0 degrees
NP_MAX_TRANSLATION = 50		# +/- 50.0 cms
NP_MAX_VALUE = 16383    
NP_MIN_VALUE = -16383    


# Constants representing the bits you need to set for NP_RequestData
# roll, pitch, yaw
NPPitch = 2	
NPYaw = 4	
NPRoll = 1	

# x, y, z 
NPX = 16
NPY = 32
NPZ = 64



# Return codes used by TrackIR functions.
class NPRESULT(Enum):
                                     
    NP_OK = 0
    NP_ERR_DEVICE_NOT_PRESENT = auto()
    NP_ERR_UNSUPPORTED_OS = auto()
    NP_ERR_INVALID_ARG = auto()
    NP_ERR_DLL_NOT_FOUND = auto()
    NP_ERR_NO_DATA = auto()
    NP_ERR_INTERNAL_DATA = auto()
    NP_ERR_ALREADY_REGISTERED = auto()  # a window handle or game ID is already registered
    NP_ERR_UNKNOWN_ID = auto()          # unknown game ID registered
    NP_ERR_READ_ONLY = auto()           # parameter is read only

    NP_ERR_FAILED = 100
    NP_ERR_INVALID_KEY = auto()

# Different states the TrackIR camera could be in. Read-only.
class NPPARAMETER(Enum):
    NP_PARAM_TRACKING = auto()         # Tracking state for the camera 
    NP_PARAM_PAUSED = auto()           # Paused state for the camera (F9 in the TrackIR app).
    NP_PARAM_CENTERED = auto()         # centered (F12 in the TrackIR app). // TrackIR app sets this flag, client can read and clear once handled. TrackIR app sets this flag, client can read and clear once handled  

 
 # This structure contains strings set by both the TrackIR Enhanced DLL and the TrackIR application itself. 
 # Used for integrity checking.
class SIGNATUREDATA(ctypes.Structure):
    _fields_ = [("DLLSignature", ctypes.c_char * 200),  # Signature set by DLL once initialized.
                ("AppSignature", ctypes.c_char * 200)]  # Signature set by TrackIR application once initialized.
    
    def __eq__(self, other):
        result = getattr(self, "DLLSignature") == getattr(other, "DLLSignature") \
            and getattr(self, "AppSignature") == getattr(other, "AppSignature")
        
        return result
    
    def __str__(self):
        return str(getattr(self, "DLLSignature")) + str(getattr(self, "AppSignature"))

 
# 
# This structure represents a single frame of head tracking data returned by the NP_GetData function.

# For historical reasons, the TrackIR SDK uses a non-standard, "hybrid" coordinate system, with
# left-handed basis vectors - from the tracked user's perspective, facing the display + TrackIR device:
# +X = Left
# +Y = Up
# +Z = Back
# And right-hand rule rotations about those vectors:
# Pitch = Rotation about X, increasing as the user looks down.
# Yaw = Rotation about Y, increasing as the user turns left.
# Roll = Rotation about Z, increasing as the user tilts left.
# The X, Y, and Z members define the position.
# 
# The values range from -16383.0f to 16383.0f, and map to a range
# of -50 cm to +50 cm.
# The Roll, Pitch, and Yaw members represent orientation as an intrinsic rotation using Tait–Bryan angles.
# To be consistent with the TrackIR 5 software view port, they should be applied in the order z-y'-x'' (R-Y-P).
# The values range from -16383.0f to 16383.0f, and map to a range of -180 degrees to +180 degrees.
class TRACKIRDATA(ctypes.Structure):
    
    _fields_ = [("Status", ctypes.c_ushort),                # Tells us whether or not the TrackIR camera is in mouse-emulation mode or not
                ("FrameSignature", ctypes.c_ushort),        # Incrementing frame number coming from the TrackIR app used to verify if the incoming frame is new. Ranges from 1 to 32766 (will loop)
                ("IOData", ctypes.c_ulong),                 # only used for hash key encryption on a few games. It is likely you won't need to use this.
                ("Roll", ctypes.c_float),                   # Roll component of head pose in TIR rotation units.
                ("Pitch", ctypes.c_float),                  # Pitch component of head pose in TIR rotation units.
                ("Yaw", ctypes.c_float),                    # Yaw component of head pose in TIR rotation units
                ("X", ctypes.c_float),                      # X component of head position in TIR units
                ("Y", ctypes.c_float),                      # Y component of head position in TIR units	
                ("Z", ctypes.c_float),                      # Z component of head position in TIR units
                ("Reserved1", ctypes.c_float),              # Unused field
                ("Reserved2", ctypes.c_float),              # Unused field
                ("Reserved3", ctypes.c_float),              # Unused field
                ("Reserved4", ctypes.c_float),              # Unused field
                ("Reserved5", ctypes.c_float),              # Unused field
                ("Reserved6", ctypes.c_float),              # Unused field
                ("Reserved7", ctypes.c_float),              # Unused field
                ("Reserved8", ctypes.c_float),              # Unused field
                ("Reserved9", ctypes.c_float)]              # Unused field

    def __init__(self):
          for field in self._fields_:
              setattr(self, field[0], 0)

    def __eq__(self, other):
        
        for field in self._fields_:
            
            if getattr(self, field[0]) != getattr(other, field[0]) :
                return False
            
        return True

    def __str__(self):
        str = ""
        for field in self._fields_:
            str += "field " + field[0] + "\n"
        return str
            

#######################################################################
# Initializing DLL and DLL imported functions with Ctypes library
#######################################################################

NP_DLL = None

# <summary>
# Typedef for pointer to the notify callback function that is implemented within
# the client -- this function receives head tracker reports from the game client API
# </summary>
PF_NOTIFYCALLBACK = ctypes.CFUNCTYPE(NPRESULT, ctypes.c_ushort, ctypes.c_ushort)


def NP_RegisterWindowHandle(hWnd):

    if(NP_DLL == None):
        return None


    return_type = NPRESULT
    arg_types = [wintypes.HWND]
    func = NP_DLL.NP_RegisterWindowHandle
    func.argtypes = arg_types
    func.restype = return_type
    return func(hWnd)

def NP_UnregisterWindowHandle():

    if(NP_DLL == None):
        return None

    return_type = NPRESULT
    arg_types = []
    func = NP_DLL.NP_UnregisterWindowHandle
    func.argtypes = arg_types
    func.restype = return_type
    return func()

def NP_RegisterProgramProfileID(wPPID):

    if(NP_DLL == None):
        return None

    return_type = NPRESULT
    arg_types = [ctypes.c_ushort]
    func = NP_DLL.NP_RegisterProgramProfileID
    func.argtypes = arg_types
    func.restype = return_type
    return func(wPPID)

def NP_QueryVersion(pwVersion):

    if(NP_DLL == None):
        return None

    return_type = NPRESULT
    arg_types = [ctypes.POINTER(ctypes.c_ushort)]
    func = NP_DLL.NP_QueryVersion
    func.argtypes = arg_types
    func.restype = return_type
    return func(pwVersion)

def NP_RequestData(wDataReq):

    if(NP_DLL == None):
        return None

    return_type = NPRESULT
    arg_types = [ctypes.c_ushort]
    func = NP_DLL.NP_RequestData
    func.argtypes = arg_types
    func.restype = return_type
    return func(wDataReq)

def NP_GetSignature(pSignature):

    if(NP_DLL == None):
        return None

    return_type = NPRESULT
    arg_types = [ctypes.POINTER(SIGNATUREDATA)]
    func = NP_DLL.NP_GetSignature
    func.argtypes = arg_types
    func.restype = return_type
    return func(pSignature)

def NP_GetData(pTID):

    if(NP_DLL == None):
        return None

    return_type = NPRESULT
    arg_types = [ctypes.POINTER(TRACKIRDATA)]
    func = NP_DLL.NP_GetData
    func.argtypes = arg_types
    func.restype = return_type
    return func(pTID)

def NP_GetDataEX(pTID, AppKeyHigh, AppKeyLow):

    if(NP_DLL == None):
        return None

    return_type = NPRESULT
    arg_types = [ctypes.POINTER(TRACKIRDATA), ctypes.c_long, ctypes.c_long]
    func = NP_DLL.NP_GetDataEX
    func.argtypes = arg_types
    func.restype = return_type
    return func(pTID, AppKeyHigh, AppKeyLow)

def NP_RegisterNotify(pfNotify):

    if(NP_DLL == None):
        return None

    return_type = NPRESULT
    arg_types = [PF_NOTIFYCALLBACK]
    func = NP_DLL.NP_RegisterNotify
    func.argtypes = arg_types
    func.restype = return_type
    return func(pfNotify)

def NP_UnregisterNotify():

    if(NP_DLL == None):
        return None

    return_type = NPRESULT
    arg_types = []
    func = NP_DLL.NP_UnregisterNotify
    func.argtypes = arg_types
    func.restype = return_type
    return func()

def NP_StartCursor():

    if(NP_DLL == None):
        return None

    return_type = NPRESULT
    arg_types = []
    func = NP_DLL.NP_StartCursor
    func.argtypes = arg_types
    func.restype = return_type
    return func()

def NP_StopCursor():

    if(NP_DLL == None):
        return None

    return_type = NPRESULT
    arg_types = []
    func = NP_DLL.NP_StopCursor
    func.argtypes = arg_types
    func.restype = return_type
    return func()

def NP_ReCenter():

    if(NP_DLL == None):
        return None

    return_type = NPRESULT
    arg_types = []
    func = NP_DLL.NP_ReCenter
    func.argtypes = arg_types
    func.restype = return_type
    return func()

def NP_StartDataTransmission():

    if(NP_DLL == None):
        return None

    return_type = NPRESULT
    arg_types = []
    func = NP_DLL.NP_StartDataTransmission
    func.argtypes = arg_types
    func.restype = return_type
    return func()

def NP_StopDataTransmission():

    if(NP_DLL == None):
        return None

    return_type = NPRESULT
    arg_types = []
    func = NP_DLL.NP_StopDataTransmission
    func.argtypes = arg_types
    func.restype = return_type
    return func()

def NP_GetParameter(param, DWORD, pdwValue):

    if(NP_DLL == None):
        return None

    return_type = NPRESULT
    arg_types = [NPPARAMETER, ctypes.POINTER(wintypes.DWORD)]
    func = NP_DLL.NP_GetParameter
    func.argtypes = arg_types
    func.restype = return_type
    return func(param, DWORD, pdwValue)

def NP_SetParameter(param, DWORD):

    if(NP_DLL == None):
        return None

    return_type = NPRESULT
    arg_types = [NPPARAMETER, wintypes.DWORD]
    func = NP_DLL.NP_SetParameter
    func.argtypes = arg_types
    func.restype = return_type
    return func(param, DWORD)

