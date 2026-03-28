using System.Windows.Input;
using Microsoft.UI;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Media;
using OpenTrackIR.WinUI.Models;
using OpenTrackIR.WinUI.Runtime;
using OpenTrackIR.WinUI.Services;

namespace OpenTrackIR.WinUI.ViewModels
{
    public sealed class MainShellViewModel : ObservableObject
    {
        private readonly ISettingsStore _settingsStore;
        private readonly ITrackIRRuntimeController _runtimeController;
        private readonly ITrayService _trayService;
        private TrackIRControlState _controlState;
        private TrackIRSnapshot _snapshot;
        private bool _isAdvancedExpanded;
        private bool _showDetectedBlobCenter = true;

        public MainShellViewModel()
            : this(new LocalSettingsStore(), new MockTrackIRRuntimeController(), new NoOpTrayService())
        {
        }

        public MainShellViewModel(
            ISettingsStore settingsStore,
            ITrackIRRuntimeController runtimeController,
            ITrayService trayService
        )
        {
            _settingsStore = settingsStore;
            _runtimeController = runtimeController;
            _trayService = trayService;
            _controlState = TrackIRUiLogic.Normalize(_settingsStore.Load());
            _snapshot = _runtimeController.CurrentSnapshot;

            RefreshCommand = new RelayCommand(Refresh);
            _runtimeController.SnapshotChanged += OnRuntimeSnapshotChanged;
            ApplyControlState(_controlState, persist: false);
            _runtimeController.UpdatePresentationState(new TrackIRPresentationState(true, true));
        }

        public ICommand RefreshCommand { get; }

        public string Title => "OpenTrackIR";

        public string Subtitle => "Windows TrackIR preview and controls";

        public string ControlsDescription => "Live controls for TrackIR, video, and mouse movement.";

        public string AdvancedDescription => "Advanced tuning for blob detection, smoothing, keep-awake, and timeout.";

        public string HotkeyHelperText => "Click the field and press a shortcut. It is stored locally for the future backend, but it is not registered system-wide yet.";

        public string BlobDetectionDescription => "Filter tiny blobs and keep the previous-regularized centroid mode for steadier results.";

        public string BlobCentroidFormulaText => "Centroid Formula: Previous-Regularized";

        public string BlobCentroidFormulaDetail => "Blend the new binary centroid with the previous center to calm tiny frame-to-frame wobble.";

        public string TimeoutHelperText => TrackIRUiLogic.TimeoutHelperText;

        public bool IsAdvancedExpanded
        {
            get => _isAdvancedExpanded;
            set => SetProperty(ref _isAdvancedExpanded, value);
        }

        public bool ShowDetectedBlobCenter
        {
            get => _showDetectedBlobCenter;
            set
            {
                if (SetProperty(ref _showDetectedBlobCenter, value))
                {
                    OnPropertyChanged(nameof(PreviewMarkerVisibility));
                }
            }
        }

        public bool IsTrackIREnabled
        {
            get => _controlState.IsTrackIREnabled;
            set => UpdateControlState(_controlState with { IsTrackIREnabled = value });
        }

        public bool IsVideoEnabled
        {
            get => _controlState.IsVideoEnabled;
            set => UpdateControlState(_controlState with { IsVideoEnabled = value });
        }

        public bool IsMouseMovementEnabled
        {
            get => _controlState.IsMouseMovementEnabled;
            set => UpdateControlState(_controlState with { IsMouseMovementEnabled = value });
        }

        public double MouseMovementSpeed
        {
            get => _controlState.MouseMovementSpeed;
            set => UpdateControlState(_controlState with { MouseMovementSpeed = value });
        }

        public bool IsXKeysFastMouseEnabled
        {
            get => _controlState.IsXKeysFastMouseEnabled;
            set => UpdateControlState(_controlState with { IsXKeysFastMouseEnabled = value });
        }

