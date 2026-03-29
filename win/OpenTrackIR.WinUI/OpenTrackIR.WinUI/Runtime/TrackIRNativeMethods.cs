using System.Runtime.InteropServices;

namespace OpenTrackIR.WinUI.Runtime
{
    internal static class TrackIRNativeMethods
    {
        private const string LibraryName = "opentrackir";

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
    }
}
