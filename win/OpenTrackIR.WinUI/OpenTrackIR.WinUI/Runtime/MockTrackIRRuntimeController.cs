using OpenTrackIR.WinUI.Models;

namespace OpenTrackIR.WinUI.Runtime
{
    public sealed class MockTrackIRRuntimeController : ITrackIRRuntimeController
    {
        private TrackIRControlState _controlState = TrackIRUiLogic.CreateDefaultControlState();
        private TrackIRPresentationState _presentationState = new(true, true);
        private ulong _revision = 1;

        public TrackIRSnapshot CurrentSnapshot { get; private set; }

        public event EventHandler<TrackIRSnapshot>? SnapshotChanged;

        public MockTrackIRRuntimeController()
        {
            CurrentSnapshot = TrackIRUiLogic.BuildMockSnapshot(_controlState, _presentationState, _revision);
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
            SnapshotChanged?.Invoke(this, CurrentSnapshot);
        }
    }
}
