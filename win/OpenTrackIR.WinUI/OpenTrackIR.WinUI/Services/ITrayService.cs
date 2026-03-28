namespace OpenTrackIR.WinUI.Services
{
    public interface ITrayService : IDisposable
    {
        void Initialize(Action showWindow, Action exitApplication);

        void UpdateState(bool isTrackIREnabled, bool isMouseMovementEnabled);
    }
}
