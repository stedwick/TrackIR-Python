using OpenTrackIR.WinUI.Models;

namespace OpenTrackIR.WinUI.Runtime
{
    public sealed class NativeTrackIRRuntimeController : ITrackIRRuntimeController, IDisposable
    {
        private readonly object _syncRoot = new();
        private readonly object _previewSyncRoot = new();
        private TrackIRControlState _controlState = TrackIRUiLogic.CreateDefaultControlState();
        private TrackIRPresentationState _presentationState = new(true, true);
        private XKeysMonitorSnapshot _xKeysMonitorSnapshot = XKeysReportLogic.Snapshot(
            isEnabled: false,
            didDetectPedal: false,
            isPressed: false
        );
        private CancellationTokenSource? _pollCancellationSource;
        private Task? _pollTask;
        private nint _session;
        private ulong _lastPreviewGeneration;
        private bool _nativeRuntimeUnavailable;
        private readonly WindowsMouseBridge _mouseBridge = new();
        private readonly XKeysFootPedalMonitor _xKeysFootPedalMonitor = new();
        private bool _wasUserMouseOverrideActive;
        private DateTimeOffset _lastMouseMovementTime = DateTimeOffset.UtcNow;
        private DateTimeOffset? _lastTelemetryPublishTime;
        private TrackIRSnapshot? _lastPublishedTelemetrySnapshot;
        private byte[] _previewPixels = Array.Empty<byte>();
        private bool _isStopped;
        private bool _isDisposed;

        public TrackIRSnapshot CurrentSnapshot { get; private set; }
        public TrackIRPreviewFrame? CurrentPreviewFrame { get; private set; }

        public event EventHandler<TrackIRSnapshot>? SnapshotChanged;
        public event EventHandler<TrackIRPreviewFrame?>? PreviewFrameChanged;

        public NativeTrackIRRuntimeController()
        {
            CurrentSnapshot = TrackIRRuntimeLogic.IdleSnapshot(_controlState, _presentationState);
            _xKeysFootPedalMonitor.SnapshotChanged += OnXKeysSnapshotChanged;
        }

        public void UpdateControlState(TrackIRControlState controlState)
        {
            lock (_syncRoot)
            {
                _controlState = TrackIRUiLogic.Normalize(controlState);
            }

            SyncXKeysMonitor();
            SyncSession(forceRestart: false);
        }

        public void UpdatePresentationState(TrackIRPresentationState presentationState)
        {
            lock (_syncRoot)
            {
                _presentationState = presentationState;
            }

            ApplyPresentationState();
            PublishSnapshot(CurrentSnapshot with
            {
                IsLowPowerMode = TrackIRRuntimeLogic.ShouldEnableLowPowerMode(_controlState, _presentationState),
            });
            if (!TrackIRRuntimeLogic.ShouldPublishPreview(_controlState, _presentationState, hasPreviewFrame: true))
            {
                PublishPreviewFrame(null);
            }
        }

        public void RecenterCursor()
        {
            if (_isStopped)
            {
                return;
            }

            _mouseBridge.ClearAbsoluteCalibration();
        }

        public void Refresh()
        {
            if (_isStopped)
            {
                return;
            }

            _nativeRuntimeUnavailable = false;
            SyncSession(forceRestart: true);
        }

        public void Stop()
        {
            if (_isStopped)
            {
                return;
            }

            _isStopped = true;
            _xKeysFootPedalMonitor.SnapshotChanged -= OnXKeysSnapshotChanged;
            _xKeysFootPedalMonitor.SetEnabled(false);
            StopSession(waitForShutdown: false);
            PublishPreviewFrame(null);
        }

        public bool TryCopyCurrentPreviewFrame(byte[] destination, out TrackIRPreviewFrame? previewFrame)
        {
            lock (_previewSyncRoot)
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
                if (destination.Length < requiredLength || _previewPixels.Length < requiredLength)
                {
                    previewFrame = null;
                    return false;
                }

                Array.Copy(_previewPixels, destination, requiredLength);
                return true;
            }
        }

        public void Dispose()
        {
            if (_isDisposed)
            {
                return;
            }

            _isDisposed = true;
            Stop();
        }

        private void OnXKeysSnapshotChanged(object? sender, XKeysMonitorSnapshot snapshot)
        {
            if (_isStopped)
            {
                return;
            }

            lock (_syncRoot)
            {
                _xKeysMonitorSnapshot = snapshot;
            }

            PublishSnapshot(CurrentSnapshot with { XKeysIndicatorState = snapshot.IndicatorState });
        }

