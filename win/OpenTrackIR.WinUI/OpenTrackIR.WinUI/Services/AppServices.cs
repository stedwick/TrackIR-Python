namespace OpenTrackIR.WinUI.Services
{
    public static class AppServices
    {
        public static ITrayService TrayService { get; set; } = new NoOpTrayService();
    }
}