        public double MouseSmoothing
        {
            get => _controlState.MouseSmoothing;
            set => UpdateControlState(_controlState with { MouseSmoothing = (int)Math.Round(value) });
        }

        public double MouseDeadzone
        {
            get => _controlState.MouseDeadzone;
            set => UpdateControlState(_controlState with { MouseDeadzone = value });
        }

        public bool IsAvoidMouseJumpsEnabled
        {
            get => _controlState.IsAvoidMouseJumpsEnabled;
            set => UpdateControlState(_controlState with { IsAvoidMouseJumpsEnabled = value });
        }

        public double MouseJumpThresholdPixels
        {
            get => _controlState.MouseJumpThresholdPixels;
            set => UpdateControlState(_controlState with { MouseJumpThresholdPixels = (int)Math.Round(value) });
        }

        public double MinimumBlobAreaPoints
        {
            get => _controlState.MinimumBlobAreaPoints;
            set => UpdateControlState(_controlState with { MinimumBlobAreaPoints = (int)Math.Round(value) });
        }

        public double KeepAwakeSeconds
        {
            get => _controlState.KeepAwakeSeconds;
            set => UpdateControlState(_controlState with { KeepAwakeSeconds = (int)Math.Round(value) });
        }

        public bool IsTimeoutEnabled
        {
            get => _controlState.IsTimeoutEnabled;
            set => UpdateControlState(_controlState with { IsTimeoutEnabled = value });
        }

        public double TimeoutSeconds
        {
            get => _controlState.TimeoutSeconds;
            set => UpdateControlState(_controlState with { TimeoutSeconds = (int)Math.Round(value) });
        }

        public bool IsVideoFlipHorizontalEnabled
        {
            get => _controlState.IsVideoFlipHorizontalEnabled;
            set => UpdateControlState(_controlState with { IsVideoFlipHorizontalEnabled = value });
        }

        public bool IsVideoFlipVerticalEnabled
        {
            get => _controlState.IsVideoFlipVerticalEnabled;
            set => UpdateControlState(_controlState with { IsVideoFlipVerticalEnabled = value });
        }

        public double VideoRotationDegrees
        {
            get => _controlState.VideoRotationDegrees;
            set => UpdateControlState(_controlState with { VideoRotationDegrees = value });
        }

        public double VideoFramesPerSecond
        {
            get => _controlState.VideoFramesPerSecond;
            set => UpdateControlState(_controlState with { VideoFramesPerSecond = value });
        }

        public string MouseToggleHotkeyText
        {
            get => _controlState.MouseToggleHotkeyText;
            set => UpdateControlState(_controlState with { MouseToggleHotkeyText = value });
        }

        public string TrackIRStatusText => IsTrackIREnabled ? "TrackIR On" : "TrackIR Off";

        public string TrackIRStatusValue => TrackIRUiLogic.ToggleStateLabel(IsTrackIREnabled, "On", "Off");

        public string VideoStatusText => IsVideoEnabled ? "Video Visible" : "Video Hidden";

        public string VideoStatusValue => TrackIRUiLogic.ToggleStateLabel(IsVideoEnabled, "Visible", "Hidden");

        public string MouseStatusText => IsMouseMovementEnabled ? "Mouse On" : "Mouse Off";

        public string MouseStatusValue => TrackIRUiLogic.ToggleStateLabel(IsMouseMovementEnabled, "On", "Off");

        public Brush TrackIRStatusBrush => CreateBrush(TrackIRUiLogic.ToggleStateColorHex(IsTrackIREnabled));

        public Brush VideoStatusBrush => CreateBrush(TrackIRUiLogic.ToggleStateColorHex(IsVideoEnabled));

        public Brush MouseStatusBrush => CreateBrush(TrackIRUiLogic.ToggleStateColorHex(IsMouseMovementEnabled));

        public string PreviewTitle => TrackIRUiLogic.PreviewTitle(_controlState, _snapshot);

