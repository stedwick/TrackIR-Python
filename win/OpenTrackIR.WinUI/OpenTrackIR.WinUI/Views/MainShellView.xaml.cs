using Microsoft.UI.Xaml.Controls;
using OpenTrackIR.WinUI.ViewModels;

namespace OpenTrackIR.WinUI.Views
{
    public sealed partial class MainShellView : UserControl
    {
        public MainShellViewModel ViewModel { get; } = new();

        public MainShellView()
        {
            InitializeComponent();
        }
    }
}
