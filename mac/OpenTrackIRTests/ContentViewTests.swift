//
//  ContentViewTests.swift
//  OpenTrackIRTests
//

import AppKit
import CoreGraphics
import KeyboardShortcuts
import Testing
@testable import OpenTrackIR

struct ContentViewTests {

    @Test func dashboardLayoutUsesTwoColumnsAtDesktopWidths() {
        #expect(dashboardLayout(for: 959) == .stacked)
        #expect(dashboardLayout(for: 960) == .twoColumn)
        #expect(dashboardLayout(for: 1_280) == .twoColumn)
    }

    @Test func videoPreviewWidthShrinksToCompactShellSizes() {
        #expect(videoPreviewWidth(for: .twoColumn, width: 1_280) == 307.2)
        #expect(videoPreviewWidth(for: .twoColumn, width: 2_000) == 320)
        #expect(videoPreviewWidth(for: .stacked, width: 760) == 288.8)
    }

    @Test func videoPreviewHeightPreservesFourByThreeAspectRatio() {
        #expect(videoPreviewHeight(for: 320) == 240)
        #expect(abs(videoPreviewHeight(for: 288.8) - 216.6) < 0.0001)
    }

    @Test func previewTelemetryWidthCanExceedVideoWidth() {
        #expect(previewTelemetryWidth(for: 288.8) == 620)
        #expect(previewTelemetryWidth(for: 700) == 700)
    }

    @Test func controlColumnCountUsesTwoColumnsOnSmallerWindows() {
        #expect(controlColumnCount(for: .stacked, width: 719) == 1)
        #expect(controlColumnCount(for: .stacked, width: 720) == 2)
        #expect(controlColumnCount(for: .twoColumn, width: 899) == 1)
        #expect(controlColumnCount(for: .twoColumn, width: 900) == 2)
    }

    @Test func controlPreferenceKeysRemainStable() {
        #expect(ControlPreferenceKey.videoEnabled.rawValue == "contentView.videoEnabled")
        #expect(ControlPreferenceKey.trackIREnabled.rawValue == "contentView.trackIREnabled")
        #expect(ControlPreferenceKey.mouseMovementEnabled.rawValue == "contentView.mouseMovementEnabled")
        #expect(ControlPreferenceKey.mouseMovementSpeed.rawValue == "contentView.mouseMovementSpeed")
        #expect(ControlPreferenceKey.videoFlipHorizontal.rawValue == "contentView.videoFlipHorizontal")
        #expect(ControlPreferenceKey.videoFlipVertical.rawValue == "contentView.videoFlipVertical")
        #expect(ControlPreferenceKey.videoRotationDegrees.rawValue == "contentView.videoRotationDegrees")
        #expect(ControlPreferenceKey.videoFramesPerSecond.rawValue == "contentView.videoFramesPerSecond")
    }

