using System.IO;
using System.Collections.Concurrent;
using System.Threading;
using OpenTrackIR.WinUI.Models;
using OpenTrackIR.WinUI.Runtime;

Console.WriteLine("OpenTrackIR X-keys HID diagnostic");
Console.WriteLine("Press Ctrl-C to exit.");
Console.WriteLine();

IReadOnlyList<XKeysHidInterop.XKeysDeviceDescriptor> devices = XKeysHidInterop.EnumerateMatchingDevices();
if (devices.Count == 0)
{
    Console.WriteLine("No matching X-keys HID devices found.");
    return;
}

for (int index = 0; index < devices.Count; index += 1)
{
    XKeysHidInterop.XKeysDeviceDescriptor device = devices[index];
    Console.WriteLine(
        $"[{index}] PID=0x{device.ProductId:X4} usage=0x{device.Usage:X4} usagePage=0x{device.UsagePage:X4} reportBytes={device.InputReportByteLength}"
    );
    Console.WriteLine($"    {device.DevicePath}");
}

using CancellationTokenSource cancellationSource = new();
ConcurrentDictionary<string, FileStream> activeStreams = new(StringComparer.OrdinalIgnoreCase);
int exitRequested = 0;
Console.CancelKeyPress += (_, eventArgs) =>
{
    eventArgs.Cancel = true;
    RequestExit();
};

List<Task> readTasks = new();
foreach (XKeysHidInterop.XKeysDeviceDescriptor device in devices)
{
    readTasks.Add(Task.Run(() => ReadDeviceReports(device, activeStreams, cancellationSource.Token), cancellationSource.Token));
}

try
{
    await Task.WhenAll(readTasks).ConfigureAwait(false);
}
catch (OperationCanceledException)
{
}

void RequestExit()
{
    if (Interlocked.Exchange(ref exitRequested, 1) != 0)
    {
        return;
    }

    cancellationSource.Cancel();
    foreach (FileStream stream in activeStreams.Values)
    {
        stream.Dispose();
    }

    Environment.Exit(0);
}

static void ReadDeviceReports(
    XKeysHidInterop.XKeysDeviceDescriptor device,
    ConcurrentDictionary<string, FileStream> activeStreams,
    CancellationToken cancellationToken
)
{
    XKeysHidInterop.XKeysDeviceConnection? connection = XKeysHidInterop.TryOpenDevice(device);
    if (connection is null)
    {
        Console.WriteLine($"Failed to open {device.DevicePath}");
        return;
    }

    using FileStream stream = new(
        connection.Value.Handle,
        FileAccess.Read,
        device.InputReportByteLength,
        isAsync: false
    );
    activeStreams[device.DevicePath] = stream;

    try
    {
        byte[] buffer = new byte[device.InputReportByteLength];
        while (!cancellationToken.IsCancellationRequested)
        {
            int bytesRead;
            try
            {
                bytesRead = stream.Read(buffer, 0, buffer.Length);
            }
            catch (IOException)
            {
                Console.WriteLine($"Device disconnected: {device.DevicePath}");
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

            bool isPressed = XKeysReportLogic.MiddlePedalPressed(buffer.AsSpan(0, bytesRead));
            string hex = Convert.ToHexString(buffer.AsSpan(0, bytesRead));
            Console.WriteLine(
                $"PID=0x{device.ProductId:X4} pressed={isPressed,-5} report={hex}"
            );
        }
    }
    finally
    {
        activeStreams.TryRemove(device.DevicePath, out _);
    }
}
