using Microsoft.UI.Windowing;
using Microsoft.UI.Xaml;
using System.Runtime.InteropServices;
using WinRT.Interop;

namespace OpenTrackIR.WinUI.Views
{
    public sealed partial class RecenterOverlayWindow : Window
    {
        private const int OverlayWidth = 120;
        private const int OverlayHeight = 140;

        public RecenterOverlayWindow()
        {
            InitializeComponent();
            ConfigureOverlay();
        }

        private void ConfigureOverlay()
        {
            AppWindow appWindow = AppWindow;
            if (appWindow.Presenter is OverlappedPresenter presenter)
            {
                presenter.IsResizable = false;
                presenter.IsMinimizable = false;
                presenter.IsMaximizable = false;
                presenter.SetBorderAndTitleBar(false, false);
                presenter.IsAlwaysOnTop = true;
            }

            GetMonitorCenter(out int centerX, out int centerY);
            appWindow.MoveAndResize(new Windows.Graphics.RectInt32(
                centerX - OverlayWidth / 2,
                centerY - OverlayHeight / 2,
                OverlayWidth,
                OverlayHeight
            ));
        }

        private static void GetMonitorCenter(out int centerX, out int centerY)
        {
            centerX = GetSystemMetrics(0) / 2;
            centerY = GetSystemMetrics(1) / 2;
        }

        [DllImport("user32.dll")]
        private static extern int GetSystemMetrics(int index);
    }
}
