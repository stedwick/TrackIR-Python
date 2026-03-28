using Microsoft.UI.Xaml;
using OpenTrackIR.WinUI.Services;

namespace OpenTrackIR.WinUI
{
    public partial class App : Application
    {
        private Window? _window;
        private ITrayService? _trayService;

        public App()
        {
            InitializeComponent();
        }

        protected override void OnLaunched(Microsoft.UI.Xaml.LaunchActivatedEventArgs args)
        {
            _trayService = new NotifyIconTrayService();
            AppServices.TrayService = _trayService;
            _window = new MainWindow();
            _window.Activate();
        }
    }
}
