using System.Windows.Input;
using Microsoft.UI;
using Microsoft.UI.Dispatching;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Media.Imaging;
using OpenTrackIR.WinUI.Models;
using OpenTrackIR.WinUI.Runtime;
using OpenTrackIR.WinUI.Services;
using System.Runtime.InteropServices.WindowsRuntime;

namespace OpenTrackIR.WinUI.ViewModels
{
    public sealed class MainShellViewModel : ObservableObject, IDisposable
    {
        private readonly ISettingsStore _settingsStore;
        private readonly ITrackIRRuntimeController _runtimeController;
        private readonly ITrayService _trayService;
        private TrackIRControlState _controlState;
        private TrackIRSnapshot _snapshot;
        private ImageSource? _previewImageSource;
        private byte[] _previewGrayPixels = Array.Empty<byte>();
        private byte[] _previewBgraPixels = Array.Empty<byte>();
        private WriteableBitmap? _previewBitmap;
        private readonly DispatcherQueue? _dispatcherQueue;
        private bool _isAdvancedExpanded;
        private bool _showDetectedBlobCenter = true;
        private bool _isDisposed;
        private CancellationTokenSource? _timeoutCancellationSource;

        public MainShellViewModel()
            : this(new LocalSettingsStore(), new NativeTrackIRRuntimeController(), AppServices.TrayService)
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
            _dispatcherQueue = DispatcherQueue.GetForCurrentThread();
            _controlState = TrackIRUiLogic.Normalize(_settingsStore.Load());
            _snapshot = _runtimeController.CurrentSnapshot;
            _previewImageSource = null;

            RefreshCommand = new RelayCommand(Refresh);
            _runtimeController.SnapshotChanged += OnRuntimeSnapshotChanged;
            _runtimeController.PreviewFrameChanged += OnRuntimePreviewFrameChanged;
            ApplyControlState(_controlState, persist: false);
            _runtimeController.UpdatePresentationState(new TrackIRPresentationState(true, true));
        }

        public ICommand RefreshCommand { get; }

        public string Title => "OpenTrackIR";

        public string Subtitle => "Windows TrackIR preview and controls";

        public string ControlsDescription => "Live controls for TrackIR, video, and mouse movement.";

        public string AdvancedDescription => "Advanced tuning for blob detection, smoothing, keep-awake, and timeout.";

        public string HotkeyHelperText => "Click the field and press a shortcut. It stays active while OpenTrackIR is running, even when the window is hidden.";

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

        public ImageSource? PreviewImageSource => _previewImageSource;

        public Visibility PreviewImageVisibility =>
            _previewImageSource is null ? Visibility.Collapsed : Visibility.Visible;

        public Visibility PreviewPlaceholderVisibility =>
            _previewImageSource is null ? Visibility.Visible : Visibility.Collapsed;

        public Visibility PreviewMarkerVisibility =>
            ShowDetectedBlobCenter && _snapshot.HasPreview && _snapshot.CentroidX.HasValue && _snapshot.CentroidY.HasValue
                ? Visibility.Visible
                : Visibility.Collapsed;

        public Visibility JumpThresholdVisibility =>
            IsAvoidMouseJumpsEnabled ? Visibility.Visible : Visibility.Collapsed;

        public Visibility TimeoutDurationVisibility =>
            IsTimeoutEnabled ? Visibility.Visible : Visibility.Collapsed;

        public double PreviewScaleX => TrackIRUiLogic.PreviewAxisScale(IsVideoFlipHorizontalEnabled);

        public double PreviewScaleY => TrackIRUiLogic.PreviewAxisScale(IsVideoFlipVerticalEnabled);

        public double PreviewRotationDegrees => TrackIRUiLogic.NormalizeRotationDegrees(VideoRotationDegrees);

        public double PreviewMarkerLeft =>
            Math.Clamp((_snapshot.CentroidX ?? 0) - 10, 0, TrackIRUiLogic.FrameWidth - 20);

