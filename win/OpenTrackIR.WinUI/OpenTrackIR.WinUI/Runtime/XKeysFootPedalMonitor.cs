using System.IO;
using OpenTrackIR.WinUI.Models;

namespace OpenTrackIR.WinUI.Runtime
{
    internal sealed class XKeysFootPedalMonitor : IDisposable
    {
        private const int DetectionRetryDelayMilliseconds = 1000;

        private readonly object _syncRoot = new();
        private CancellationTokenSource? _monitorCancellationSource;
        private Task? _monitorTask;
        private FileStream? _activeStream;

        public XKeysMonitorSnapshot Snapshot { get; private set; } = XKeysReportLogic.Snapshot(
            isEnabled: false,
            didDetectPedal: false,
            isPressed: false
        );

        public event EventHandler<XKeysMonitorSnapshot>? SnapshotChanged;

        public void SetEnabled(bool isEnabled)
        {
            if (isEnabled)
            {
                StartIfNeeded();
                return;
            }

            Stop(waitForShutdown: false);
            PublishSnapshot(XKeysReportLogic.Snapshot(
                isEnabled: false,
                didDetectPedal: false,
                isPressed: false
            ));
        }

        public void Dispose()
        {
            Stop(waitForShutdown: true);
        }

        private void StartIfNeeded()
        {
            lock (_syncRoot)
            {
                if (_monitorTask is { IsCompleted: false })
                {
                    return;
                }

                _monitorCancellationSource = new CancellationTokenSource();
                CancellationToken cancellationToken = _monitorCancellationSource.Token;
                _monitorTask = Task.Run(() => MonitorLoopAsync(cancellationToken), cancellationToken);
            }
        }

        private void Stop(bool waitForShutdown)
        {
            CancellationTokenSource? cancellationSource;
            Task? monitorTask;
            FileStream? activeStream;
            lock (_syncRoot)
            {
                cancellationSource = _monitorCancellationSource;
                monitorTask = _monitorTask;
                activeStream = _activeStream;
                _activeStream = null;
                _monitorCancellationSource = null;
                _monitorTask = null;
            }

            activeStream?.Dispose();
            cancellationSource?.Cancel();
            if (waitForShutdown)
            {
                try
                {
                    monitorTask?.Wait();
                }
                catch (AggregateException)
                {
                }
            }

            cancellationSource?.Dispose();
        }

        private async Task MonitorLoopAsync(CancellationToken cancellationToken)
        {
            while (!cancellationToken.IsCancellationRequested)
            {
                XKeysHidInterop.XKeysDeviceConnection? connection = XKeysHidInterop.TryOpenMatchingDevice();
                if (connection is null)
                {
                    PublishSnapshot(XKeysReportLogic.Snapshot(
                        isEnabled: true,
                        didDetectPedal: false,
                        isPressed: false
                    ));
                    await DelayForRetryAsync(cancellationToken).ConfigureAwait(false);
                    continue;
                }

                FileStream? stream = null;
                try
                {
                    stream = new FileStream(
                        connection.Value.Handle,
                        FileAccess.Read,
                        connection.Value.InputReportByteLength,
                        isAsync: true
                    );
                    SetActiveStream(stream);
                    PublishSnapshot(XKeysReportLogic.Snapshot(
                        isEnabled: true,
                        didDetectPedal: true,
                        isPressed: false
                    ));
                    await ReadReportsAsync(stream, connection.Value.InputReportByteLength, cancellationToken)
                        .ConfigureAwait(false);
                }
                catch (OperationCanceledException)
                {
                    return;
                }
                catch (IOException)
                {
                }
                finally
                {
                    ClearActiveStream(stream);
                    stream?.Dispose();
                }

                if (!cancellationToken.IsCancellationRequested)
                {
                    PublishSnapshot(XKeysReportLogic.Snapshot(
                        isEnabled: true,
                        didDetectPedal: false,
                        isPressed: false
                    ));
                }
            }
        }

        private async Task ReadReportsAsync(
            FileStream stream,
            int inputReportByteLength,
            CancellationToken cancellationToken
        )
        {
            byte[] reportBuffer = new byte[inputReportByteLength];
            while (!cancellationToken.IsCancellationRequested)
            {
                int bytesRead = await stream.ReadAsync(
                    reportBuffer.AsMemory(0, inputReportByteLength),
                    cancellationToken
                ).ConfigureAwait(false);
                if (bytesRead <= 0)
                {
                    return;
                }

                bool isPressed = XKeysReportLogic.MiddlePedalPressed(reportBuffer.AsSpan(0, bytesRead));
                PublishSnapshot(XKeysReportLogic.Snapshot(
                    isEnabled: true,
                    didDetectPedal: true,
                    isPressed: isPressed
                ));
            }
        }

        private static Task DelayForRetryAsync(CancellationToken cancellationToken)
        {
            return Task.Delay(DetectionRetryDelayMilliseconds, cancellationToken);
        }

        private void PublishSnapshot(XKeysMonitorSnapshot snapshot)
        {
            if (Snapshot == snapshot)
            {
                return;
            }

            Snapshot = snapshot;
            SnapshotChanged?.Invoke(this, snapshot);
        }

        private void SetActiveStream(FileStream stream)
        {
            lock (_syncRoot)
            {
                _activeStream = stream;
            }
        }

        private void ClearActiveStream(FileStream? stream)
        {
            lock (_syncRoot)
            {
                if (ReferenceEquals(_activeStream, stream))
                {
                    _activeStream = null;
                }
            }
        }
    }
}
