namespace OpenTrackIR.WinUI.Models
{
    public static class TrackIRRuntimeLogic
    {
        public const double VisiblePreviewFramesPerSecond = 30.0;
        public const int BackgroundPollIntervalMilliseconds = 250;
        public const int RegularizedBinaryCentroidMode = 5;
        public const string MissingNativeRuntimeMessage =
            "OpenTrackIR native runtime is unavailable. Build opentrackir.dll and place it next to the app.";

        public static bool ShouldRunSession(TrackIRControlState controlState)
        {
            return controlState.IsTrackIREnabled;
        }

        public static bool ShouldPublishPreview(
            TrackIRControlState controlState,
            TrackIRPresentationState presentationState,
            bool hasPreviewFrame
        )
        {
            return controlState.IsTrackIREnabled &&
                controlState.IsVideoEnabled &&
                presentationState.IsWindowVisible &&
                hasPreviewFrame;
        }

        public static bool ShouldEnableLowPowerMode(
            TrackIRControlState controlState,
            TrackIRPresentationState presentationState
        )
        {
            return (!presentationState.IsWindowVisible || !presentationState.IsAppActive) &&
                !controlState.IsMouseMovementEnabled;
        }

        public static int PollIntervalMilliseconds(
            TrackIRControlState controlState,
            TrackIRPresentationState presentationState
        )
        {
            if (controlState.IsTrackIREnabled && controlState.IsMouseMovementEnabled)
            {
                double targetFramesPerSecond = controlState.VideoFramesPerSecond > 0.0
                    ? controlState.VideoFramesPerSecond
                    : 60.0;
                return Math.Max(1, (int)Math.Round(1000.0 / targetFramesPerSecond));
            }

            return controlState.IsTrackIREnabled &&
                controlState.IsVideoEnabled &&
                presentationState.IsWindowVisible
                    ? (int)Math.Round(1000.0 / VisiblePreviewFramesPerSecond)
                    : BackgroundPollIntervalMilliseconds;
        }

        public static bool ShouldApplyRuntimeUpdate(bool isDisposed)
        {
            return !isDisposed;
        }

        public static bool ShouldScheduleTimeout(TrackIRControlState controlState)
        {
            return controlState.IsTrackIREnabled &&
                controlState.IsTimeoutEnabled &&
                controlState.TimeoutSeconds > 0;
        }

        public static TrackIRControlState TimedOutControlState(TrackIRControlState controlState)
        {
            return controlState with
            {
                IsTrackIREnabled = false,
                IsVideoEnabled = false,
                IsMouseMovementEnabled = false,
            };
        }

        public static TrackIRSnapshot MissingNativeRuntimeSnapshot(TrackIRControlState controlState)
        {
            return MissingNativeRuntimeSnapshot(controlState, new TrackIRPresentationState(true, true));
        }

        public static TrackIRSnapshot MissingNativeRuntimeSnapshot(
            TrackIRControlState controlState,
            TrackIRPresentationState presentationState
        )
        {
            return new TrackIRSnapshot(
                Phase: controlState.IsTrackIREnabled ? TrackIRRuntimePhase.Failed : TrackIRRuntimePhase.Idle,
                ErrorDescription: controlState.IsTrackIREnabled ? MissingNativeRuntimeMessage : null,
                FrameIndex: 0,
                SourceFrameRate: null,
                CentroidX: null,
                CentroidY: null,
                PacketType: null,
                DeviceLabel: controlState.IsTrackIREnabled ? "Native runtime missing" : "Idle",
                BackendLabel: "C session / pending",
                XKeysIndicatorState: controlState.IsXKeysFastMouseEnabled
                    ? XKeysIndicatorState.NotDetected
                    : XKeysIndicatorState.Disabled,
                HasPreview: false,
                IsLowPowerMode: ShouldEnableLowPowerMode(controlState, presentationState)
            );
        }

        public static TrackIRSnapshot IdleSnapshot(
            TrackIRControlState controlState,
            TrackIRPresentationState presentationState
        )
        {
            return new TrackIRSnapshot(
                Phase: TrackIRRuntimePhase.Idle,
                ErrorDescription: null,
                FrameIndex: 0,
                SourceFrameRate: null,
                CentroidX: null,
                CentroidY: null,
                PacketType: null,
                DeviceLabel: "Idle",
                BackendLabel: "C session / pending",
                XKeysIndicatorState: controlState.IsXKeysFastMouseEnabled
                    ? XKeysIndicatorState.NotDetected
                    : XKeysIndicatorState.Disabled,
                HasPreview: false,
                IsLowPowerMode: ShouldEnableLowPowerMode(controlState, presentationState)
            );
        }
    }
}
