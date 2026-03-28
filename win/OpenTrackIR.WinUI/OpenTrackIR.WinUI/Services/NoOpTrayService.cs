namespace OpenTrackIR.WinUI.Services
{
    public sealed class NoOpTrayService : ITrayService
    {
        public void Initialize(Action showWindow, Action exitApplication)
        {
        }

        public void UpdateState(bool isTrackIREnabled, bool isMouseMovementEnabled)
        {
        }

        public void Dispose()
        {
        }
    }
}
