using OpenTrackIR.WinUI.Models;
using Windows.Storage;

namespace OpenTrackIR.WinUI.Services
{
    public sealed class LocalSettingsStore : ISettingsStore
    {
        private const string ControlStateKey = "trackir.controlState";

        public TrackIRControlState Load()
        {
            object? rawValue = ApplicationData.Current.LocalSettings.Values[ControlStateKey];
            if (rawValue is string json)
            {
                TrackIRControlState? state = TrackIRControlStateJson.Deserialize(json);
                if (state is not null)
                {
                    return TrackIRUiLogic.Normalize(state);
                }
            }

            return TrackIRUiLogic.CreateDefaultControlState();
        }

        public void Save(TrackIRControlState controlState)
        {
            TrackIRControlState normalizedState = TrackIRUiLogic.Normalize(controlState);
            ApplicationData.Current.LocalSettings.Values[ControlStateKey] =
                TrackIRControlStateJson.Serialize(normalizedState);
        }
    }
}
