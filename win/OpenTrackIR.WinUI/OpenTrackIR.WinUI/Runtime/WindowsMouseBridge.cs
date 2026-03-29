using System.Runtime.InteropServices;
using OpenTrackIR.WinUI.Models;

namespace OpenTrackIR.WinUI.Runtime
{
    internal sealed class WindowsMouseBridge
    {
        private const uint InputMouse = 0;
        private const uint MouseEventMove = 0x0001;
        private static readonly int InputSize = Marshal.SizeOf<Input>();

        private TrackIRNativeMethods.NativeTrackIRMouseTrackerState _trackerState;
        private readonly Input[] _sendInputBuffer = new Input[1];
        private double _pendingDeltaX;
        private double _pendingDeltaY;
        private int _keepAwakeDirection = 1;

        public WindowsMouseBridge()
        {
            _trackerState = CreateTrackerState();
            Reset();
        }

        public void Reset()
        {
            TrackIRNativeMethods.TrackIRMouseTrackerReset(ref _trackerState);
            _pendingDeltaX = 0.0;
            _pendingDeltaY = 0.0;
            _keepAwakeDirection = 1;
        }

        public bool TryApplyTrackingDelta(
            bool hasCentroid,
            double centroidX,
            double centroidY,
            TrackIRControlState controlState,
            double effectiveMouseSpeed
        )
        {
            TrackIRNativeMethods.NativeTrackIRMouseStep mouseStep =
                TrackIRNativeMethods.TrackIRMouseTrackerUpdate(
                    ref _trackerState,
                    hasCentroid,
                    new TrackIRNativeMethods.NativeTrackIRMousePoint
                    {
                        X = centroidX,
                        Y = centroidY,
                    },
                    CreateTrackerConfig(controlState, effectiveMouseSpeed)
                );

            if (!mouseStep.HasCursorDelta)
            {
                return false;
            }

            _pendingDeltaX += mouseStep.CursorDelta.X;
            _pendingDeltaY += mouseStep.CursorDelta.Y;
            RelativeMouseDispatch dispatch = TrackIRMouseRuntimeLogic.ConsumeRelativeDelta(
                _pendingDeltaX,
                _pendingDeltaY
            );
            _pendingDeltaX = dispatch.RemainingX;
            _pendingDeltaY = dispatch.RemainingY;

            if (dispatch.DeltaX == 0 && dispatch.DeltaY == 0)
            {
                return false;
            }

            return SendRelativeMouseInput(dispatch.DeltaX, dispatch.DeltaY);
        }

        public bool TryNudge()
        {
            int direction = _keepAwakeDirection == 0 ? 1 : _keepAwakeDirection;
            _keepAwakeDirection = -direction;
            return SendRelativeMouseInput(direction, 0);
        }

        private static TrackIRNativeMethods.NativeTrackIRMouseTrackerState CreateTrackerState()
        {
            return new TrackIRNativeMethods.NativeTrackIRMouseTrackerState
            {
                DeltaHistory = new TrackIRNativeMethods.NativeTrackIRMousePoint[
                    TrackIRNativeMethods.MaxSmoothingWindow
                ],
            };
        }

        private static TrackIRNativeMethods.NativeTrackIRMouseTrackerConfig CreateTrackerConfig(
            TrackIRControlState controlState,
            double effectiveMouseSpeed
        )
        {
            return new TrackIRNativeMethods.NativeTrackIRMouseTrackerConfig
            {
                IsMovementEnabled = controlState.IsMouseMovementEnabled,
                Speed = TrackIRMouseRuntimeLogic.MouseBackendSpeed(effectiveMouseSpeed),
                Smoothing = controlState.MouseSmoothing,
                Deadzone = controlState.MouseDeadzone,
                AvoidMouseJumps = controlState.IsAvoidMouseJumpsEnabled,
                JumpThresholdPixels = controlState.MouseJumpThresholdPixels,
                Transform = new TrackIRNativeMethods.NativeTrackIRMouseTransform
                {
                    ScaleX = TrackIRUiLogic.PreviewAxisScale(controlState.IsVideoFlipHorizontalEnabled),
                    ScaleY = TrackIRUiLogic.PreviewAxisScale(controlState.IsVideoFlipVerticalEnabled),
                    RotationDegrees = TrackIRUiLogic.NormalizeRotationDegrees(
                        controlState.VideoRotationDegrees
                    ),
                },
            };
        }

        private bool SendRelativeMouseInput(int deltaX, int deltaY)
        {
            _sendInputBuffer[0] =
                new Input
                {
                    Type = InputMouse,
                    Data = new InputUnion
                    {
                        MouseInput = new MouseInput
                        {
                            DeltaX = deltaX,
                            DeltaY = deltaY,
                            MouseData = 0,
                            Flags = MouseEventMove,
                            Time = 0,
                            ExtraInfo = nint.Zero,
                        },
                    },
                };

            return SendInput((uint)_sendInputBuffer.Length, _sendInputBuffer, InputSize) ==
                _sendInputBuffer.Length;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct Input
        {
            public uint Type;
            public InputUnion Data;
        }

        [StructLayout(LayoutKind.Explicit)]
        private struct InputUnion
        {
            [FieldOffset(0)]
            public MouseInput MouseInput;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct MouseInput
        {
            public int DeltaX;
            public int DeltaY;
            public uint MouseData;
            public uint Flags;
            public uint Time;
            public nint ExtraInfo;
        }

        [DllImport("user32.dll", SetLastError = true)]
        private static extern uint SendInput(uint inputCount, [In] Input[] inputs, int inputSize);
    }
}
