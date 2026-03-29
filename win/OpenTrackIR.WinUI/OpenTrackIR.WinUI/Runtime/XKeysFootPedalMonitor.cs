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
        private Dictionary<string, FileStream> _activeStreams = new(StringComparer.OrdinalIgnoreCase);
        private Dictionary<string, bool> _pressedStates = new(StringComparer.OrdinalIgnoreCase);

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
            List<FileStream> activeStreams;
            lock (_syncRoot)
            {
                cancellationSource = _monitorCancellationSource;
                monitorTask = _monitorTask;
                activeStreams = _activeStreams.Values.ToList();
                _activeStreams.Clear();
                _pressedStates.Clear();
                _monitorCancellationSource = null;
                _monitorTask = null;
            }

            foreach (FileStream activeStream in activeStreams)
            {
                activeStream.Dispose();
            }

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
                IReadOnlyList<XKeysHidInterop.XKeysDeviceDescriptor> devices =
                    XKeysHidInterop.EnumerateMatchingDevices();
                if (devices.Count == 0)
                {
                    PublishSnapshot(XKeysReportLogic.Snapshot(
                        isEnabled: true,
                        didDetectPedal: false,
                        isPressed: false
                    ));
                    await DelayForRetryAsync(cancellationToken).ConfigureAwait(false);
                    continue;
                }

                List<OpenedXKeysDevice> openedDevices = OpenDevices(devices);
                if (openedDevices.Count == 0)
                {
                    PublishSnapshot(XKeysReportLogic.Snapshot(
                        isEnabled: true,
                        didDetectPedal: false,
                        isPressed: false
                    ));
                    await DelayForRetryAsync(cancellationToken).ConfigureAwait(false);
                    continue;
                }

                SetActiveDevices(openedDevices);
                PublishAggregateSnapshot();

                try
                {
                    Task[] readTasks = openedDevices
                        .Select(device => Task.Run(
                            () => ReadReports(device, cancellationToken),
                            cancellationToken
                        ))
                        .ToArray();
                    await Task.WhenAll(readTasks).ConfigureAwait(false);
                }
                catch (OperationCanceledException)
                {
                    return;
                }
                finally
                {
                    ClearActiveDevices(openedDevices);
                    foreach (OpenedXKeysDevice openedDevice in openedDevices)
                    {
                        openedDevice.Stream.Dispose();
                    }
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

        private List<OpenedXKeysDevice> OpenDevices(
            IReadOnlyList<XKeysHidInterop.XKeysDeviceDescriptor> devices
        )
        {
            List<OpenedXKeysDevice> openedDevices = new();
            foreach (XKeysHidInterop.XKeysDeviceDescriptor device in devices)
            {
                XKeysHidInterop.XKeysDeviceConnection? connection = XKeysHidInterop.TryOpenDevice(device);
                if (connection is null)
                {
                    continue;
                }

                try
                {
                    FileStream stream = new(
                        connection.Value.Handle,
                        FileAccess.Read,
                        connection.Value.Descriptor.InputReportByteLength,
                        isAsync: false
                    );
                    openedDevices.Add(new OpenedXKeysDevice(connection.Value.Descriptor, stream));
                }
                catch
                {
                    connection.Value.Handle.Dispose();
                }
            }

            return openedDevices;
        }

        private void ReadReports(OpenedXKeysDevice device, CancellationToken cancellationToken)
        {
            byte[] reportBuffer = new byte[device.Descriptor.InputReportByteLength];
            while (!cancellationToken.IsCancellationRequested)
            {
                int bytesRead;
                try
                {
                    bytesRead = device.Stream.Read(reportBuffer, 0, reportBuffer.Length);
                }
                catch (IOException)
                {
                    return;
                }
                catch (ObjectDisposedException)
                {
                    return;
                }

                if (bytesRead <= 0)
                {
                    return;
                }

                bool isPressed = XKeysReportLogic.MiddlePedalPressed(reportBuffer.AsSpan(0, bytesRead));
                UpdatePressedState(device.Descriptor.DevicePath, isPressed);
            }
        }

        private static Task DelayForRetryAsync(CancellationToken cancellationToken)
        {
            return Task.Delay(DetectionRetryDelayMilliseconds, cancellationToken);
        }

        private void SetActiveDevices(IReadOnlyList<OpenedXKeysDevice> openedDevices)
        {
            lock (_syncRoot)
            {
                _activeStreams = openedDevices.ToDictionary(
                    device => device.Descriptor.DevicePath,
                    device => device.Stream,
                    StringComparer.OrdinalIgnoreCase
                );
                _pressedStates = openedDevices.ToDictionary(
                    device => device.Descriptor.DevicePath,
                    _ => false,
                    StringComparer.OrdinalIgnoreCase
                );
            }
        }

        private void ClearActiveDevices(IReadOnlyList<OpenedXKeysDevice> openedDevices)
        {
            lock (_syncRoot)
            {
                foreach (OpenedXKeysDevice openedDevice in openedDevices)
                {
                    _activeStreams.Remove(openedDevice.Descriptor.DevicePath);
                    _pressedStates.Remove(openedDevice.Descriptor.DevicePath);
                }
            }
        }

        private void UpdatePressedState(string devicePath, bool isPressed)
        {
            XKeysMonitorSnapshot? snapshot = null;
            lock (_syncRoot)
            {
                if (!_pressedStates.TryGetValue(devicePath, out bool previousState) ||
                    previousState == isPressed)
                {
                    return;
                }

                _pressedStates[devicePath] = isPressed;
                snapshot = XKeysReportLogic.AggregateSnapshot(
                    isEnabled: true,
                    pressedStates: _pressedStates.Values
                );
            }

            PublishSnapshot(snapshot.Value);
        }

        private void PublishAggregateSnapshot()
        {
            XKeysMonitorSnapshot snapshot;
            lock (_syncRoot)
            {
                snapshot = XKeysReportLogic.AggregateSnapshot(
                    isEnabled: true,
                    pressedStates: _pressedStates.Values
                );
            }

            PublishSnapshot(snapshot);
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

        private readonly record struct OpenedXKeysDevice(
            XKeysHidInterop.XKeysDeviceDescriptor Descriptor,
            FileStream Stream
        );
    }
}
