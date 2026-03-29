using Microsoft.UI.Windowing;
using Microsoft.UI.Dispatching;
using Microsoft.UI.Xaml;
using OpenTrackIR.WinUI.Services;
using System.IO;

namespace OpenTrackIR.WinUI
{
    public sealed partial class MainWindow : Window
    {
        private readonly ITrayService _trayService;
        private readonly DispatcherQueue _dispatcherQueue;
        private bool _allowClose;

        public MainWindow()
        {
            InitializeComponent();
            _trayService = AppServices.TrayService;
            _dispatcherQueue = DispatcherQueue.GetForCurrentThread();
            AppWindow.SetIcon(Path.Combine(AppContext.BaseDirectory, "Assets", "Square44x44Logo.scale-200.png"));
            AppWindow.Closing += OnAppWindowClosing;
            _trayService.Initialize(ShowWindowFromTray, ExitApplication);
        }

        private void OnAppWindowClosing(AppWindow sender, AppWindowClosingEventArgs args)
        {
            if (_allowClose)
            {
                RootView.Dispose();
                _trayService.Dispose();
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
    }
}
