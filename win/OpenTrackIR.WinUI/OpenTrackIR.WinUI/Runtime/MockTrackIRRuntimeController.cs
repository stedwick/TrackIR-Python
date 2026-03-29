using OpenTrackIR.WinUI.Models;

namespace OpenTrackIR.WinUI.Runtime
{
    public sealed class MockTrackIRRuntimeController : ITrackIRRuntimeController
    {
        private TrackIRControlState _controlState = TrackIRUiLogic.CreateDefaultControlState();
        private TrackIRPresentationState _presentationState = new(true, true);
        private ulong _revision = 1;

        public TrackIRSnapshot CurrentSnapshot { get; private set; }
        public TrackIRPreviewFrame? CurrentPreviewFrame { get; private set; }

        public event EventHandler<TrackIRSnapshot>? SnapshotChanged;
        public event EventHandler<TrackIRPreviewFrame?>? PreviewFrameChanged;

        public MockTrackIRRuntimeController()
        {
            CurrentSnapshot = TrackIRUiLogic.BuildMockSnapshot(_controlState, _presentationState, _revision);
            CurrentPreviewFrame = BuildPreviewFrame(CurrentSnapshot);
        }

        public void UpdateControlState(TrackIRControlState controlState)
        {
            _controlState = TrackIRUiLogic.Normalize(controlState);
            PublishNextSnapshot();
        }

        public void UpdatePresentationState(TrackIRPresentationState presentationState)
        {
            _presentationState = presentationState;
            PublishNextSnapshot();
        }

        public void Refresh()
        {
            PublishNextSnapshot();
        }

        private void PublishNextSnapshot()
        {
            _revision += 1;
            CurrentSnapshot = TrackIRUiLogic.BuildMockSnapshot(_controlState, _presentationState, _revision);
            CurrentPreviewFrame = BuildPreviewFrame(CurrentSnapshot);
            SnapshotChanged?.Invoke(this, CurrentSnapshot);
            PreviewFrameChanged?.Invoke(this, CurrentPreviewFrame);
        }

        private static TrackIRPreviewFrame? BuildPreviewFrame(TrackIRSnapshot snapshot)
        {
            if (!snapshot.HasPreview)
            {
                return null;
            }

            byte[] pixels = new byte[TrackIRUiLogic.FrameWidth * TrackIRUiLogic.FrameHeight];
            for (int index = 0; index < pixels.Length; index++)
            {
                int x = index % TrackIRUiLogic.FrameWidth;
                int y = index / TrackIRUiLogic.FrameWidth;
                pixels[index] = (byte)((x + y) % 256);
            }

            return new TrackIRPreviewFrame(
                Generation: snapshot.FrameIndex,
                Width: TrackIRUiLogic.FrameWidth,
                Height: TrackIRUiLogic.FrameHeight,
                Gray8Pixels: pixels
            );
        }
    }
}
