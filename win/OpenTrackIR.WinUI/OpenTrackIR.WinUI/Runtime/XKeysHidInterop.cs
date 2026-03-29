using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;
using OpenTrackIR.WinUI.Models;

namespace OpenTrackIR.WinUI.Runtime
{
    internal static class XKeysHidInterop
    {
        private const uint DigcfPresent = 0x00000002;
        private const uint DigcfDeviceInterface = 0x00000010;
        private const uint GenericRead = 0x80000000;
        private const uint FileShareRead = 0x00000001;
        private const uint FileShareWrite = 0x00000002;
        private const uint OpenExisting = 3;
        private const uint FileFlagOverlapped = 0x40000000;
        private const int HidpStatusSuccess = 0x00110000;
        private static readonly nint InvalidHandleValue = new(-1);

        internal readonly record struct XKeysDeviceConnection(
            SafeFileHandle Handle,
            int InputReportByteLength
        );

        public static XKeysDeviceConnection? TryOpenMatchingDevice()
        {
            HidD_GetHidGuid(out Guid hidGuid);
            nint deviceInfoSet = SetupDiGetClassDevs(
                ref hidGuid,
                null,
                nint.Zero,
                DigcfPresent | DigcfDeviceInterface
            );

            if (deviceInfoSet == InvalidHandleValue)
            {
                return null;
            }

            try
            {
                uint memberIndex = 0;
                while (true)
                {
                    SpDeviceInterfaceData interfaceData = new()
                    {
                        CbSize = Marshal.SizeOf<SpDeviceInterfaceData>(),
                    };
                    if (!SetupDiEnumDeviceInterfaces(
                        deviceInfoSet,
                        nint.Zero,
                        ref hidGuid,
                        memberIndex,
                        ref interfaceData
                    ))
                    {
                        return null;
                    }

                    memberIndex += 1;
                    if (TryOpenMatchingDeviceConnection(deviceInfoSet, interfaceData) is XKeysDeviceConnection connection)
                    {
                        return connection;
                    }
                }
            }
            finally
            {
                SetupDiDestroyDeviceInfoList(deviceInfoSet);
            }
        }

        private static XKeysDeviceConnection? TryOpenMatchingDeviceConnection(
            nint deviceInfoSet,
            SpDeviceInterfaceData interfaceData
        )
        {
            if (!SetupDiGetDeviceInterfaceDetail(
                deviceInfoSet,
                ref interfaceData,
                nint.Zero,
                0,
                out uint requiredSize,
                nint.Zero
            ) || requiredSize == 0)
            {
                return null;
            }

            nint detailBuffer = Marshal.AllocHGlobal((int)requiredSize);
            try
            {
                Marshal.WriteInt32(detailBuffer, IntPtr.Size == 8 ? 8 : 6);
                if (!SetupDiGetDeviceInterfaceDetail(
                    deviceInfoSet,
                    ref interfaceData,
                    detailBuffer,
                    requiredSize,
                    out _,
                    nint.Zero
                ))
                {
                    return null;
                }

                string? devicePath = Marshal.PtrToStringUni(detailBuffer + 4);
                if (string.IsNullOrWhiteSpace(devicePath))
                {
                    return null;
                }

                SafeFileHandle handle = CreateFile(
                    devicePath,
                    GenericRead,
                    FileShareRead | FileShareWrite,
                    nint.Zero,
                    OpenExisting,
                    FileFlagOverlapped,
                    nint.Zero
                );
                if (handle.IsInvalid)
                {
                    handle.Dispose();
                    return null;
                }

                if (!TryGetMatchingCaps(handle, out HidpCaps caps))
                {
                    handle.Dispose();
                    return null;
                }

                return new XKeysDeviceConnection(
                    Handle: handle,
                    InputReportByteLength: caps.InputReportByteLength
                );
            }
            finally
            {
                Marshal.FreeHGlobal(detailBuffer);
            }
        }

        private static bool TryGetMatchingCaps(SafeFileHandle handle, out HidpCaps caps)
        {
            caps = default;
            HiddAttributes attributes = new()
            {
                Size = Marshal.SizeOf<HiddAttributes>(),
            };
            if (!HidD_GetAttributes(handle, ref attributes))
            {
                return false;
            }

            if (!HidD_GetPreparsedData(handle, out nint preparsedData))
            {
                return false;
            }

            try
            {
                if (HidP_GetCaps(preparsedData, out caps) != HidpStatusSuccess)
                {
                    return false;
                }

                return XKeysReportLogic.IsMatchingFootPedal(
                    attributes.VendorID,
                    attributes.ProductID,
                    caps.UsagePage,
                    caps.Usage
                ) && caps.InputReportByteLength > 0;
            }
            finally
            {
                HidD_FreePreparsedData(preparsedData);
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct SpDeviceInterfaceData
        {
            public int CbSize;
            public Guid InterfaceClassGuid;
            public int Flags;
            public nint Reserved;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct HiddAttributes
        {
            public int Size;
            public ushort VendorID;
            public ushort ProductID;
            public ushort VersionNumber;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct HidpCaps
        {
            public short Usage;
            public short UsagePage;
            public short InputReportByteLength;
            public short OutputReportByteLength;
            public short FeatureReportByteLength;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 17)]
            public short[] Reserved;
            public short NumberLinkCollectionNodes;
            public short NumberInputButtonCaps;
            public short NumberInputValueCaps;
            public short NumberInputDataIndices;
            public short NumberOutputButtonCaps;
            public short NumberOutputValueCaps;
            public short NumberOutputDataIndices;
            public short NumberFeatureButtonCaps;
            public short NumberFeatureValueCaps;
            public short NumberFeatureDataIndices;
        }

        [DllImport("hid.dll")]
        private static extern void HidD_GetHidGuid(out Guid hidGuid);

        [DllImport("setupapi.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern nint SetupDiGetClassDevs(
            ref Guid classGuid,
            string? enumerator,
            nint hwndParent,
            uint flags
        );

        [DllImport("setupapi.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool SetupDiEnumDeviceInterfaces(
            nint deviceInfoSet,
            nint deviceInfoData,
            ref Guid interfaceClassGuid,
            uint memberIndex,
            ref SpDeviceInterfaceData deviceInterfaceData
        );

        [DllImport("setupapi.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool SetupDiGetDeviceInterfaceDetail(
            nint deviceInfoSet,
            ref SpDeviceInterfaceData deviceInterfaceData,
            nint deviceInterfaceDetailData,
            uint deviceInterfaceDetailDataSize,
            out uint requiredSize,
            nint deviceInfoData
        );

        [DllImport("setupapi.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool SetupDiDestroyDeviceInfoList(nint deviceInfoSet);

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern SafeFileHandle CreateFile(
            string fileName,
            uint desiredAccess,
            uint shareMode,
            nint securityAttributes,
            uint creationDisposition,
            uint flagsAndAttributes,
            nint templateFile
        );

        [DllImport("hid.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool HidD_GetAttributes(
            SafeFileHandle hidDeviceObject,
            ref HiddAttributes attributes
        );

        [DllImport("hid.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool HidD_GetPreparsedData(
            SafeFileHandle hidDeviceObject,
            out nint preparsedData
        );

        [DllImport("hid.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool HidD_FreePreparsedData(nint preparsedData);

        [DllImport("hid.dll")]
        private static extern int HidP_GetCaps(nint preparsedData, out HidpCaps capabilities);
    }
}
