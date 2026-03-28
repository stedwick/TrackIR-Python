using Microsoft.UI.Xaml;
using System.IO;

namespace OpenTrackIR.WinUI
{
    public sealed partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            AppWindow.SetIcon(Path.Combine(AppContext.BaseDirectory, "Assets", "Square44x44Logo.scale-200.png"));
        }
    }
}
