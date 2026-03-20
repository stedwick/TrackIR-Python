#*******************************************************************************
#* Copyright 2023, NaturalPoint Inc.
#*******************************************************************************
#* Description:
#*
#*     Simple Python GUI code to display TrackIR functionality. Outputs tracking data
#*     to text field. 
#* 
#*******************************************************************************

import tkinter
import ctypes
import NPClientWraps as npc
from NPClient import *

import tkinter as tk


# If you were one of the few given an app key, please read the documentation about using app keys
# developer ID provided by NaturalPoint for your game/app. The SDK developer ID is available to everyone.
NP_DEVELOPER_ID = 1000        #== Developer ID =============--

class cHeadTrackingGUI:

    # create GUI
    def __init__(self, master):

        self.master = master
        self.master.title("TrackIR SDK - Python Client")
        self.frame = tk.Frame(self.master)

        self.devID = NP_DEVELOPER_ID  # SDK profile

        # enable getdata loop
        self.readData = True
        # keep track of no new data calls
        self.timerMsgNum = 0
        # previous frame signature
        self.frameSig = 0

        self.scrollbar = tk.Scrollbar(self.frame,orient=tk.VERTICAL)
        self.scrollbar.grid(row=1, rowspan=3,column=0, sticky='ens');

        # Create tracking information text field
        self.trackingInfoTopText = tk.Text(self.frame, width=80, height=1)
        self.trackingInfoTopText.grid(row=0, padx=10, pady=10)
        self.trackingInfoText = tk.Listbox(
            self.frame, yscrollcommand = self.scrollbar.set, state=tk.NORMAL)

        self.trackingInfoText.grid(
            row=1, column=0, rowspan=3, sticky='ew', padx=10, pady=5)

        self.scrollbar.config( command = self.trackingInfoText.yview )

        # Create start and stop buttons
        self.startButton = tk.Button(
            self.frame, text="Start", command=self.StartCursor)
        self.startButton.grid(row=1, column=1, sticky='ew')
        self.stopButton = tk.Button(
            self.frame, text="Stop", command=self.StopCursor, state=tk.DISABLED)
        self.stopButton.grid(row=1, column=2, sticky='ew', padx=10)

        # Create register and unregister buttons
        self.registerButton = tk.Button(
            self.frame, text="Register", command=self.Register)
        self.registerButton.grid(row=2, column=1, sticky='ew')
        self.unregisterButton = tk.Button(
            self.frame, text="Unregister", command=self.Unregister)
        self.unregisterButton.grid(row=2, column=2, sticky='ew', padx=10)

        # Create Submit devID button
        self.submitButton = tk.Button(
            self.frame, text="Submit\n DevID", command=self.SubmitDevID)
        self.submitButton.grid(row=3, column=1, sticky='ew')

        # devID input field
        devidValidateCmd = (self.master.register(self.IsValidInput), '%P')
        self.devidEntry = tk.Entry(
            self.frame, validate='key', validatecommand=devidValidateCmd, width=8)
        self.devidEntry.insert(tk.INSERT, "1000")
        self.devidEntry.grid(row=3, column=2, sticky='ew', padx=10)

        self.frame.pack()

        # Create timer for updating tracking information
        self.timer_id = None

    # start/continue timer routine for getting tracking information from TrackIR.
    # TrackIR sends at a rate of ~120fps, but older ones send at a rate of 60fps.
    # It's best to poll at the rate at which TrackIR sends, but it will be fine
    # to get data whenever the application is ready/every frame update.
    def StartTimer(self):
        # update ever 17ms roughly ~60fps
        self.timerId = self.master.after(17, self.UpdateTrackingInfo)

    # display message queue on a timer for updating
    def UpdateTrackingInfo(self):

        # Update tracking information
        self.GetDataLoop()

        # only continue loop if we can read data and the start button has been pressed
        if(self.readData and self.startButton["state"] == tk.DISABLED ):
            self.timerId = self.master.after(17, self.UpdateTrackingInfo)

        

    # start tracking
    def StartCursor(self):

        result = npc.StartCursor()

        if (result == NPRESULT.NP_OK):

            self.AddLine("NPClient : Starting cursor")
            self.startButton.config(state=tk.DISABLED)
            self.stopButton.config(state=tk.NORMAL)
            self.readData = True
            self.StartTimer()
            

        else:
            self.AddLine("NPClient : Error starting cursor")

    # stop tracking  (does not close connection with TrackIR)
    def StopCursor(self):

        result = npc.StopCursor()

        if (result == NPRESULT.NP_OK):

            self.AddLine("NPClient : Stopped cursor")
            self.stopButton.config(state=tk.DISABLED)
            self.startButton.config(state=tk.NORMAL)
            self.readData = False

        else:

            self.AddLine("NPClient : Error stopping cursor")

    # register window handle for TrackIR to automatically sense when you close the application
    def Register(self):

        result = self.ClientInit()
        if (result == NPRESULT.NP_OK):
            self.StartCursor()

    # unregister window handle.
    def Unregister(self):

        result = npc.UnregisterWindowHandle()

        if (result == NPRESULT.NP_OK):
            self.AddLine("NPClient : Unregistered window handle successfully.")

            self.StopCursor()
        else:
            self.AddLine("NPClient : Error unregistering window handle")

    # Submit the developer ID NaturalPoint provided you (or SDK dev ID 1000) to access
    # your game's profile and to work with TrackIR.
    def SubmitDevID(self):

        # get() function can throw error if nothing is in the text box
        try:
            self.devID = self.devidEntry.get()

            result = npc.RegisterProgramProfileID(int(self.devID))
            if (result == NPRESULT.NP_OK):
                self.AddLine(
                    "NPClient : Registered program profile ID successfully")
            else:
                self.AddLine(
                    "NPclient : There was an error registering the program profile ID.")
                self.AddLine(
                    "        The program may already be communicating with the TrackIR software.")
                self.AddLine(
                    "        If there is a problem, try unregistering, submit the DevID, then register again.")

        except:
            self.AddLine("Please insert a valid ID")

    # Display line on the top text box
    def AddTopLine(self, line: str):
        if (isinstance(line, str)):
            self.trackingInfoTopText.delete("1.0", tk.END)
            self.trackingInfoTopText.insert(tk.INSERT, line + "\n")

    # Append line of text to main text box
    def AddLine(self, line: str):
        # check if we are sending a string
        if (isinstance(line, str)):
            self.trackingInfoText.insert(tk.END, line)
            self.trackingInfoText.see(tk.END)


    # check if the devID is an integer
    @staticmethod
    def IsValidInput(input):
        if input.isdigit():
            return True
        elif input == "":
            return True
        else:
            return False

    # <summary>
    # Shutdown procedure for TrackIR consists of 2 steps.
    # 1. stop data transmission
    # 2. unregister window handle
    # </summary>
    # <remarks>
    # TrackIR will automatically detect through the window handle if you 
    # close the application. However, this step is still good practice 
    # just to make sure everything happens the way you want it to happen.
    # </remarks>
    def OnExit(self):
        npc.NP_StopDataTransmission()
        npc.UnregisterWindowHandle()
        self.master.destroy()

    # <summary> Initialize the NPClient interface to start recieving data. </summary>
    # <remarks>
    # This function demonstrates how to communicate with TrackIR to start receiving tracking data.
    # 1. We locate and load the code from NPClient(64).dll. See \ref NPClient::NPClient_Init and \ref FindDllLocation.
    # 2. Then, we register the console's window handle so that TrackIR can start communicating with the console. \ref NPClient::NP_RegisterWindowHandle
    # and \ref GetConsoleHwnd.
    # 3. Next, we get the TrackIR version that is being used, and request the types of data we want to receive.
    #  See \ref NPClient::NP_RequestData
    # 4. Lastly, we tell TrackIR we are ready to start receiving data and start processing tracking data.
    # </remarks>
    def ClientInit(self):
        devID = self.devID

        result = NPRESULT.NP_OK

        # check if dll is loaded correctly
        npc.NPClient_init()

        if (result == NPRESULT.NP_OK):
            self.AddLine("NPClient interface -- initialize OK.")
        else:
            self.AddLine("Error initializing NPClient interface!!")
            return result

        # register the window's window handle
        result = npc.RegisterWindowHandle(root.winfo_id())

        if (result == NPRESULT.NP_OK):
            self.AddLine("NPClient : Window handle registration successful.")
        else:
            self.AddLine("NPClient : Error registering window handle")
            return result

        # get the version of trackIR in format of verMajor.verMinor
        result, verMajor, verMinor = npc.QueryVersion()

        if (result == NPRESULT.NP_OK):
            self.AddLine("NaturalPoint software version is " +
                         str(verMajor) + "." + str(verMinor))
        else:
            self.AddLine(
                "NPClient : Error querying NaturalPoint software version!!")
            return result

        # Set bits for telling TrackIR what types of data we want to use. Mostly for bookkeeping on TrackIR's end.
        # TrackIR will still send all 6DOF tracking data regardless

        dataFields = 0             
        dataFields |= npc.NPPitch
        dataFields |= npc.NPYaw
        dataFields |= npc.NPRoll

        result = npc.RequestData(dataFields)


        # register your game's ID to access your game settings in the TrackIR client
        result = npc.RegisterProgramProfileID(int(devID))


        # Tell TrackIR we are ready to start receiving data
        result = npc.StartDataTransmission()
        if (result == NPRESULT.NP_OK):
            self.AddLine("Data transmission started")
        else:
            self.AddLine("NPClient : Error starting data transmission")

        self.StartCursor()

  
    # <summary>
    # Processes data received from TrackIR.
    # </summary>
    # <remarks>
    # This is a simple loop controlled by a timer called once every 17ms or 60fps (although, TrackIR sends at a rate of 120fps). 
    # It consists of the following steps:
    # 1. Call NP_GetData and see if the function returned NP_OK
    # 2. Check if TrackIR is not paused, and check if we received a new frame based on the frame signature
    # 3. Convert data from TrackIR to degrees and cms in a left-handed coordinate basis, and finally output. 
    # </remarks>
    # <returns>If successful returns NP_OK, otherwise will return NP_ERR_NO_DATA.</returns>
    def HandleTrackIRData(self):

        # poll for the tracking data 
        result, tid = npc.GetData()
        # check if polling worked
        if result == NPRESULT.NP_OK :
            # check if the device is active (ie not in mouse-emulation mode) and the frame has changed
            # otherwise we should not display the data
            if tid.Status == npc.NPSTATUS_REMOTEACTIVE:

                if (self.frameSig != tid.FrameSignature):

				    # convert TIR translation values from TIR units to centimeters
				    #x = ( tid.X / NP_MAX_VALUE ) * NP_MAX_TRANSLATION;
				    #y = ( tid.Y / NP_MAX_VALUE ) * NP_MAX_TRANSLATION;
				    #z = ( tid.Z / NP_MAX_VALUE ) * NP_MAX_TRANSLATION;

				    # convert TIR rotation values from TIR units to degrees
                    yaw = ( tid.Yaw / NP_MAX_VALUE ) * NP_MAX_ROTATION;
                    pitch = ( tid.Pitch / NP_MAX_VALUE ) * NP_MAX_ROTATION;
                    roll = ( tid.Roll / NP_MAX_VALUE ) * NP_MAX_ROTATION;

                    self.AddTopLine(("Pitch = " + str(round(pitch,2))
                                    + " Yaw = " + str(round(yaw,2))
                                    + " Roll = " + str(round(roll,2))
                                    + " NPStatus = "
                                    + str(tid.Status)
                                    + " Frame = "
                                    + str(tid.FrameSignature)))

                    self.frameSig = tid.FrameSignature

                else:

                    self.AddTopLine("No Data")
                    result = NPRESULT.NP_ERR_NO_DATA

            else:

                self.AddTopLine("User Disabled")
                RESULT = NPRESULT.NP_ERR_NO_DATA

        return result

    # loop to control polling data with HandleTrackIRData
    def GetDataLoop(self):

        if self.readData:
            result = self.HandleTrackIRData()

            if result == NPRESULT.NP_ERR_NO_DATA:
                self.timerMsgNum += 1
                self.AddLine("No new data on timer call " +
                             str(self.timerMsgNum))

# code to start on application start
if __name__ == "__main__":

    root = tk.Tk()
    app = cHeadTrackingGUI(root)

    # unregister and delete window on exit
    root.protocol("WM_DELETE_WINDOW", app.OnExit)
    root.resizable(False, False)
    root.mainloop()