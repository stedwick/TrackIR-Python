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

    @Test func trackIRFramePacketFilterMatchesImageBearingPackets() {
        #expect(packetTypeSupportsTrackIRFrame(0x00))
        #expect(packetTypeSupportsTrackIRFrame(0x05))
        #expect(!packetTypeSupportsTrackIRFrame(0x03))
    }

    @Test func trackIRFrameRateUsesFramesPerElapsedSecond() {
        #expect(trackIRFrameRate(frameCount: 30, elapsedSeconds: 0.25) == 120.0)
        #expect(trackIRFrameRate(frameCount: 0, elapsedSeconds: 0.25) == nil)
        #expect(trackIRFrameRate(frameCount: 30, elapsedSeconds: 0) == nil)
    }

    @Test func trackIRFrameRateLabelUsesCompactFpsText() {
        #expect(trackIRFrameRateLabel(for: 123.44) == "123.4 fps")
        #expect(trackIRFrameRateLabel(for: nil) == "-")
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
