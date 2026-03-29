namespace OpenTrackIR.WinUI.Models
{
    public readonly record struct XKeysMonitorSnapshot(
        XKeysIndicatorState IndicatorState,
        bool IsPressed
    );
}
