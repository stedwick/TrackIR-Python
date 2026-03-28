using OpenTrackIR.WinUI.Models;

namespace OpenTrackIR.WinUI.Services
{
    public interface ISettingsStore
    {
        TrackIRControlState Load();

        void Save(TrackIRControlState controlState);
    }
}