        public string PreviewMessage => TrackIRUiLogic.PreviewMessage(_controlState, _snapshot);

        public string DeviceLabel => _snapshot.DeviceLabel;

        public string FramesPerSecondSummary =>
            TrackIRUiLogic.TrackIRRateSummaryLabel(VideoFramesPerSecond, _snapshot.SourceFrameRate);

        public string PositionLabel => TrackIRUiLogic.CentroidPairLabel(_snapshot.CentroidX, _snapshot.CentroidY);

        public string BackendLabel => _snapshot.BackendLabel;

        public string TrackIRFramesPerSecondLabel =>
            TrackIRUiLogic.TrackIRFramesPerSecondValueLabel(VideoFramesPerSecond);

        public string MouseSpeedLabel => TrackIRUiLogic.MouseSpeedValueLabel(MouseMovementSpeed);

        public string MouseSmoothingLabel => TrackIRUiLogic.MouseSmoothingValueLabel(MouseSmoothing);

        public string MouseDeadzoneLabel => TrackIRUiLogic.MouseDeadzoneValueLabel(MouseDeadzone);

        public string VideoRotationLabel => TrackIRUiLogic.VideoRotationValueLabel(VideoRotationDegrees);

        public string PacketTypeLabel => TrackIRUiLogic.PacketTypeLabel(_snapshot.PacketType);

        public string XKeysIndicatorText => TrackIRUiLogic.XKeysIndicatorLabel(_snapshot.XKeysIndicatorState);

        public Brush XKeysIndicatorBrush => CreateBrush(TrackIRUiLogic.XKeysIndicatorColorHex(_snapshot.XKeysIndicatorState));

        public string RuntimeModeLabel => _snapshot.IsLowPowerMode ? "Background" : "Interactive";

        public Visibility PreviewMarkerVisibility =>
            ShowDetectedBlobCenter && _snapshot.HasPreview && _snapshot.CentroidX.HasValue && _snapshot.CentroidY.HasValue
                ? Visibility.Visible
                : Visibility.Collapsed;

        public Visibility JumpThresholdVisibility =>
            IsAvoidMouseJumpsEnabled ? Visibility.Visible : Visibility.Collapsed;

        public Visibility TimeoutDurationVisibility =>
            IsTimeoutEnabled ? Visibility.Visible : Visibility.Collapsed;

        public double PreviewMarkerLeft =>
            Math.Clamp((_snapshot.CentroidX ?? 0) - 10, 0, TrackIRUiLogic.FrameWidth - 20);

        public double PreviewMarkerTop =>
            Math.Clamp((_snapshot.CentroidY ?? 0) - 10, 0, TrackIRUiLogic.FrameHeight - 20);

        public void SetPresentationState(bool isWindowVisible, bool isAppActive)
        {
            _runtimeController.UpdatePresentationState(new TrackIRPresentationState(isWindowVisible, isAppActive));
        }

        private void Refresh()
        {
            ApplyControlState(_settingsStore.Load(), persist: false);
            _runtimeController.Refresh();
        }

        private void UpdateControlState(TrackIRControlState controlState)
        {
            ApplyControlState(controlState, persist: true);
        }

        private void ApplyControlState(TrackIRControlState controlState, bool persist)
        {
            _controlState = TrackIRUiLogic.Normalize(controlState);
            if (persist)
            {
                _settingsStore.Save(_controlState);
            }

            _trayService.UpdateState(_controlState.IsTrackIREnabled, _controlState.IsMouseMovementEnabled);
            _runtimeController.UpdateControlState(_controlState);
            OnControlStateChanged();
        }

        private void OnRuntimeSnapshotChanged(object? sender, TrackIRSnapshot snapshot)
        {
            _snapshot = snapshot;
            OnSnapshotChanged();
        }

