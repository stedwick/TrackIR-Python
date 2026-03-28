namespace OpenTrackIR.WinUI.Models
{
    public sealed record TrackIRSnapshot(
        TrackIRRuntimePhase Phase,
        string? ErrorDescription,
        ulong FrameIndex,
        double? SourceFrameRate,
        double? CentroidX,
        double? CentroidY,
        byte? PacketType,
        string DeviceLabel,
        string BackendLabel,
        XKeysIndicatorState XKeysIndicatorState,
        bool HasPreview,
        bool IsLowPowerMode
    );
}
