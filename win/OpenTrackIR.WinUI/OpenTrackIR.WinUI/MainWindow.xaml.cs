using Microsoft.UI.Dispatching;
using Microsoft.UI.Windowing;
using Microsoft.UI.Xaml;
using OpenTrackIR.WinUI.Models;
using OpenTrackIR.WinUI.Runtime;
using OpenTrackIR.WinUI.Services;
using OpenTrackIR.WinUI.Views;
using System.ComponentModel;
using System.IO;
using System.Runtime.InteropServices;
using WinRT.Interop;

namespace OpenTrackIR.WinUI
{
    public sealed partial class MainWindow : Window
    {
        private const int GlobalMouseToggleHotkeyId = 0x4F544952;
        private const int GlobalRecenterHotkeyId = 0x4F544953;
        private const uint WindowMessageHotkey = 0x0312;
        private const uint WindowMessageActivate = 0x0006;
        private const uint WindowMessageSize = 0x0005;
        private const uint WindowMessageShowWindow = 0x0018;
        private const int WindowLongWindowProcedure = -4;
        private const nint SizeMinimized = 1;
        private const uint HotkeyModifierAlt = 0x0001;
        private const uint HotkeyModifierControl = 0x0002;
        private const uint HotkeyModifierShift = 0x0004;
        private const uint HotkeyModifierWindows = 0x0008;
        private const uint HotkeyModifierNoRepeat = 0x4000;

        private readonly ITrayService _trayService;
        private readonly DispatcherQueue _dispatcherQueue;
        private readonly nint _windowHandle;
        private readonly WindowProcedure _windowProcedure;
        private readonly nint _windowProcedurePointer;
        private readonly nint _previousWindowProcedure;
        private bool _allowClose;
        private bool _isExitRequested;
        private bool _isDisposed;
        private RegisteredHotkey? _registeredMouseToggleHotkey;
        private RegisteredHotkey? _registeredRecenterHotkey;
        private RecenterOverlayWindow? _recenterOverlay;
        private DispatcherTimer? _recenterReleaseTimer;
        private int _recenterScreenCenterX;
        private int _recenterScreenCenterY;
        private int _recenterVirtualKey;
        private bool _wasMouseMovementEnabledBeforeRecenter;

        private delegate nint WindowProcedure(nint windowHandle, uint message, nint wParam, nint lParam);

        public MainWindow()
        {
            InitializeComponent();
            _trayService = AppServices.TrayService;
            _dispatcherQueue = DispatcherQueue.GetForCurrentThread();
            _windowHandle = WindowNative.GetWindowHandle(this);
            _windowProcedure = WindowProcedureHook;
            _windowProcedurePointer = Marshal.GetFunctionPointerForDelegate(_windowProcedure);
            _previousWindowProcedure = SetWindowLongPtr(
                _windowHandle,
                WindowLongWindowProcedure,
                _windowProcedurePointer
            );
            AppWindow.SetIcon(Path.Combine(AppContext.BaseDirectory, "Assets", "Square44x44Logo.scale-200.png"));
            AppWindow.Closing += OnAppWindowClosing;
            RootView.ViewModel.PropertyChanged += OnRootViewModelPropertyChanged;
            UpdateMouseToggleHotkeyRegistration();
            UpdateRecenterHotkeyRegistration();
            UserMouseMonitor.Install();
            _trayService.Initialize(ShowWindowFromTray, ExitApplication);
            UpdateRuntimePresentationState();
        }

        private void OnAppWindowClosing(AppWindow sender, AppWindowClosingEventArgs args)
        {
            if (_allowClose)
            {
                RootView.ViewModel.BeginShutdown();
                DisposeWindowResources();
                return;
            }

            args.Cancel = true;
            sender.Hide();
            UpdateRuntimePresentationState();
        }

        private void ShowWindowFromTray()
        {
            if (!MainWindowLogic.ShouldProcessTrayWindowAction(
                _isDisposed,
                _isExitRequested,
                Content is not null
            ))
            {
                return;
            }

            AppWindow? appWindow = AppWindow;
            if (appWindow is null)
            {
                return;
            }

            appWindow.Show();
            Activate();
            UpdateRuntimePresentationState();
        }

        private void ExitApplication()
        {
            if (!MainWindowLogic.ShouldBeginExit(_isDisposed, _isExitRequested))
            {
                return;
            }

            _isExitRequested = true;
            _allowClose = true;
            if (_dispatcherQueue.HasThreadAccess)
            {
                RootView.ViewModel.BeginShutdown();
                Close();
                return;
            }

            _dispatcherQueue.TryEnqueue(() =>
            {
                RootView.ViewModel.BeginShutdown();
                Close();
            });
        }

