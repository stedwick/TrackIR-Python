using OpenTrackIR.WinUI.Models;

namespace OpenTrackIR.WinUI.Runtime
{
    public interface ITrackIRRuntimeController
    {
        TrackIRSnapshot CurrentSnapshot { get; }

        event EventHandler<TrackIRSnapshot>? SnapshotChanged;

        void UpdateControlState(TrackIRControlState controlState);

        void UpdatePresentationState(TrackIRPresentationState presentationState);

        void Refresh();
    }
}
