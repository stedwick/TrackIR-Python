using System.Runtime.InteropServices;
using OpenTrackIR.WinUI.Models;

namespace OpenTrackIR.WinUI.Services
{
    public sealed class NotifyIconTrayService : ITrayService
    {
        private const int WmApp = 0x8000;
        private const int WmCommand = 0x0111;
        private const int WmNull = 0x0000;
        private const int WmLButtonDblClk = 0x0203;
        private const int WmRButtonUp = 0x0205;
        private const uint NifMessage = 0x00000001;
        private const uint NifIcon = 0x00000002;
        private const uint NifTip = 0x00000004;
        private const uint NimAdd = 0x00000000;
        private const uint NimModify = 0x00000001;
        private const uint NimDelete = 0x00000002;
        private const uint WmTrayIcon = WmApp + 1;
        private const uint TpmLeftAlign = 0x0000;
        private const uint TpmBottomAlign = 0x0020;
        private const uint TpmRightButton = 0x0002;
        private const int IdShowWindow = 1001;
        private const int IdExit = 1002;
        private static readonly WndProcDelegate WindowProcedure = WindowProc;
        private static NotifyIconTrayService? s_current;

        private Action? _showWindow;
        private Action? _exitApplication;
        private nint _windowHandle;
        private nint _menuHandle;
        private nint _iconHandle;
        private ushort _windowClassAtom;
        private bool _isInitialized;

        public void Initialize(Action showWindow, Action exitApplication)
        {
            if (_isInitialized)
            {
                return;
            }

            _showWindow = showWindow;
            _exitApplication = exitApplication;
            s_current = this;

            string className = "OpenTrackIRTrayWindow";
            WndClass windowClass = new()
            {
                lpszClassName = className,
                lpfnWndProc = Marshal.GetFunctionPointerForDelegate(WindowProcedure),
                hInstance = GetModuleHandle(null),
            };

            _windowClassAtom = RegisterClass(ref windowClass);
            _windowHandle = CreateWindowEx(
                0,
                className,
                string.Empty,
                0,
                0,
                0,
                0,
                0,
                new nint(-3),
                nint.Zero,
                windowClass.hInstance,
                nint.Zero
            );

            _menuHandle = CreatePopupMenu();
            AppendMenu(_menuHandle, 0x0000, (nuint)IdShowWindow, "Show Window");
            AppendMenu(_menuHandle, 0x0000, (nuint)IdExit, "Exit");

            _iconHandle = LoadImage(
                nint.Zero,
                Path.Combine(AppContext.BaseDirectory, "Assets", "OpenTrackIR.ico"),
                1,
                0,
                0,
                0x0010
            );

            NotifyIcon(NimAdd, BuildIconData(TrayUiLogic.TooltipText(false, false)));
            _isInitialized = true;
        }

        public void UpdateState(bool isTrackIREnabled, bool isMouseMovementEnabled)
        {
            if (!_isInitialized)
            {
                return;
            }

            NotifyIcon(NimModify, BuildIconData(TrayUiLogic.TooltipText(isTrackIREnabled, isMouseMovementEnabled)));
        }

        public void Dispose()
        {
            if (!_isInitialized)
            {
                return;
            }

            NotifyIcon(NimDelete, BuildIconData(string.Empty));

            if (_menuHandle != nint.Zero)
            {
                DestroyMenu(_menuHandle);
                _menuHandle = nint.Zero;
            }

            if (_windowHandle != nint.Zero)
            {
                DestroyWindow(_windowHandle);
                _windowHandle = nint.Zero;
            }

            if (_windowClassAtom != 0)
            {
                UnregisterClass("OpenTrackIRTrayWindow", GetModuleHandle(null));
                _windowClassAtom = 0;
            }

            if (_iconHandle != nint.Zero)
            {
                DestroyIcon(_iconHandle);
                _iconHandle = nint.Zero;
            }

            _isInitialized = false;
            s_current = null;
        }

        private NotifyIconData BuildIconData(string tooltipText)
        {
            return new NotifyIconData
            {
                cbSize = (uint)Marshal.SizeOf<NotifyIconData>(),
                hWnd = _windowHandle,
                uID = 1,
                uFlags = NifMessage | NifIcon | NifTip,
                uCallbackMessage = WmTrayIcon,
                hIcon = _iconHandle,
                szTip = tooltipText,
                szInfo = string.Empty,
                szInfoTitle = string.Empty,
            };
        }