        public double PreviewMarkerTop =>
            Math.Clamp((_snapshot.CentroidY ?? 0) - 10, 0, TrackIRUiLogic.FrameHeight - 20);

        public void SetPresentationState(bool isWindowVisible, bool isAppActive)
        {
            _runtimeController.UpdatePresentationState(new TrackIRPresentationState(isWindowVisible, isAppActive));
        }

        public void ToggleMouseMovement()
        {
            IsMouseMovementEnabled = TrackIRUiLogic.ToggledMouseMovementState(IsMouseMovementEnabled);
        }

        public void Dispose()
        {
            if (_isDisposed)
            {
                return;
            }

            _isDisposed = true;
            _runtimeController.SnapshotChanged -= OnRuntimeSnapshotChanged;
            _runtimeController.PreviewFrameChanged -= OnRuntimePreviewFrameChanged;
            _timeoutCancellationSource?.Cancel();
            _timeoutCancellationSource?.Dispose();
            _timeoutCancellationSource = null;
            if (_runtimeController is IDisposable disposableRuntimeController)
            {
                disposableRuntimeController.Dispose();
            }
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
            TrackIRControlState previousControlState = _controlState;
            _controlState = TrackIRUiLogic.Normalize(controlState);
            if (persist)
            {
                _settingsStore.Save(_controlState);
            }

            _trayService.UpdateState(_controlState.IsTrackIREnabled, _controlState.IsMouseMovementEnabled);
            _runtimeController.UpdateControlState(_controlState);
            if ((_timeoutCancellationSource is null && TrackIRRuntimeLogic.ShouldScheduleTimeout(_controlState)) ||
                TrackIRRuntimeLogic.ShouldRescheduleTimeout(previousControlState, _controlState))
            {
                SyncTimeoutTask();
            }
            OnControlStateChanged();
        }

        private void OnRuntimeSnapshotChanged(object? sender, TrackIRSnapshot snapshot)
        {
            if (!TrackIRRuntimeLogic.ShouldApplyRuntimeUpdate(_isDisposed))
            {
                return;
            }

            if (_dispatcherQueue is null)
            {
                _snapshot = snapshot;
                OnSnapshotChanged();
                return;
            }

            _dispatcherQueue.TryEnqueue(() =>
            {
                if (!TrackIRRuntimeLogic.ShouldApplyRuntimeUpdate(_isDisposed))
                {
                    return;
                }

                _snapshot = snapshot;
                OnSnapshotChanged();
            });
        }

        private void OnRuntimePreviewFrameChanged(object? sender, TrackIRPreviewFrame? previewFrame)
        {
            if (!TrackIRRuntimeLogic.ShouldApplyRuntimeUpdate(_isDisposed))
            {
                return;
            }

            if (_dispatcherQueue is null)
            {
                ApplyPreviewFrame(previewFrame);
                return;
            }

            _dispatcherQueue.TryEnqueue(() =>
            {
                if (!TrackIRRuntimeLogic.ShouldApplyRuntimeUpdate(_isDisposed))
                {
                    return;
                }

                ApplyPreviewFrame(previewFrame);
            });
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
            OnPropertyChanged(nameof(PreviewScaleX));
            OnPropertyChanged(nameof(PreviewScaleY));
            OnPropertyChanged(nameof(PreviewRotationDegrees));
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
            OnPropertyChanged(nameof(PreviewImageVisibility));
            OnPropertyChanged(nameof(PreviewPlaceholderVisibility));
            OnPropertyChanged(nameof(PreviewMarkerVisibility));
            OnPropertyChanged(nameof(PreviewMarkerLeft));
            OnPropertyChanged(nameof(PreviewMarkerTop));
        }