        private void OnRootViewModelPropertyChanged(object? sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(RootView.ViewModel.MouseToggleHotkeyText))
            {
                UpdateMouseToggleHotkeyRegistration();
            }

            if (e.PropertyName == nameof(RootView.ViewModel.RecenterHotkeyText))
            {
                UpdateRecenterHotkeyRegistration();
            }
        }

        private void UpdateMouseToggleHotkeyRegistration()
        {
            RegisteredHotkey? previousHotkey = _registeredMouseToggleHotkey;
            if (!HotkeyCaptureLogic.TryParseHotkeyText(
                    RootView.ViewModel.MouseToggleHotkeyText,
                    out RegisteredHotkey nextHotkey))
            {
                UnregisterMouseToggleHotkey();
                _registeredMouseToggleHotkey = null;
                return;
            }

            if (previousHotkey.HasValue && previousHotkey.Value.Equals(nextHotkey))
            {
                return;
            }

            UnregisterMouseToggleHotkey();
            if (TryRegisterMouseToggleHotkey(nextHotkey))
            {
                _registeredMouseToggleHotkey = nextHotkey;
                return;
            }

            if (previousHotkey.HasValue && TryRegisterMouseToggleHotkey(previousHotkey.Value))
            {
                _registeredMouseToggleHotkey = previousHotkey.Value;
                return;
            }

            _registeredMouseToggleHotkey = null;
        }

        private bool TryRegisterMouseToggleHotkey(RegisteredHotkey hotkey)
        {
            return RegisterHotKey(
                _windowHandle,
                GlobalMouseToggleHotkeyId,
                HotkeyModifierFlags(hotkey),
                (uint)hotkey.VirtualKeyCode
            );
        }

        private void UnregisterMouseToggleHotkey()
        {
            if (_registeredMouseToggleHotkey.HasValue)
            {
                UnregisterHotKey(_windowHandle, GlobalMouseToggleHotkeyId);
            }
        }

        private nint WindowProcedureHook(nint windowHandle, uint message, nint wParam, nint lParam)
        {
            if (message == WindowMessageHotkey && wParam == GlobalMouseToggleHotkeyId)
            {
                RootView.ViewModel.ToggleMouseMovement();
                return 0;
            }

            if (message == WindowMessageHotkey && wParam == GlobalRecenterHotkeyId)
            {
                BeginRecenterHold();
                return 0;
            }

            if (message == WindowMessageActivate ||
                message == WindowMessageShowWindow ||
                (message == WindowMessageSize && wParam == SizeMinimized))
            {
                _dispatcherQueue.TryEnqueue(UpdateRuntimePresentationState);
            }

            return CallWindowProc(_previousWindowProcedure, windowHandle, message, wParam, lParam);
        }

        private void UpdateRuntimePresentationState()
        {
            if (!MainWindowLogic.ShouldProcessTrayWindowAction(
                _isDisposed,
                _isExitRequested,
                Content is not null
            ))
            {
                return;
            }

            bool isWindowVisible = IsWindowVisible(_windowHandle);
            bool isWindowMinimized = IsIconic(_windowHandle);
            bool isWindowFocused = GetForegroundWindow() == _windowHandle;
            TrackIRPresentationState presentationState = TrackIRRuntimeLogic.PresentationState(
                isWindowVisible,
                isWindowMinimized,
                isWindowFocused
            );
            RootView.ViewModel.SetPresentationState(
                presentationState.IsWindowVisible,
                presentationState.IsAppActive
            );
        }

        private void UpdateRecenterHotkeyRegistration()
        {
            RegisteredHotkey? previousHotkey = _registeredRecenterHotkey;
            if (!HotkeyCaptureLogic.TryParseHotkeyText(
                    RootView.ViewModel.RecenterHotkeyText,
                    out RegisteredHotkey nextHotkey))
            {
                UnregisterRecenterHotkey();
                _registeredRecenterHotkey = null;
                return;
            }

            if (previousHotkey.HasValue && previousHotkey.Value.Equals(nextHotkey))
            {
                return;
            }

            UnregisterRecenterHotkey();
            if (TryRegisterRecenterHotkey(nextHotkey))
            {
                _registeredRecenterHotkey = nextHotkey;
                return;
            }

            if (previousHotkey.HasValue && TryRegisterRecenterHotkey(previousHotkey.Value))
            {
                _registeredRecenterHotkey = previousHotkey.Value;
                return;
            }

            _registeredRecenterHotkey = null;
        }