        private void OnControlStateChanged()
        {
            OnPropertyChanged(nameof(IsTrackIREnabled));
            OnPropertyChanged(nameof(IsVideoEnabled));
            OnPropertyChanged(nameof(IsMouseMovementEnabled));
            OnPropertyChanged(nameof(MouseMovementSpeed));
            OnPropertyChanged(nameof(IsXKeysFastMouseEnabled));
            OnPropertyChanged(nameof(MouseSmoothing));
            OnPropertyChanged(nameof(MouseDeadzone));
            OnPropertyChanged(nameof(IsAvoidMouseJumpsEnabled));
            OnPropertyChanged(nameof(MouseJumpThresholdPixels));
            OnPropertyChanged(nameof(MinimumBlobAreaPoints));
            OnPropertyChanged(nameof(KeepAwakeSeconds));
            OnPropertyChanged(nameof(IsTimeoutEnabled));
            OnPropertyChanged(nameof(TimeoutSeconds));
            OnPropertyChanged(nameof(IsVideoFlipHorizontalEnabled));
            OnPropertyChanged(nameof(IsVideoFlipVerticalEnabled));
            OnPropertyChanged(nameof(VideoRotationDegrees));
            OnPropertyChanged(nameof(VideoFramesPerSecond));
            OnPropertyChanged(nameof(MouseToggleHotkeyText));
            OnPropertyChanged(nameof(TrackIRStatusText));
            OnPropertyChanged(nameof(TrackIRStatusValue));
            OnPropertyChanged(nameof(TrackIRStatusBrush));
            OnPropertyChanged(nameof(VideoStatusText));
            OnPropertyChanged(nameof(VideoStatusValue));
            OnPropertyChanged(nameof(VideoStatusBrush));
            OnPropertyChanged(nameof(MouseStatusText));
            OnPropertyChanged(nameof(MouseStatusValue));
            OnPropertyChanged(nameof(MouseStatusBrush));
            OnPropertyChanged(nameof(TrackIRFramesPerSecondLabel));
            OnPropertyChanged(nameof(MouseSpeedLabel));
            OnPropertyChanged(nameof(MouseSmoothingLabel));
            OnPropertyChanged(nameof(MouseDeadzoneLabel));
            OnPropertyChanged(nameof(VideoRotationLabel));
            OnPropertyChanged(nameof(JumpThresholdVisibility));
            OnPropertyChanged(nameof(TimeoutDurationVisibility));
        }

        private void OnSnapshotChanged()
        {
            OnPropertyChanged(nameof(PreviewTitle));
            OnPropertyChanged(nameof(PreviewMessage));
            OnPropertyChanged(nameof(DeviceLabel));
            OnPropertyChanged(nameof(FramesPerSecondSummary));
            OnPropertyChanged(nameof(PositionLabel));
            OnPropertyChanged(nameof(BackendLabel));
            OnPropertyChanged(nameof(PacketTypeLabel));
            OnPropertyChanged(nameof(XKeysIndicatorText));
            OnPropertyChanged(nameof(XKeysIndicatorBrush));
            OnPropertyChanged(nameof(RuntimeModeLabel));
            OnPropertyChanged(nameof(PreviewMarkerVisibility));
            OnPropertyChanged(nameof(PreviewMarkerLeft));
            OnPropertyChanged(nameof(PreviewMarkerTop));
        }

        private static SolidColorBrush CreateBrush(string hexColor)
        {
            string normalized = hexColor.TrimStart('#');
            byte a = 0xFF;
            int offset = 0;

            if (normalized.Length == 8)
            {
                a = Convert.ToByte(normalized[..2], 16);
                offset = 2;
            }

            byte r = Convert.ToByte(normalized.Substring(offset, 2), 16);
            byte g = Convert.ToByte(normalized.Substring(offset + 2, 2), 16);
            byte b = Convert.ToByte(normalized.Substring(offset + 4, 2), 16);
            return new SolidColorBrush(ColorHelper.FromArgb(a, r, g, b));
        }
    }
}
