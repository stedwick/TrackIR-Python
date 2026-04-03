namespace OpenTrackIR.WinUI.Models
{
    public readonly record struct KeepAwakeNudge(int DeltaX, int DeltaY);
    public readonly record struct AbsoluteCursorTarget(int X, int Y);

    public readonly record struct RelativeMouseDispatch(
        int DeltaX,
        int DeltaY,
        double RemainingX,
        double RemainingY
    );

    public static class TrackIRMouseRuntimeLogic
    {
        public const int KeepAwakeDirectionCount = 4;
        public const int KeepAwakeNudgePixels = 5;

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
            return Math.Clamp(controlSpeed, 0.1, 20.0) * 10.0;
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

        public static KeepAwakeNudge KeepAwakeNudgeForIndex(int directionIndex)
        {
            return Math.Abs(directionIndex % KeepAwakeDirectionCount) switch
            {
                0 => new KeepAwakeNudge(KeepAwakeNudgePixels, 0),
                1 => new KeepAwakeNudge(-KeepAwakeNudgePixels, 0),
                2 => new KeepAwakeNudge(0, KeepAwakeNudgePixels),
                _ => new KeepAwakeNudge(0, -KeepAwakeNudgePixels),
            };
        }

        public static AbsoluteCursorTarget AbsoluteCursorTargetForDelta(
            int currentCursorX,
            int currentCursorY,
            int deltaX,
            int deltaY
        )
        {
            return new AbsoluteCursorTarget(
                X: currentCursorX + deltaX,
                Y: currentCursorY + deltaY
            );
        }
    }
}