        private bool TryRegisterRecenterHotkey(RegisteredHotkey hotkey)
        {
            return RegisterHotKey(
                _windowHandle,
                GlobalRecenterHotkeyId,
                HotkeyModifierFlags(hotkey),
                (uint)hotkey.VirtualKeyCode
            );
        }

        private void UnregisterRecenterHotkey()
        {
            if (_registeredRecenterHotkey.HasValue)
            {
                UnregisterHotKey(_windowHandle, GlobalRecenterHotkeyId);
            }
        }

        private void BeginRecenterHold()
        {
            if (_recenterOverlay is not null || !_registeredRecenterHotkey.HasValue)
            {
                return;
            }

            _recenterVirtualKey = _registeredRecenterHotkey.Value.VirtualKeyCode;
            _recenterScreenCenterX = (GetSystemMetrics(0) / 2) + 36;
            _recenterScreenCenterY = GetSystemMetrics(1) / 2;

            _wasMouseMovementEnabledBeforeRecenter = RootView.ViewModel.IsMouseMovementEnabled;
            RootView.ViewModel.IsMouseMovementEnabled = false;

            SetCursorPos(_recenterScreenCenterX, _recenterScreenCenterY);

            _recenterOverlay = new RecenterOverlayWindow();
            _recenterOverlay.Activate();

            _recenterReleaseTimer = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(30) };
            _recenterReleaseTimer.Tick += OnRecenterReleaseTimerTick;
            _recenterReleaseTimer.Start();
        }

        private void OnRecenterReleaseTimerTick(object? sender, object e)
        {
            // Hold cursor at screen center while key is pressed
            SetCursorPos(_recenterScreenCenterX, _recenterScreenCenterY);

            if ((GetAsyncKeyState(_recenterVirtualKey) & 0x8000) != 0)
            {
                return;
            }

            CommitRecenter();
        }

        private void CommitRecenter()
        {
            _recenterReleaseTimer?.Stop();
            _recenterReleaseTimer = null;

            // Clear calibration so the next poll frame auto-calibrates with the live centroid
            RootView.ViewModel.RecenterCursorCommand.Execute(null);

            // Pin cursor to screen center so auto-calibration anchors here
            SetCursorPos(_recenterScreenCenterX, _recenterScreenCenterY);

            if (_wasMouseMovementEnabledBeforeRecenter)
            {
                RootView.ViewModel.IsMouseMovementEnabled = true;
            }

            _recenterOverlay?.Close();
            _recenterOverlay = null;
        }

        private void DisposeWindowResources()
        {
            if (_isDisposed)
            {
                return;
            }

            _isExitRequested = true;
            _isDisposed = true;
            RootView.ViewModel.PropertyChanged -= OnRootViewModelPropertyChanged;
            UnregisterMouseToggleHotkey();
            UnregisterRecenterHotkey();
            CommitRecenter();
            UserMouseMonitor.Uninstall();
            SetWindowLongPtr(_windowHandle, WindowLongWindowProcedure, _previousWindowProcedure);
            RootView.Dispose();
            _trayService.Dispose();
        }

        private static uint HotkeyModifierFlags(RegisteredHotkey hotkey)
        {
            uint flags = HotkeyModifierNoRepeat;
            if (hotkey.IsAltPressed)
            {
                flags |= HotkeyModifierAlt;
            }

            if (hotkey.IsControlPressed)
            {
                flags |= HotkeyModifierControl;
            }

            if (hotkey.IsShiftPressed)
            {
                flags |= HotkeyModifierShift;
            }

            if (hotkey.IsWindowsPressed)
            {
                flags |= HotkeyModifierWindows;
            }

            return flags;
        }

        [DllImport("user32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool RegisterHotKey(nint windowHandle, int id, uint modifiers, uint virtualKeyCode);

        [DllImport("user32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool UnregisterHotKey(nint windowHandle, int id);

        [DllImport("user32.dll", EntryPoint = "SetWindowLongPtrW", SetLastError = true)]
        private static extern nint SetWindowLongPtr(nint windowHandle, int index, nint newLong);

        [DllImport("user32.dll", EntryPoint = "CallWindowProcW")]
        private static extern nint CallWindowProc(
            nint previousWindowProcedure,
            nint windowHandle,
            uint message,
            nint wParam,
            nint lParam
        );

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool IsWindowVisible(nint windowHandle);

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool IsIconic(nint windowHandle);

        [DllImport("user32.dll")]
        private static extern nint GetForegroundWindow();

        [DllImport("user32.dll")]
        private static extern short GetAsyncKeyState(int virtualKeyCode);

        [DllImport("user32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool SetCursorPos(int x, int y);

        [DllImport("user32.dll")]
        private static extern int GetSystemMetrics(int index);
    }
}