        private void SyncTimeoutTask()
        {
            _timeoutCancellationSource?.Cancel();
            _timeoutCancellationSource?.Dispose();
            _timeoutCancellationSource = null;

            if (!TrackIRRuntimeLogic.ShouldScheduleTimeout(_controlState))
            {
                return;
            }

            CancellationTokenSource cancellationSource = new();
            _timeoutCancellationSource = cancellationSource;
            _ = Task.Run(async () =>
            {
                try
                {
                    await Task.Delay(TimeSpan.FromSeconds(_controlState.TimeoutSeconds), cancellationSource.Token)
                        .ConfigureAwait(false);
                }
                catch (OperationCanceledException)
                {
                    return;
                }

                if (cancellationSource.IsCancellationRequested ||
                    !TrackIRRuntimeLogic.ShouldApplyRuntimeUpdate(_isDisposed)) {
                    return;
                }

                if (_dispatcherQueue is null)
                {
                    ApplyTimeout();
                    return;
                }

                _dispatcherQueue.TryEnqueue(() =>
                {
                    if (cancellationSource.IsCancellationRequested ||
                        !TrackIRRuntimeLogic.ShouldApplyRuntimeUpdate(_isDisposed))
                    {
                        return;
                    }

                    ApplyTimeout();
                });
            });
        }

        private void ApplyTimeout()
        {
            UpdateControlState(TrackIRRuntimeLogic.TimedOutControlState(_controlState));
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

        private void ApplyPreviewFrame(TrackIRPreviewFrame? previewFrame)
        {
            if (previewFrame is null)
            {
                _previewBitmap = null;
                _previewGrayPixels = Array.Empty<byte>();
                _previewBgraPixels = Array.Empty<byte>();
                _previewImageSource = null;
                OnPropertyChanged(nameof(PreviewImageSource));
                OnPropertyChanged(nameof(PreviewImageVisibility));
                OnPropertyChanged(nameof(PreviewPlaceholderVisibility));
                OnPropertyChanged(nameof(PreviewMarkerVisibility));
                return;
            }

            int grayLength = TrackIRPreviewBitmapLogic.Gray8BufferLength(previewFrame.Width, previewFrame.Height);
            if (_previewGrayPixels.Length != grayLength)
            {
                _previewGrayPixels = new byte[grayLength];
            }

            if (!_runtimeController.TryCopyCurrentPreviewFrame(_previewGrayPixels, out TrackIRPreviewFrame? copiedPreviewFrame) ||
                copiedPreviewFrame is null)
            {
                return;
            }

            int bgraLength = TrackIRPreviewBitmapLogic.Bgra32BufferLength(
                copiedPreviewFrame.Width,
                copiedPreviewFrame.Height
            );
            if (_previewBgraPixels.Length != bgraLength)
            {
                _previewBgraPixels = new byte[bgraLength];
            }

            TrackIRPreviewBitmapLogic.ExpandGray8ToBgra32(_previewGrayPixels, _previewBgraPixels);

            bool didCreateBitmap = _previewBitmap is null ||
                _previewBitmap.PixelWidth != copiedPreviewFrame.Width ||
                _previewBitmap.PixelHeight != copiedPreviewFrame.Height;
            if (didCreateBitmap)
            {
                _previewBitmap = new WriteableBitmap(copiedPreviewFrame.Width, copiedPreviewFrame.Height);
            }

            using Stream pixelStream = _previewBitmap!.PixelBuffer.AsStream();
            pixelStream.Position = 0;
            pixelStream.Write(_previewBgraPixels, 0, _previewBgraPixels.Length);
            _previewBitmap.Invalidate();
            _previewImageSource = _previewBitmap;
            if (didCreateBitmap)
            {
                OnPropertyChanged(nameof(PreviewImageSource));
            }
            OnPropertyChanged(nameof(PreviewImageVisibility));
            OnPropertyChanged(nameof(PreviewPlaceholderVisibility));
            OnPropertyChanged(nameof(PreviewMarkerVisibility));
        }
    }
}
