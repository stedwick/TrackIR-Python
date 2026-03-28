namespace OpenTrackIR.WinUI.ViewModels
{
    public sealed class MainShellViewModel
    {
        public string Title => "OpenTrackIR";

        public string Subtitle => "Windows TrackIR shell";

        public string Description =>
            "This view now owns the shell layout so MainWindow can stay a thin host while runtime and UI logic move into dedicated layers.";
    }
}
