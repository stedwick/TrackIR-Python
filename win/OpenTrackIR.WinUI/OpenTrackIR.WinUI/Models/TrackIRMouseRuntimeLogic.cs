namespace OpenTrackIR.WinUI.Models
{
    public readonly record struct RelativeMouseDispatch(
        int DeltaX,
        int DeltaY,
        double RemainingX,
        double RemainingY
    );

    public static class TrackIRMouseRuntimeLogic
    {
        public static double EffectiveMouseSpeed(
            double baseSpeed,
            bool isXKeysFastMouseEnabled,
            bool isXKeysPedalPressed,
            double multiplier = XKeysReportLogic.FastMouseMultiplier
        )
        {
            return isXKeysFastMouseEnabled && isXKeysPedalPressed
                ? baseSpeed * multiplier
                : baseSpeed;
        }

        public static double MouseBackendSpeed(double controlSpeed)
        {
            return Math.Clamp(controlSpeed, 0.1, 5.0) * 10.0;
        }

        public static bool ShouldFireKeepAwake(
            TrackIRControlState controlState,
            TimeSpan timeSinceLastMouseMovement
        )
        {
            return controlState.IsTrackIREnabled &&
                !controlState.IsMouseMovementEnabled &&
                controlState.KeepAwakeSeconds > 0 &&
                timeSinceLastMouseMovement >= TimeSpan.FromSeconds(controlState.KeepAwakeSeconds);
        }

        public static RelativeMouseDispatch ConsumeRelativeDelta(double pendingX, double pendingY)
        {
            int deltaX = (int)Math.Truncate(pendingX);
            int deltaY = (int)Math.Truncate(pendingY);

            return new RelativeMouseDispatch(
                DeltaX: deltaX,
                DeltaY: deltaY,
                RemainingX: pendingX - deltaX,
                RemainingY: pendingY - deltaY
            );
        }
    }
}
