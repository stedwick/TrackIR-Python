using OpenTrackIR.WinUI.Models;

namespace OpenTrackIR.WinUI.Runtime
{
    public interface ITrackIRRuntimeController
    {
        TrackIRSnapshot CurrentSnapshot { get; }
        TrackIRPreviewFrame? CurrentPreviewFrame { get; }

        event EventHandler<TrackIRSnapshot>? SnapshotChanged;
        event EventHandler<TrackIRPreviewFrame?>? PreviewFrameChanged;

        bool TryCopyCurrentPreviewFrame(byte[] destination, out TrackIRPreviewFrame? previewFrame);

        void UpdateControlState(TrackIRControlState controlState);

        void UpdatePresentationState(TrackIRPresentationState presentationState);

        void Refresh();

        void Stop();
    }
}
