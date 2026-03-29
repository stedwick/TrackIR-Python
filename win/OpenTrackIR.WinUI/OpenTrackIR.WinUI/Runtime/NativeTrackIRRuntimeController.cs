using OpenTrackIR.WinUI.Models;

namespace OpenTrackIR.WinUI.Runtime
{
    public sealed class NativeTrackIRRuntimeController : ITrackIRRuntimeController, IDisposable
    {
        private readonly object _syncRoot = new();
        private TrackIRControlState _controlState = TrackIRUiLogic.CreateDefaultControlState();
        private TrackIRPresentationState _presentationState = new(true, true);
        private CancellationTokenSource? _pollCancellationSource;
        private Task? _pollTask;
        private nint _session;
        private ulong _lastPreviewGeneration;
        private bool _nativeRuntimeUnavailable;

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
                PublishPreviewFrame(null);
                PublishSnapshot(TrackIRRuntimeLogic.IdleSnapshot(controlState, _presentationState));
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
                TrackIRNativeMethods.TrackIRSessionSetLowPowerModeEnabled(
                    _session,
                    !_presentationState.IsWindowVisible || !_presentationState.IsAppActive
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
                    !_presentationState.IsWindowVisible || !_presentationState.IsAppActive
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
        }

        private void PollOnce()
        {
            if (_session == 0 || _nativeRuntimeUnavailable)
            {
                return;
            }

            try
            {
                TrackIRNativeMethods.TrackIRSessionCopySnapshot(_session, out TrackIRNativeMethods.NativeTrackIRSessionSnapshot nativeSnapshot);
                PublishSnapshot(MapSnapshot(nativeSnapshot));

                bool shouldPublishPreview = TrackIRRuntimeLogic.ShouldPublishPreview(
                    _controlState,
                    _presentationState,
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

                int frameLength = nativeSnapshot.PreviewWidth * nativeSnapshot.PreviewHeight;
                if (frameLength <= 0)
                {
                    PublishPreviewFrame(null);
                    return;
                }

                byte[] previewPixels = new byte[frameLength];
                if (!TrackIRNativeMethods.TrackIRSessionCopyPreviewFrame(
                    _session,
                    previewPixels,
                    (nuint)previewPixels.Length,
                    out ulong generation
                ))
                {
                    PublishPreviewFrame(null);
                    return;
                }

                _lastPreviewGeneration = generation;
                PublishPreviewFrame(
                    new TrackIRPreviewFrame(
                        Generation: generation,
                        Width: nativeSnapshot.PreviewWidth,
                        Height: nativeSnapshot.PreviewHeight,
                        Gray8Pixels: previewPixels
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
            CurrentSnapshot = snapshot;
            SnapshotChanged?.Invoke(this, snapshot);
        }

        private void PublishPreviewFrame(TrackIRPreviewFrame? previewFrame)
        {
            bool didChange = !Equals(CurrentPreviewFrame, previewFrame);
            CurrentPreviewFrame = previewFrame;
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
