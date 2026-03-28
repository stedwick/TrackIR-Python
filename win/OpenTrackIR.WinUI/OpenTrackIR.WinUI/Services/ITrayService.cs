namespace OpenTrackIR.WinUI.Services
{
    public interface ITrayService
    {
        void UpdateState(bool isTrackIREnabled, bool isMouseMovementEnabled);
    }
}
