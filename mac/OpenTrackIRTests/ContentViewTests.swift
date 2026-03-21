//
//  ContentViewTests.swift
//  OpenTrackIRTests
//

import CoreGraphics
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
}
