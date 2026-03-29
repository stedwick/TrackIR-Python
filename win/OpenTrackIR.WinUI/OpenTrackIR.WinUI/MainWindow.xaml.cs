using Microsoft.UI.Windowing;
using Microsoft.UI.Dispatching;
using Microsoft.UI.Xaml;
using OpenTrackIR.WinUI.Services;
using System.IO;
using System.ComponentModel;
using System.Runtime.InteropServices;
using OpenTrackIR.WinUI.Models;
using WinRT.Interop;

namespace OpenTrackIR.WinUI
{
    public sealed partial class MainWindow : Window
    {
        private const int GlobalMouseToggleHotkeyId = 0x4F544952;
        private const uint WindowMessageHotkey = 0x0312;
        private const int WindowLongWindowProcedure = -4;
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
        private bool _isDisposed;
        private RegisteredHotkey? _registeredMouseToggleHotkey;

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
            _trayService.Initialize(ShowWindowFromTray, ExitApplication);
        }

        private void OnAppWindowClosing(AppWindow sender, AppWindowClosingEventArgs args)
        {
            if (_allowClose)
            {
                DisposeWindowResources();
                return;
            }

            args.Cancel = true;
            sender.Hide();
        }

        private void ShowWindowFromTray()
        {
            AppWindow.Show();
            Activate();
        }

        private void ExitApplication()
        {
            _dispatcherQueue.TryEnqueue(() =>
            {
                RootView.Dispose();
                Content = null;
                _allowClose = true;
                Close();
            });
        }

        private void OnRootViewModelPropertyChanged(object? sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(RootView.ViewModel.MouseToggleHotkeyText))
            {
                UpdateMouseToggleHotkeyRegistration();
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

            return CallWindowProc(_previousWindowProcedure, windowHandle, message, wParam, lParam);
        }

        private void DisposeWindowResources()
        {
            if (_isDisposed)
            {
                return;
            }

            _isDisposed = true;
            RootView.ViewModel.PropertyChanged -= OnRootViewModelPropertyChanged;
            UnregisterMouseToggleHotkey();
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
    }
}
