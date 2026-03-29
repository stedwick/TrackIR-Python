namespace OpenTrackIR.WinUI.Models
{
    public static class XKeysReportLogic
    {
        public const int VendorId = 0x05F3;
        public const int ProductIdClassic = 0x042C;
        public const int ProductIdV2 = 0x0438;
        public const int UsagePageConsumer = 0x000C;
        public const int UsageConsumerControl = 0x0001;
        public const double FastMouseMultiplier = 2.5;

        public static bool IsMatchingFootPedal(
            int vendorId,
            int productId,
            int usagePage,
            int usage
        )
        {
            return vendorId == VendorId &&
                (productId == ProductIdClassic || productId == ProductIdV2) &&
                usagePage == UsagePageConsumer &&
                usage == UsageConsumerControl;
        }

        public static bool MiddlePedalPressed(ReadOnlySpan<byte> report)
        {
            return report.Length > 2 && (report[2] & 0x04) != 0;
        }

        public static XKeysIndicatorState IndicatorState(
            bool isEnabled,
            bool didDetectPedal,
            bool isPressed
        )
        {
            if (!isEnabled)
            {
                return XKeysIndicatorState.Disabled;
            }

            if (!didDetectPedal)
            {
                return XKeysIndicatorState.NotDetected;
            }

            return isPressed ? XKeysIndicatorState.Pressed : XKeysIndicatorState.Ready;
        }

        public static XKeysMonitorSnapshot Snapshot(
            bool isEnabled,
            bool didDetectPedal,
            bool isPressed
        )
        {
            return new XKeysMonitorSnapshot(
                IndicatorState: IndicatorState(isEnabled, didDetectPedal, isPressed),
                IsPressed: isEnabled && didDetectPedal && isPressed
            );
        }
    }
}