        private nint HandleWindowMessage(nint hwnd, uint message, nuint wParam, nint lParam)
        {
            if (message == WmTrayIcon)
            {
                switch ((uint)lParam)
                {
                    case WmLButtonDblClk:
                        _showWindow?.Invoke();
                        return 0;
                    case WmRButtonUp:
                        ShowContextMenu();
                        return 0;
                }
            }

            if (message == WmCommand)
            {
                int commandId = LowWord(wParam);
                if (commandId == IdShowWindow)
                {
                    _showWindow?.Invoke();
                    return 0;
                }

                if (commandId == IdExit)
                {
                    _exitApplication?.Invoke();
                    return 0;
                }
            }

            return DefWindowProc(hwnd, message, wParam, lParam);
        }

        private void ShowContextMenu()
        {
            if (_menuHandle == nint.Zero || _windowHandle == nint.Zero)
            {
                return;
            }

            GetCursorPos(out Point point);
            SetForegroundWindow(_windowHandle);
            TrackPopupMenuEx(
                _menuHandle,
                TpmLeftAlign | TpmBottomAlign | TpmRightButton,
                point.X,
                point.Y,
                _windowHandle,
                nint.Zero
            );
            PostMessage(_windowHandle, WmNull, 0, 0);
        }

        private static nint WindowProc(nint hwnd, uint message, nuint wParam, nint lParam)
        {
            return s_current?.HandleWindowMessage(hwnd, message, wParam, lParam)
                ?? DefWindowProc(hwnd, message, wParam, lParam);
        }

        private static int LowWord(nuint value)
        {
            return (int)(value & 0xFFFF);
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        private struct WndClass
        {
            public uint style;
            public nint lpfnWndProc;
            public int cbClsExtra;
            public int cbWndExtra;
            public nint hInstance;
            public nint hIcon;
            public nint hCursor;
            public nint hbrBackground;
            public string? lpszMenuName;
            public string lpszClassName;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        private struct NotifyIconData
        {
            public uint cbSize;
            public nint hWnd;
            public uint uID;
            public uint uFlags;
            public uint uCallbackMessage;
            public nint hIcon;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string szTip;
            public uint dwState;
            public uint dwStateMask;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string szInfo;
            public uint uTimeoutOrVersion;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
            public string szInfoTitle;
            public uint dwInfoFlags;
            public Guid guidItem;
            public nint hBalloonIcon;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct Point
        {
            public int X;
            public int Y;
        }

        private delegate nint WndProcDelegate(nint hwnd, uint message, nuint wParam, nint lParam);

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern nint GetModuleHandle(string? lpModuleName);

        [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern ushort RegisterClass(ref WndClass lpWndClass);

        [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern bool UnregisterClass(string lpClassName, nint hInstance);

        [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern nint CreateWindowEx(
            uint dwExStyle,
            string lpClassName,
            string lpWindowName,
            uint dwStyle,
            int x,
            int y,
            int nWidth,
            int nHeight,
            nint hWndParent,
            nint hMenu,
            nint hInstance,
            nint lpParam
        );

        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool DestroyWindow(nint hWnd);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern nint DefWindowProc(nint hWnd, uint msg, nuint wParam, nint lParam);

        [DllImport("shell32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern bool Shell_NotifyIcon(uint dwMessage, ref NotifyIconData lpData);

        [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern bool AppendMenu(nint hMenu, uint uFlags, nuint uIDNewItem, string lpNewItem);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern nint CreatePopupMenu();

        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool DestroyMenu(nint hMenu);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool GetCursorPos(out Point lpPoint);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool SetForegroundWindow(nint hWnd);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool PostMessage(nint hWnd, uint msg, nuint wParam, nint lParam);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool TrackPopupMenuEx(
            nint hMenu,
            uint uFlags,
            int x,
            int y,
            nint hwnd,
            nint lptpm
        );

        [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern nint LoadImage(
            nint hInst,
            string name,
            uint type,
            int cx,
            int cy,
            uint fuLoad
        );

        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool DestroyIcon(nint hIcon);

        private static void NotifyIcon(uint message, NotifyIconData data)
        {
            if (!Shell_NotifyIcon(message, ref data))
            {
                throw new InvalidOperationException("Tray icon update failed.");
            }
        }
    }
}