    @Test func controlDefaultValuesMatchExpectedStartupState() {
        #expect(controlDefaultValues() == ControlDefaultValues(
            videoEnabled: false,
            trackIREnabled: false,
            mouseMovementEnabled: false,
            mouseMovementSpeed: 1.0,
            videoFlipHorizontalEnabled: true,
            videoFlipVerticalEnabled: false,
            videoRotationDegrees: 0.0,
            videoFramesPerSecond: 60.0
        ))
    }

    @Test func defaultMouseMovementShortcutUsesShiftF7() {
        #expect(defaultMouseMovementShortcut() == KeyboardShortcuts.Shortcut(.f7, modifiers: [.shift]))
        #expect(KeyboardShortcuts.Name.toggleMouseMovement.rawValue == "toggleMouseMovement")
        #expect(KeyboardShortcuts.Name.toggleMouseMovement.defaultShortcut == defaultMouseMovementShortcut())
    }

    @Test func toggledMouseMovementStateFlipsTheStoredValue() {
        #expect(toggledMouseMovementState(isEnabled: true) == false)
        #expect(toggledMouseMovementState(isEnabled: false) == true)
    }

    @Test func mouseSpeedValueLabelUsesCompactMultiplierText() {
        #expect(mouseSpeedValueLabel(for: 1.0) == "1x")
        #expect(mouseSpeedValueLabel(for: 1.25) == "1.25x")
        #expect(mouseSpeedValueLabel(for: 0.5) == "0.5x")
    }

    @Test func previewVideoTransformMapsFlipsAndRotation() {
        #expect(previewVideoTransform(
            flipHorizontal: true,
            flipVertical: false,
            rotationDegrees: 450
        ) == VideoPreviewTransform(scaleX: -1, scaleY: 1, rotationDegrees: 90))
        #expect(previewVideoTransform(
            flipHorizontal: false,
            flipVertical: true,
            rotationDegrees: -90
        ) == VideoPreviewTransform(scaleX: 1, scaleY: -1, rotationDegrees: 270))
    }

    @Test func videoRotationValueLabelUsesNormalizedDegrees() {
        #expect(videoRotationValueLabel(for: 0) == "0°")
        #expect(videoRotationValueLabel(for: 360) == "0°")
        #expect(videoRotationValueLabel(for: 450) == "90°")
    }

    @Test func trackIRFramesPerSecondValueLabelHandlesUncappedAndWholeNumbers() {
        #expect(trackIRFramesPerSecondValueLabel(for: 0) == "Uncapped")
        #expect(trackIRFramesPerSecondValueLabel(for: 60) == "60 fps")
        #expect(trackIRFramesPerSecondValueLabel(for: 124.6) == "125 fps")
    }

    @Test func trackIRRateSummaryLabelCombinesCapAndSourceRate() {
        #expect(trackIRRateSummaryLabel(maximumFramesPerSecond: 60, sourceFrameRate: 122.4) == "60 / max 122")
        #expect(trackIRRateSummaryLabel(maximumFramesPerSecond: 0, sourceFrameRate: 122.6) == "Uncapped / max 123")
        #expect(trackIRRateSummaryLabel(maximumFramesPerSecond: 30, sourceFrameRate: nil) == "30 / max -")
    }

    @Test func trackIRStreamingDependsOnTrackIREnableOnly() {
        #expect(shouldStreamTrackIRSession(isTrackIREnabled: true, isVideoEnabled: true))
        #expect(shouldStreamTrackIRSession(isTrackIREnabled: true, isVideoEnabled: false))
        #expect(!shouldStreamTrackIRSession(isTrackIREnabled: false, isVideoEnabled: true))
    }

    @Test func xcodePreviewEnvironmentDetectionMatchesKnownFlags() {
        #expect(isRunningInXcodePreview(environment: ["XCODE_RUNNING_FOR_PREVIEWS": "1"]))
        #expect(isRunningInXcodePreview(environment: ["XCODE_RUNNING_FOR_PLAYGROUNDS": "1"]))
        #expect(!isRunningInXcodePreview(environment: [:]))
    }

    @Test func trackIRHardwareAccessSkipsPreviewRuns() {
        #expect(shouldAccessTrackIRHardware(
            isTrackIREnabled: true,
            isVideoEnabled: true,
            environment: [:]
        ))
        #expect(!shouldAccessTrackIRHardware(
            isTrackIREnabled: true,
            isVideoEnabled: true,
            environment: ["XCODE_RUNNING_FOR_PREVIEWS": "1"]
        ))
        #expect(!shouldAccessTrackIRHardware(
            isTrackIREnabled: false,
            isVideoEnabled: true,
            environment: [:]
        ))
        #expect(shouldAccessTrackIRHardware(
            isTrackIREnabled: true,
            isVideoEnabled: false,
            environment: [:]
        ))
    }

    @Test func trackIRUIUpdateIntervalUsesThrottledPolling() {
        #expect(abs(trackIRUIUpdateInterval(isVideoEnabled: true) - (1.0 / 30.0)) < 0.0001)
        #expect(trackIRUIUpdateInterval(isVideoEnabled: false) == 0.2)
    }

    @Test func trackIRCameraPhaseMapsNativeSessionStates() {
        #expect(trackIRCameraPhase(sessionPhase: OTIR_TRACKIR_SESSION_PHASE_IDLE) == .idle)
        #expect(trackIRCameraPhase(sessionPhase: OTIR_TRACKIR_SESSION_PHASE_STARTING) == .starting)
        #expect(trackIRCameraPhase(sessionPhase: OTIR_TRACKIR_SESSION_PHASE_STREAMING) == .streaming)
        #expect(trackIRCameraPhase(sessionPhase: OTIR_TRACKIR_SESSION_PHASE_UNAVAILABLE) == .unavailable)
        #expect(trackIRCameraPhase(sessionPhase: OTIR_TRACKIR_SESSION_PHASE_FAILED) == .failed)
    }

    @Test func trackIRFrameRateLabelUsesCompactFpsText() {
        #expect(trackIRFrameRateLabel(for: 123.44) == "123.4 fps")
        #expect(trackIRFrameRateLabel(for: nil) == "-")
    }

    @Test func trackIRCoordinateLabelTruncatesToIntegers() {
        #expect(trackIRCoordinateLabel(for: 123.9) == "123")
        #expect(trackIRCoordinateLabel(for: -45.8) == "-45")
        #expect(trackIRCoordinateLabel(for: nil) == "-")
    }

    @Test func trackIRCoordinatePairLabelUsesCommaSeparatedIntegers() {
        #expect(trackIRCoordinatePairLabel(x: 123.9, y: 45.2) == "123, 45")
        #expect(trackIRCoordinatePairLabel(x: nil, y: 45.2) == "-")
        #expect(trackIRCoordinatePairLabel(x: 123.9, y: nil) == "-")
    }

    @Test func trackIRSessionErrorDescriptionReadsUtf8Message() {
        var snapshot = otir_trackir_session_snapshot()
        let message = Array("TrackIR busy".utf8)

        snapshot.has_error_message = true
        withUnsafeMutableBytes(of: &snapshot.error_message) { buffer in
            _ = buffer.copyBytes(from: message)
        }

        #expect(trackIRSessionErrorDescription(snapshot: snapshot) == "TrackIR busy")
    }

    @Test func trackIRPreviewImageUsesProvidedDimensions() {
        let image = trackIRPreviewImage(frameBytes: [0, 64, 128, 255], width: 2, height: 2)

        #expect(image?.width == 2)
        #expect(image?.height == 2)
    }

    @Test func notFoundFailureMessageStaysUserFacing() {
        #expect(trackIRFailureMessage(for: OTIR_STATUS_NOT_FOUND, operation: "Open") == "TrackIR not found. Connect the device and try again.")
    }

    @Test func openIoFailureMessageSuggestsRefreshWhenDeviceIsBusy() {
        #expect(trackIRFailureMessage(for: OTIR_STATUS_IO, operation: "Open") == "Open failed: io. TrackIR may be busy in another app. Quit other TrackIR tools, then Refresh.")
    }
}
