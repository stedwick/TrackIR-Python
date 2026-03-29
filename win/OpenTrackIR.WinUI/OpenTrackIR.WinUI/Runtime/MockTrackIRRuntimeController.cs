using OpenTrackIR.WinUI.Models;

namespace OpenTrackIR.WinUI.Runtime
{
    public sealed class MockTrackIRRuntimeController : ITrackIRRuntimeController
    {
        private TrackIRControlState _controlState = TrackIRUiLogic.CreateDefaultControlState();
        private TrackIRPresentationState _presentationState = new(true, true);
        private ulong _revision = 1;
        private byte[] _currentPreviewPixels = Array.Empty<byte>();

        public TrackIRSnapshot CurrentSnapshot { get; private set; }
        public TrackIRPreviewFrame? CurrentPreviewFrame { get; private set; }

        public event EventHandler<TrackIRSnapshot>? SnapshotChanged;
        public event EventHandler<TrackIRPreviewFrame?>? PreviewFrameChanged;

        public MockTrackIRRuntimeController()
        {
            CurrentSnapshot = TrackIRUiLogic.BuildMockSnapshot(_controlState, _presentationState, _revision);
            _currentPreviewPixels = CurrentSnapshot.HasPreview ? BuildPreviewPixels() : Array.Empty<byte>();
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

        public void Stop()
        {
        }

        public bool TryCopyCurrentPreviewFrame(byte[] destination, out TrackIRPreviewFrame? previewFrame)
        {
            previewFrame = CurrentPreviewFrame;
            if (previewFrame is null)
            {
                return false;
            }

            int requiredLength = TrackIRPreviewBitmapLogic.Gray8BufferLength(
                previewFrame.Width,
                previewFrame.Height
            );
            if (destination.Length < requiredLength || _currentPreviewPixels.Length < requiredLength)
            {
                previewFrame = null;
                return false;
            }

            Array.Copy(_currentPreviewPixels, destination, requiredLength);
            return true;
        }

        private void PublishNextSnapshot()
        {
            _revision += 1;
            CurrentSnapshot = TrackIRUiLogic.BuildMockSnapshot(_controlState, _presentationState, _revision);
            _currentPreviewPixels = CurrentSnapshot.HasPreview
                ? BuildPreviewPixels()
                : Array.Empty<byte>();
            CurrentPreviewFrame = BuildPreviewFrame(CurrentSnapshot);
            SnapshotChanged?.Invoke(this, CurrentSnapshot);
            PreviewFrameChanged?.Invoke(this, CurrentPreviewFrame);
        }

        private static byte[] BuildPreviewPixels()
        {
            byte[] pixels = new byte[TrackIRUiLogic.FrameWidth * TrackIRUiLogic.FrameHeight];
            for (int index = 0; index < pixels.Length; index++)
            {
                int x = index % TrackIRUiLogic.FrameWidth;
                int y = index / TrackIRUiLogic.FrameWidth;
                pixels[index] = (byte)((x + y) % 256);
            }

            return pixels;
        }

        private static TrackIRPreviewFrame? BuildPreviewFrame(TrackIRSnapshot snapshot)
        {
            if (!snapshot.HasPreview)
            {
                return null;
            }

            return new TrackIRPreviewFrame(
                Generation: snapshot.FrameIndex,
                Width: TrackIRUiLogic.FrameWidth,
                Height: TrackIRUiLogic.FrameHeight
            );
        }
    }
}
