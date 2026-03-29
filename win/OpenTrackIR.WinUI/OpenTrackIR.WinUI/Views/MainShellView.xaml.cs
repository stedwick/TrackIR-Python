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
            if (e.Key == VirtualKey.Tab)
            {
                return;
            }

            if (e.Key == VirtualKey.Escape)
            {
                e.Handled = true;
                return;
            }

            if (HotkeyCaptureLogic.IsModifierKey((int)e.Key))
            {
                e.Handled = true;
                return;
            }

            string? keyToken = HotkeyCaptureLogic.KeyTokenForVirtualKey((int)e.Key);
            string? hotkeyText = HotkeyCaptureLogic.FormatHotkeyText(
                IsKeyDown(VirtualKey.Control),
                IsKeyDown(VirtualKey.Menu),
                IsKeyDown(VirtualKey.Shift),
                IsKeyDown(VirtualKey.LeftWindows) || IsKeyDown(VirtualKey.RightWindows),
                keyToken
            );

            if (hotkeyText is null)
            {
                e.Handled = true;
                return;
            }

            ViewModel.MouseToggleHotkeyText = hotkeyText;
            e.Handled = true;
        }

        private static bool IsKeyDown(VirtualKey key)
        {
            return InputKeyboardSource.GetKeyStateForCurrentThread(key).HasFlag(CoreVirtualKeyStates.Down);
        }
    }
}
