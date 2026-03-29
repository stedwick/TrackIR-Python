namespace OpenTrackIR.WinUI.Models
{
    public static class MainWindowLogic
    {
        public static bool ShouldBeginExit(bool isDisposed, bool isExitRequested)
        {
            return !isDisposed && !isExitRequested;
        }

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
