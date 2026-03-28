namespace OpenTrackIR.WinUI.Models
{
    public static class TrayUiLogic
    {
        public static string TooltipText(bool isTrackIREnabled, bool isMouseMovementEnabled)
        {
            string trackIRLabel = isTrackIREnabled ? "TrackIR On" : "TrackIR Off";
            string mouseLabel = isMouseMovementEnabled ? "Mouse On" : "Mouse Off";
            return $"OpenTrackIR: {trackIRLabel}, {mouseLabel}";
        }
    }
}
