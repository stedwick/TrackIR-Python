namespace OpenTrackIR.WinUI.Models
{
    public static class MainWindowLogic
    {
        public static bool ShouldProcessTrayWindowAction(
            bool isDisposed,
            bool isExitRequested,
            bool hasContent
        )
        {
            return !isDisposed && !isExitRequested && hasContent;
        }
    }
}
