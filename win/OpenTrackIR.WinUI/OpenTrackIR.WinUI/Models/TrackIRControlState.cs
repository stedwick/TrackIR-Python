namespace OpenTrackIR.WinUI.Models
{
    public sealed record TrackIRControlState(
        bool IsVideoEnabled,
        bool IsTrackIREnabled,
        bool IsMouseMovementEnabled,
        bool IsWindowsAbsoluteMousePositioningEnabled,
        double MouseMovementSpeed,
        bool IsXKeysFastMouseEnabled,
        int MouseSmoothing,
        double MouseDeadzone,
        bool IsAvoidMouseJumpsEnabled,
        int MouseJumpThresholdPixels,
        int MinimumBlobAreaPoints,
        int KeepAwakeSeconds,
        bool IsTimeoutEnabled,
        int TimeoutSeconds,
        bool IsVideoFlipHorizontalEnabled,
        bool IsVideoFlipVerticalEnabled,
        double VideoRotationDegrees,
        double VideoFramesPerSecond,
        string MouseToggleHotkeyText,
        string RecenterHotkeyText,
        int MouseOverrideDelayMilliseconds,
        bool IsMouseButtonOverrideEnabled
    );
}
