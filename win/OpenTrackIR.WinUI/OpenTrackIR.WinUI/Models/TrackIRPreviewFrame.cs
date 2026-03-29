namespace OpenTrackIR.WinUI.Models
{
    public sealed record TrackIRPreviewFrame(
        ulong Generation,
        int Width,
        int Height,
        byte[] Gray8Pixels
    );
}
