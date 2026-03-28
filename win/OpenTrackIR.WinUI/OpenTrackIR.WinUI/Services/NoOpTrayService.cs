namespace OpenTrackIR.WinUI.Services
{
    public sealed class NoOpTrayService : ITrayService
    {
        public void UpdateState(bool isTrackIREnabled, bool isMouseMovementEnabled)
        {
        }
    }
}
