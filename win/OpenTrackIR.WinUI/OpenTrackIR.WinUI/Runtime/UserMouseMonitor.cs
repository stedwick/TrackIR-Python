using System.Runtime.InteropServices;

namespace OpenTrackIR.WinUI.Runtime
{
    internal static class UserMouseMonitor
    {
        private const int WhMouseLl = 14;
        private const uint WmMouseMove = 0x0200;
        private const uint WmLButtonDown = 0x0201;
        private const uint WmLButtonUp = 0x0202;
        private const uint WmRButtonDown = 0x0204;
        private const uint WmRButtonUp = 0x0205;
        private const uint WmMButtonDown = 0x0207;
        private const uint WmMButtonUp = 0x0208;
        private const uint WmMouseWheel = 0x020A;
        private const uint LlmhfInjected = 0x00000001;

        private static readonly LowLevelMouseProc HookCallback = LowLevelMouseProcCallback;
        private static nint _hookHandle;
        private static long _lastUserMouseMoveTick;
        private static long _isAnyMouseButtonPressed;

        public static void Install()
        {
            if (_hookHandle != 0)
            {
                return;
            }

            _hookHandle = SetWindowsHookEx(WhMouseLl, HookCallback, 0, 0);
        }

        public static void Uninstall()
        {
            if (_hookHandle == 0)
            {
                return;
            }

            UnhookWindowsHookEx(_hookHandle);
            _hookHandle = 0;
            _isAnyMouseButtonPressed = 0;
        }

        public static bool IsUserMouseOverrideActive(int delayMilliseconds, bool mouseButtonOverrideEnabled)
        {
            // If any button is pressed and button override is enabled, override TrackIR
            if (mouseButtonOverrideEnabled && Interlocked.Read(ref _isAnyMouseButtonPressed) != 0)
            {
                return true;
            }

            if (delayMilliseconds <= 0)
            {
                return false;
            }

            long lastTick = Interlocked.Read(ref _lastUserMouseMoveTick);
            if (lastTick == 0)
            {
                return false;
            }

            return Environment.TickCount64 - lastTick < delayMilliseconds;
        }

        private static nint LowLevelMouseProcCallback(int nCode, nint wParam, nint lParam)
        {
            if (nCode >= 0)
            {
                uint message = (uint)wParam;

                // Track button press/release
                switch (message)
                {                     
                    case WmLButtonDown:
                    case WmRButtonDown:
                    case WmMButtonDown:
                        Interlocked.Exchange(ref _isAnyMouseButtonPressed, 1);
                        break;

                    case WmLButtonUp:
                    case WmRButtonUp:
                    case WmMButtonUp:
                        Interlocked.Exchange(ref _isAnyMouseButtonPressed, 0);
                        break;

                    case WmMouseWheel:
                    case WmMouseMove:                    
                        MsllHookStruct hookStruct = Marshal.PtrToStructure<MsllHookStruct>(lParam);
                        if ((hookStruct.Flags & LlmhfInjected) == 0)
                        {
                            Interlocked.Exchange(ref _lastUserMouseMoveTick, Environment.TickCount64);
                        }
                        break;
                }
            }

            return CallNextHookEx(_hookHandle, nCode, wParam, lParam);
        }

        private delegate nint LowLevelMouseProc(int nCode, nint wParam, nint lParam);

        [StructLayout(LayoutKind.Sequential)]
        private struct MsllHookStruct
        {
            public int X;
            public int Y;
            public uint MouseData;
            public uint Flags;
            public uint Time;
            public nuint ExtraInfo;
        }

        [DllImport("user32.dll", EntryPoint = "SetWindowsHookExW", SetLastError = true)]
        private static extern nint SetWindowsHookEx(int idHook, LowLevelMouseProc lpfn, nint hMod, uint dwThreadId);

        [DllImport("user32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool UnhookWindowsHookEx(nint hhk);

        [DllImport("user32.dll")]
        private static extern nint CallNextHookEx(nint hhk, int nCode, nint wParam, nint lParam);
    }
}
