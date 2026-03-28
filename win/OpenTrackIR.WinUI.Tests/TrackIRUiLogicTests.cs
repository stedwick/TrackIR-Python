using OpenTrackIR.WinUI.Models;

namespace OpenTrackIR.WinUI.Tests
{
    public sealed class TrackIRUiLogicTests
    {
        [Fact]
        public void CreateDefaultControlState_matches_macos_defaults()
        {
            TrackIRControlState state = TrackIRUiLogic.CreateDefaultControlState();

            Assert.True(state.IsVideoEnabled);
            Assert.True(state.IsTrackIREnabled);
            Assert.True(state.IsMouseMovementEnabled);
            Assert.Equal(2.0, state.MouseMovementSpeed);
            Assert.Equal(3, state.MouseSmoothing);
            Assert.Equal(0.04, state.MouseDeadzone);
            Assert.True(state.IsAvoidMouseJumpsEnabled);
            Assert.Equal(50, state.MouseJumpThresholdPixels);
            Assert.Equal(100, state.MinimumBlobAreaPoints);
            Assert.Equal(29, state.KeepAwakeSeconds);
            Assert.True(state.IsTimeoutEnabled);
            Assert.Equal(28800, state.TimeoutSeconds);
            Assert.Equal(60.0, state.VideoFramesPerSecond);
            Assert.Equal("Shift+F7", state.MouseToggleHotkeyText);
        }

        [Fact]
        public void Normalize_clamps_and_fills_hotkey_defaults()
        {
            TrackIRControlState state = new(
                IsVideoEnabled: true,
                IsTrackIREnabled: true,
                IsMouseMovementEnabled: true,
                MouseMovementSpeed: 9.0,
                IsXKeysFastMouseEnabled: false,
                MouseSmoothing: 99,
                MouseDeadzone: -1.0,
                IsAvoidMouseJumpsEnabled: true,
                MouseJumpThresholdPixels: -4,
                MinimumBlobAreaPoints: 0,
                KeepAwakeSeconds: -8,
                IsTimeoutEnabled: true,
                TimeoutSeconds: 0,
                IsVideoFlipHorizontalEnabled: false,
                IsVideoFlipVerticalEnabled: false,
                VideoRotationDegrees: -90.0,
                VideoFramesPerSecond: 999,
                MouseToggleHotkeyText: "   "
            );

            TrackIRControlState normalized = TrackIRUiLogic.Normalize(state);

            Assert.Equal(5.0, normalized.MouseMovementSpeed);
            Assert.Equal(10, normalized.MouseSmoothing);
            Assert.Equal(0.0, normalized.MouseDeadzone);
            Assert.Equal(1, normalized.MouseJumpThresholdPixels);
            Assert.Equal(1, normalized.MinimumBlobAreaPoints);
            Assert.Equal(0, normalized.KeepAwakeSeconds);
            Assert.Equal(1, normalized.TimeoutSeconds);
            Assert.Equal(270.0, normalized.VideoRotationDegrees);
            Assert.Equal(125.0, normalized.VideoFramesPerSecond);
            Assert.Equal("Shift+F7", normalized.MouseToggleHotkeyText);
        }

        [Fact]
        public void BuildMockSnapshot_switches_to_low_power_when_not_visible()
        {
            TrackIRControlState state = TrackIRUiLogic.CreateDefaultControlState();

            TrackIRSnapshot snapshot = TrackIRUiLogic.BuildMockSnapshot(
                state,
                new TrackIRPresentationState(false, true),
                3
            );

            Assert.Equal(TrackIRRuntimePhase.Streaming, snapshot.Phase);
            Assert.True(snapshot.IsLowPowerMode);
            Assert.False(snapshot.HasPreview);
            Assert.Equal(12.0, snapshot.SourceFrameRate);
            Assert.Equal("Mock Runtime / Low Power", snapshot.BackendLabel);
        }

        [Fact]
        public void PreviewTitle_and_message_match_current_ui_rules()
        {
            TrackIRControlState defaults = TrackIRUiLogic.CreateDefaultControlState();
            TrackIRSnapshot idleSnapshot = TrackIRUiLogic.BuildMockSnapshot(
                defaults with { IsTrackIREnabled = false },
                new TrackIRPresentationState(true, true),
                1
            );

            Assert.Equal("TrackIR Off", TrackIRUiLogic.PreviewTitle(defaults with { IsTrackIREnabled = false }, idleSnapshot));
            Assert.Equal(
                "Turn TrackIR on to start the camera.",
                TrackIRUiLogic.PreviewMessage(defaults with { IsTrackIREnabled = false }, idleSnapshot)
            );
            Assert.Equal(
                "Video Preview Hidden",
                TrackIRUiLogic.PreviewTitle(defaults with { IsVideoEnabled = false }, idleSnapshot)
            );
        }

        [Fact]
        public void Label_helpers_render_expected_strings()
        {
            Assert.Equal("Uncapped", TrackIRUiLogic.TrackIRFramesPerSecondValueLabel(0));
            Assert.Equal("60 / max 75", TrackIRUiLogic.TrackIRRateSummaryLabel(60, 75));
            Assert.Equal("2.5x", TrackIRUiLogic.MouseSpeedValueLabel(2.5));
            Assert.Equal("3", TrackIRUiLogic.MouseSmoothingValueLabel(3.4));
            Assert.Equal("0.04", TrackIRUiLogic.MouseDeadzoneValueLabel(0.04));
            Assert.Equal("270°", TrackIRUiLogic.VideoRotationValueLabel(-90));
            Assert.Equal("No Pedal", TrackIRUiLogic.XKeysIndicatorLabel(XKeysIndicatorState.NotDetected));
            Assert.Equal("#FFB020", TrackIRUiLogic.XKeysIndicatorColorHex(XKeysIndicatorState.NotDetected));
            Assert.Equal("Visible", TrackIRUiLogic.ToggleStateLabel(true, "Visible", "Hidden"));
            Assert.Equal("#7A8797", TrackIRUiLogic.ToggleStateColorHex(false));
        }

        [Fact]
        public void DashboardLayoutForWidth_uses_expected_breakpoints()
        {
            Assert.Equal(DashboardLayoutMode.Narrow, TrackIRUiLogic.DashboardLayoutForWidth(700));
            Assert.Equal(DashboardLayoutMode.Medium, TrackIRUiLogic.DashboardLayoutForWidth(800));
            Assert.Equal(DashboardLayoutMode.Wide, TrackIRUiLogic.DashboardLayoutForWidth(1100));
        }

        [Fact]
        public void HotkeyCaptureLogic_formats_display_text_and_ignores_modifier_only_keys()
        {
            Assert.Equal("Ctrl+Shift+F7", HotkeyCaptureLogic.FormatHotkeyText(true, false, true, false, "F7"));
            Assert.True(HotkeyCaptureLogic.IsModifierKey(16));
            Assert.Equal("A", HotkeyCaptureLogic.KeyTokenForVirtualKey(65));
            Assert.Equal("F7", HotkeyCaptureLogic.KeyTokenForVirtualKey(118));
            Assert.Null(HotkeyCaptureLogic.KeyTokenForVirtualKey(17));
        }

        [Fact]
        public void TrayUiLogic_builds_short_notify_icon_tooltip_text()
        {
            Assert.Equal(
                "OpenTrackIR: TrackIR On, Mouse Off",
                TrayUiLogic.TooltipText(isTrackIREnabled: true, isMouseMovementEnabled: false)
            );
        }
    }
}
