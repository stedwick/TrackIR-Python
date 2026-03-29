using System.Runtime.InteropServices;

namespace OpenTrackIR.WinUI.Runtime
{
    internal static class TrackIRNativeMethods
    {
        private const string LibraryName = "opentrackir";
        internal const int MaxSmoothingWindow = 31;

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        internal struct NativeTrackIRSessionSnapshot
        {
            public int Phase;
            public int Status;
            public ulong FrameIndex;
            public byte HasFrameRate;
            public double FrameRate;
            public byte HasCentroid;
            public double CentroidX;
            public double CentroidY;
            public byte HasPacketType;
            public byte PacketType;
            public byte HasPreviewFrame;
            public ulong PreviewFrameGeneration;
            public ushort PreviewWidth;
            public ushort PreviewHeight;
            public byte IsLowPowerMode;
            public byte HasErrorMessage;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 160)]
            public string ErrorMessage;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct NativeTrackIRMousePoint
        {
            public double X;
            public double Y;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct NativeTrackIRMouseTransform
        {
            public double ScaleX;
            public double ScaleY;
            public double RotationDegrees;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct NativeTrackIRMouseTrackerConfig
        {
            [MarshalAs(UnmanagedType.I1)]
            public bool IsMovementEnabled;
            public double Speed;
            public double Smoothing;
            public double Deadzone;
            [MarshalAs(UnmanagedType.I1)]
            public bool AvoidMouseJumps;
            public double JumpThresholdPixels;
            public NativeTrackIRMouseTransform Transform;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct NativeTrackIRMouseTrackerState
        {
            [MarshalAs(UnmanagedType.I1)]
            public bool HasPreviousCentroid;
            public NativeTrackIRMousePoint PreviousCentroid;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = MaxSmoothingWindow)]
            public NativeTrackIRMousePoint[] DeltaHistory;
            public nuint DeltaHistoryCount;
            public nuint DeltaHistoryWriteIndex;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct NativeTrackIRMouseStep
        {
            [MarshalAs(UnmanagedType.I1)]
            public bool HasCursorDelta;
            public NativeTrackIRMousePoint CursorDelta;
            [MarshalAs(UnmanagedType.I1)]
            public bool HasNextCentroid;
            public NativeTrackIRMousePoint NextCentroid;
        }

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "otir_trackir_session_create")]
        internal static extern nint TrackIRSessionCreate();

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "otir_trackir_session_destroy")]
        internal static extern void TrackIRSessionDestroy(nint session);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "otir_trackir_session_start")]
        internal static extern int TrackIRSessionStart(nint session);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "otir_trackir_session_stop")]
        internal static extern void TrackIRSessionStop(nint session, [MarshalAs(UnmanagedType.I1)] bool waitForShutdown);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "otir_trackir_session_set_maximum_tracking_frames_per_second")]
        internal static extern void TrackIRSessionSetMaximumTrackingFramesPerSecond(nint session, double maximumFramesPerSecond);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "otir_trackir_session_set_video_enabled")]
        internal static extern void TrackIRSessionSetVideoEnabled(nint session, [MarshalAs(UnmanagedType.I1)] bool enabled);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "otir_trackir_session_set_minimum_blob_area_points")]
        internal static extern void TrackIRSessionSetMinimumBlobAreaPoints(nint session, int minimumBlobAreaPoints);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "otir_trackir_session_set_centroid_mode")]
        internal static extern void TrackIRSessionSetCentroidMode(nint session, int centroidMode);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "otir_trackir_session_set_low_power_mode_enabled")]
        internal static extern void TrackIRSessionSetLowPowerModeEnabled(nint session, [MarshalAs(UnmanagedType.I1)] bool enabled);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "otir_trackir_session_copy_snapshot")]
        internal static extern void TrackIRSessionCopySnapshot(nint session, out NativeTrackIRSessionSnapshot snapshot);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "otir_trackir_session_copy_preview_frame")]
        [return: MarshalAs(UnmanagedType.I1)]
        internal static extern bool TrackIRSessionCopyPreviewFrame(
            nint session,
            [Out] byte[] frame,
            nuint capacity,
            out ulong generation
        );

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "otir_trackir_mouse_tracker_reset")]
        internal static extern void TrackIRMouseTrackerReset(ref NativeTrackIRMouseTrackerState state);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "otir_trackir_mouse_tracker_update")]
        internal static extern NativeTrackIRMouseStep TrackIRMouseTrackerUpdate(
            ref NativeTrackIRMouseTrackerState state,
            [MarshalAs(UnmanagedType.I1)] bool hasCurrentCentroid,
            NativeTrackIRMousePoint currentCentroid,
            NativeTrackIRMouseTrackerConfig config
        );
    }
}