        private void SyncSession(bool forceRestart)
        {
            if (_isStopped)
            {
                return;
            }

            TrackIRControlState controlState;
            lock (_syncRoot)
            {
                controlState = _controlState;
            }

            if (!TrackIRRuntimeLogic.ShouldRunSession(controlState))
            {
                StopSession(waitForShutdown: false);
                _mouseBridge.Reset();
                PublishPreviewFrame(null);
                PublishSnapshot(TrackIRRuntimeLogic.IdleSnapshot(controlState, _presentationState));
                _lastTelemetryPublishTime = null;
                _lastPublishedTelemetrySnapshot = null;
                return;
            }

            if (_nativeRuntimeUnavailable)
            {
                PublishSnapshot(TrackIRRuntimeLogic.MissingNativeRuntimeSnapshot(controlState, _presentationState));
                return;
            }

            if (!EnsureSessionCreated())
            {
                PublishSnapshot(TrackIRRuntimeLogic.MissingNativeRuntimeSnapshot(controlState, _presentationState));
                return;
            }

            if (forceRestart)
            {
                StopPollLoop(waitForShutdown: true);
                TryNativeCall(() => TrackIRNativeMethods.TrackIRSessionStop(_session, true));
            }

            TryNativeCall(() =>
            {
                TrackIRNativeMethods.TrackIRSessionSetMaximumTrackingFramesPerSecond(
                    _session,
                    controlState.VideoFramesPerSecond
                );
                TrackIRNativeMethods.TrackIRSessionSetVideoEnabled(_session, controlState.IsVideoEnabled);
                TrackIRNativeMethods.TrackIRSessionSetMinimumBlobAreaPoints(
                    _session,
                    controlState.MinimumBlobAreaPoints
                );
                TrackIRNativeMethods.TrackIRSessionSetCentroidMode(
                    _session,
                    TrackIRRuntimeLogic.RegularizedBinaryCentroidMode
                );
                TrackIRNativeMethods.TrackIRSessionSetLowPowerModeEnabled(
                    _session,
                    TrackIRRuntimeLogic.ShouldEnableLowPowerMode(controlState, _presentationState)
                );
                TrackIRNativeMethods.TrackIRSessionStart(_session);
            });

            StartPollLoopIfNeeded();
        }

        private void SyncXKeysMonitor()
        {
            if (_isStopped)
            {
                return;
            }

            _xKeysFootPedalMonitor.SetEnabled(_controlState.IsXKeysFastMouseEnabled);
        }

        private void ApplyPresentationState()
        {
            if (_session == 0 || _nativeRuntimeUnavailable)
            {
                return;
            }

            TryNativeCall(() =>
            {
                TrackIRNativeMethods.TrackIRSessionSetLowPowerModeEnabled(
                    _session,
                    TrackIRRuntimeLogic.ShouldEnableLowPowerMode(_controlState, _presentationState)
                );
            });
        }

        private bool EnsureSessionCreated()
        {
            if (_isStopped)
            {
                return false;
            }

            if (_session != 0)
            {
                return true;
            }

            try
            {
                _session = TrackIRNativeMethods.TrackIRSessionCreate();
                if (_session == 0)
                {
                    _nativeRuntimeUnavailable = true;
                    return false;
                }

                return true;
            }
            catch (DllNotFoundException)
            {
                _nativeRuntimeUnavailable = true;
                return false;
            }
            catch (EntryPointNotFoundException)
            {
                _nativeRuntimeUnavailable = true;
                return false;
            }
        }

        private void StartPollLoopIfNeeded()
        {
            if (_isStopped)
            {
                return;
            }

            if (_pollTask is { IsCompleted: false })
            {
                return;
            }

            _pollCancellationSource = new CancellationTokenSource();
            CancellationToken cancellationToken = _pollCancellationSource.Token;
            _pollTask = Task.Run(async () =>
            {
                while (!cancellationToken.IsCancellationRequested)
                {
                    PollOnce();
                    int delayMilliseconds;
                    lock (_syncRoot)
                    {
                        delayMilliseconds = TrackIRRuntimeLogic.PollIntervalMilliseconds(_controlState, _presentationState);
                    }

                    await Task.Delay(delayMilliseconds, cancellationToken).ConfigureAwait(false);
                }
            }, cancellationToken);
        }

        private void StopPollLoop(bool waitForShutdown)
        {
            if (_pollCancellationSource is null)
            {
                return;
            }

            _pollCancellationSource.Cancel();
            if (!waitForShutdown)
            {
                return;
            }

            try
            {
                _pollTask?.Wait();
            }
            catch (AggregateException)
            {
            }
            finally
            {
                _pollCancellationSource.Dispose();
                _pollCancellationSource = null;
                _pollTask = null;
            }
        }

