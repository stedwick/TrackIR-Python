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
}
