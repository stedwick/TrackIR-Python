using Microsoft.UI.Input;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Input;
using OpenTrackIR.WinUI.Models;
using OpenTrackIR.WinUI.ViewModels;
using Windows.System;
using Windows.UI.Core;

namespace OpenTrackIR.WinUI.Views
{
    public sealed partial class MainShellView : UserControl, IDisposable
    {
        private bool _isDisposed;
        public MainShellViewModel ViewModel { get; } = new();

        public MainShellView()
        {
            InitializeComponent();
        }

        public void Dispose()
        {
            if (_isDisposed)
            {
                return;
            }

            _isDisposed = true;
            Bindings?.StopTracking();
            ViewModel.Dispose();
        }

        private void MouseToggleHotkeyCaptureBox_KeyDown(object sender, KeyRoutedEventArgs e)
        {
            string? hotkeyText = CaptureHotkeyFromKeyDown(e);
            if (hotkeyText is not null)
            {
                ViewModel.MouseToggleHotkeyText = hotkeyText;
            }
        }

        private void RecenterHotkeyCaptureBox_KeyDown(object sender, KeyRoutedEventArgs e)
        {
            string? hotkeyText = CaptureHotkeyFromKeyDown(e);
            if (hotkeyText is not null)
            {
                ViewModel.RecenterHotkeyText = hotkeyText;
            }
        }

        private static string? CaptureHotkeyFromKeyDown(KeyRoutedEventArgs e)
        {
            if (e.Key == VirtualKey.Tab)
            {
                return null;
            }

            if (e.Key == VirtualKey.Escape)
            {
                e.Handled = true;
                return null;
            }

            if (HotkeyCaptureLogic.IsModifierKey((int)e.Key))
            {
                e.Handled = true;
                return null;
            }

            string? keyToken = HotkeyCaptureLogic.KeyTokenForVirtualKey((int)e.Key);
            string? hotkeyText = HotkeyCaptureLogic.FormatHotkeyText(
                IsKeyDown(VirtualKey.Control),
                IsKeyDown(VirtualKey.Menu),
                IsKeyDown(VirtualKey.Shift),
                IsKeyDown(VirtualKey.LeftWindows) || IsKeyDown(VirtualKey.RightWindows),
                keyToken
            );

            e.Handled = true;
            return hotkeyText;
        }

        private static bool IsKeyDown(VirtualKey key)
        {
            return InputKeyboardSource.GetKeyStateForCurrentThread(key).HasFlag(CoreVirtualKeyStates.Down);
        }
    }
}
