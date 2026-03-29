using OpenTrackIR.WinUI.Models;

namespace OpenTrackIR.WinUI.Runtime
{
    public sealed class NativeTrackIRRuntimeController : ITrackIRRuntimeController, IDisposable
    {
        private readonly object _syncRoot = new();
        private readonly object _previewSyncRoot = new();
        private TrackIRControlState _controlState = TrackIRUiLogic.CreateDefaultControlState();
        private TrackIRPresentationState _presentationState = new(true, true);
        private CancellationTokenSource? _pollCancellationSource;
        private Task? _pollTask;
        private nint _session;
        private ulong _lastPreviewGeneration;
        private bool _nativeRuntimeUnavailable;
        private readonly WindowsMouseBridge _mouseBridge = new();
        private DateTimeOffset _lastMouseMovementTime = DateTimeOffset.UtcNow;
        private DateTimeOffset? _lastTelemetryPublishTime;
        private TrackIRSnapshot? _lastPublishedTelemetrySnapshot;
        private byte[] _previewPixels = Array.Empty<byte>();

        public TrackIRSnapshot CurrentSnapshot { get; private set; }
        public TrackIRPreviewFrame? CurrentPreviewFrame { get; private set; }

        public event EventHandler<TrackIRSnapshot>? SnapshotChanged;
        public event EventHandler<TrackIRPreviewFrame?>? PreviewFrameChanged;

        public NativeTrackIRRuntimeController()
        {
            CurrentSnapshot = TrackIRRuntimeLogic.IdleSnapshot(_controlState, _presentationState);
        }

        public void UpdateControlState(TrackIRControlState controlState)
        {
            lock (_syncRoot)
            {
                _controlState = TrackIRUiLogic.Normalize(controlState);
            }

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

        public void Refresh()
        {
            _nativeRuntimeUnavailable = false;
            SyncSession(forceRestart: true);
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
            StopSession(waitForShutdown: true);
        }

        private void SyncSession(bool forceRestart)
        {
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
                StopPollLoop();
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

        private void StopPollLoop()
        {
            if (_pollCancellationSource is null)
            {
                return;
            }

            _pollCancellationSource.Cancel();
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
            StopPollLoop();

            if (_session == 0)
            {
                return;
            }

            TryNativeCall(() => TrackIRNativeMethods.TrackIRSessionStop(_session, waitForShutdown));
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
            if (_session == 0 || _nativeRuntimeUnavailable)
            {
                return;
            }

            try
            {
                TrackIRControlState controlState;
                TrackIRPresentationState presentationState;
                lock (_syncRoot)
                {
                    controlState = _controlState;
                    presentationState = _presentationState;
                }

                bool shouldReadSnapshot = TrackIRRuntimeLogic.ShouldReadSnapshot(
                    controlState,
                    presentationState
                );
                if (!shouldReadSnapshot)
                {
                    return;
                }

                TrackIRNativeMethods.TrackIRSessionCopySnapshot(
                    _session,
                    out TrackIRNativeMethods.NativeTrackIRSessionSnapshot nativeSnapshot
                );
                TrackIRSnapshot mappedSnapshot = MapSnapshot(nativeSnapshot);
                TimeSpan? elapsedSinceLastTelemetryPublish = _lastTelemetryPublishTime.HasValue
                    ? DateTimeOffset.UtcNow - _lastTelemetryPublishTime.Value
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
                    _lastTelemetryPublishTime = DateTimeOffset.UtcNow;
                }

                bool didMoveMouse = _mouseBridge.TryApplyTrackingDelta(
                    nativeSnapshot.HasCentroid != 0,
                    nativeSnapshot.CentroidX,
                    nativeSnapshot.CentroidY,
                    controlState
                );
                if (didMoveMouse)
                {
                    _lastMouseMovementTime = DateTimeOffset.UtcNow;
                }
                else if (TrackIRMouseRuntimeLogic.ShouldFireKeepAwake(
                    controlState,
                    DateTimeOffset.UtcNow - _lastMouseMovementTime
                ) && _mouseBridge.TryNudge())
                {
                    _lastMouseMovementTime = DateTimeOffset.UtcNow;
                }

                bool shouldPublishPreview = TrackIRRuntimeLogic.ShouldPublishPreview(
                    controlState,
                    presentationState,
                    nativeSnapshot.HasPreviewFrame != 0
                );

                if (!shouldPublishPreview)
                {
                    PublishPreviewFrame(null);
                    return;
                }

                if (nativeSnapshot.PreviewFrameGeneration == _lastPreviewGeneration)
                {
                    return;
                }

                int frameLength = TrackIRPreviewBitmapLogic.Gray8BufferLength(
                    nativeSnapshot.PreviewWidth,
                    nativeSnapshot.PreviewHeight
                );
                if (frameLength <= 0)
                {
                    PublishPreviewFrame(null);
                    return;
                }

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
                XKeysIndicatorState: _controlState.IsXKeysFastMouseEnabled
                    ? XKeysIndicatorState.NotDetected
                    : XKeysIndicatorState.Disabled,
                HasPreview: nativeSnapshot.HasPreviewFrame != 0,
                IsLowPowerMode: nativeSnapshot.IsLowPowerMode != 0
            );
        }

        private void PublishSnapshot(TrackIRSnapshot snapshot)
        {
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

            if (didChange)
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