        private void StopSession(bool waitForShutdown)
        {
            StopPollLoop(waitForShutdown);

            if (_session == 0)
            {
                return;
            }

            TryNativeCall(() => TrackIRNativeMethods.TrackIRSessionStop(_session, waitForShutdown));
            if (!waitForShutdown)
            {
                _lastPreviewGeneration = 0;
                _lastTelemetryPublishTime = null;
                _lastPublishedTelemetrySnapshot = null;
                lock (_previewSyncRoot)
                {
                    _previewPixels = Array.Empty<byte>();
                }
                _mouseBridge.Reset();
                return;
            }

            TryNativeCall(() => TrackIRNativeMethods.TrackIRSessionDestroy(_session));
            _session = 0;
            _lastPreviewGeneration = 0;
            _lastTelemetryPublishTime = null;
            _lastPublishedTelemetrySnapshot = null;
            lock (_previewSyncRoot)
            {
                _previewPixels = Array.Empty<byte>();
            }
            _mouseBridge.Reset();
        }

        private void PollOnce()
        {
            if (_isStopped || _session == 0 || _nativeRuntimeUnavailable)
            {
                return;
            }

            try
            {
                TrackIRControlState controlState;
                TrackIRPresentationState presentationState;
                XKeysMonitorSnapshot xKeysMonitorSnapshot;
                lock (_syncRoot)
                {
                    controlState = _controlState;
                    presentationState = _presentationState;
                    xKeysMonitorSnapshot = _xKeysMonitorSnapshot;
                }

                double effectiveMouseSpeed = TrackIRMouseRuntimeLogic.EffectiveMouseSpeed(
                    controlState.MouseMovementSpeed,
                    controlState.IsXKeysFastMouseEnabled,
                    xKeysMonitorSnapshot.IsPressed
                );
                DateTimeOffset now = DateTimeOffset.UtcNow;
                bool didMoveMouse = false;
                bool shouldReadSnapshot = TrackIRRuntimeLogic.ShouldReadSnapshot(
                    controlState,
                    presentationState
                );
                if (shouldReadSnapshot)
                {
                    TrackIRNativeMethods.TrackIRSessionCopySnapshot(
                        _session,
                        out TrackIRNativeMethods.NativeTrackIRSessionSnapshot nativeSnapshot
                    );
                    TrackIRSnapshot mappedSnapshot = MapSnapshot(nativeSnapshot);
                    TimeSpan? elapsedSinceLastTelemetryPublish = _lastTelemetryPublishTime.HasValue
                        ? now - _lastTelemetryPublishTime.Value
                        : null;
                    if (TrackIRRuntimeLogic.ShouldPublishTelemetry(
                        presentationState.IsAppActive,
                        mappedSnapshot,
                        _lastPublishedTelemetrySnapshot,
                        elapsedSinceLastTelemetryPublish,
                        TrackIRRuntimeLogic.VisibleTelemetryFramesPerSecond
                    ))
                    {
                        PublishSnapshot(mappedSnapshot);
                        _lastPublishedTelemetrySnapshot = mappedSnapshot;
                        _lastTelemetryPublishTime = now;
                    }

                    bool isUserMouseOverrideActive = UserMouseMonitor.IsUserMouseOverrideActive(
                        controlState.MouseOverrideDelayMilliseconds,
                        controlState.IsMouseButtonOverrideEnabled
                    );

                    if (isUserMouseOverrideActive)
                    {
                        _wasUserMouseOverrideActive = true;
                    }
                    else
                    {
                        if (_wasUserMouseOverrideActive)
                        {
                            _wasUserMouseOverrideActive = false;
                            _mouseBridge.Reset();
                        }

                        didMoveMouse = _mouseBridge.TryApplyTrackingDelta(
                            nativeSnapshot.HasCentroid != 0,
                            nativeSnapshot.CentroidX,
                            nativeSnapshot.CentroidY,
                            controlState,
                            effectiveMouseSpeed
                        );
                    }

                    bool shouldPublishPreview = TrackIRRuntimeLogic.ShouldPublishPreview(
                        controlState,
                        presentationState,
                        nativeSnapshot.HasPreviewFrame != 0
                    );

                    if (!shouldPublishPreview)
                    {
                        PublishPreviewFrame(null);
                    }
                    else if (nativeSnapshot.PreviewFrameGeneration != _lastPreviewGeneration)
                    {
                        int frameLength = TrackIRPreviewBitmapLogic.Gray8BufferLength(
                            nativeSnapshot.PreviewWidth,
                            nativeSnapshot.PreviewHeight
                        );
                        if (frameLength <= 0)
                        {
                            PublishPreviewFrame(null);
                        }
                        else
                        {
                            ulong generation;
                            lock (_previewSyncRoot)
                            {
                                if (_previewPixels.Length != frameLength)
                                {
                                    _previewPixels = new byte[frameLength];
                                }

                                if (!TrackIRNativeMethods.TrackIRSessionCopyPreviewFrame(
                                    _session,
                                    _previewPixels,
                                    (nuint)frameLength,
                                    out generation
                                ))
                                {
                                    PublishPreviewFrame(null);
                                    return;
                                }
                            }

                            _lastPreviewGeneration = generation;
                            PublishPreviewFrame(
                                new TrackIRPreviewFrame(
                                    Generation: generation,
                                    Width: nativeSnapshot.PreviewWidth,
                                    Height: nativeSnapshot.PreviewHeight
                                )
                            );
                        }
                    }
                }

                if (didMoveMouse)
                {
                    _lastMouseMovementTime = now;
                }
                else if (TrackIRMouseRuntimeLogic.ShouldFireKeepAwake(
                    controlState,
                    now - _lastMouseMovementTime
                ) && _mouseBridge.TryNudge(controlState))
                {
                    _lastMouseMovementTime = now;
                }
            }
            catch (DllNotFoundException)
            {
                _nativeRuntimeUnavailable = true;
                PublishSnapshot(TrackIRRuntimeLogic.MissingNativeRuntimeSnapshot(_controlState, _presentationState));
            }
            catch (EntryPointNotFoundException)
            {
                _nativeRuntimeUnavailable = true;
                PublishSnapshot(TrackIRRuntimeLogic.MissingNativeRuntimeSnapshot(_controlState, _presentationState));
            }
        }

        private TrackIRSnapshot MapSnapshot(TrackIRNativeMethods.NativeTrackIRSessionSnapshot nativeSnapshot)
        {
            XKeysIndicatorState xKeysIndicatorState;
            lock (_syncRoot)
            {
                xKeysIndicatorState = _xKeysMonitorSnapshot.IndicatorState;
            }

            return new TrackIRSnapshot(
                Phase: nativeSnapshot.Phase switch
                {
                    1 => TrackIRRuntimePhase.Starting,
                    2 => TrackIRRuntimePhase.Streaming,
                    3 => TrackIRRuntimePhase.Unavailable,
                    4 => TrackIRRuntimePhase.Failed,
                    _ => TrackIRRuntimePhase.Idle,
                },
                ErrorDescription: nativeSnapshot.HasErrorMessage != 0 ? nativeSnapshot.ErrorMessage : null,
                FrameIndex: nativeSnapshot.FrameIndex,
                SourceFrameRate: nativeSnapshot.HasFrameRate != 0 ? nativeSnapshot.FrameRate : null,
                CentroidX: nativeSnapshot.HasCentroid != 0 ? nativeSnapshot.CentroidX : null,
                CentroidY: nativeSnapshot.HasCentroid != 0 ? nativeSnapshot.CentroidY : null,
                PacketType: nativeSnapshot.HasPacketType != 0 ? nativeSnapshot.PacketType : null,
                DeviceLabel: nativeSnapshot.Phase switch
                {
                    2 => "TrackIR 5",
                    3 => "No device",
                    4 => "Error",
                    1 => "Opening",
                    _ => "Idle",
                },
                BackendLabel: "C + libusb",
                XKeysIndicatorState: xKeysIndicatorState,
                HasPreview: nativeSnapshot.HasPreviewFrame != 0,
                IsLowPowerMode: nativeSnapshot.IsLowPowerMode != 0
            );
        }

        private void PublishSnapshot(TrackIRSnapshot snapshot)
        {
            if (_isStopped)
            {
                CurrentSnapshot = snapshot;
                return;
            }

            if (!TrackIRRuntimeLogic.ShouldPublishSnapshot(CurrentSnapshot, snapshot))
            {
                return;
            }

            CurrentSnapshot = snapshot;
            SnapshotChanged?.Invoke(this, snapshot);
        }

        private void PublishPreviewFrame(TrackIRPreviewFrame? previewFrame)
        {
            bool didChange;
            lock (_previewSyncRoot)
            {
                didChange = !Equals(CurrentPreviewFrame, previewFrame);
                CurrentPreviewFrame = previewFrame;
                if (previewFrame is null)
                {
                    _previewPixels = Array.Empty<byte>();
                }
            }

            if (didChange && !_isStopped)
            {
                PreviewFrameChanged?.Invoke(this, previewFrame);
            }
        }

        private void TryNativeCall(Action action)
        {
            try
            {
                action();
            }
            catch (DllNotFoundException)
            {
                _nativeRuntimeUnavailable = true;
            }
            catch (EntryPointNotFoundException)
            {
                _nativeRuntimeUnavailable = true;
            }
        }
    }
}
