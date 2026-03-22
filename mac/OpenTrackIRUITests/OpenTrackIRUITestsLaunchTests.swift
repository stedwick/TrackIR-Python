//
//  OpenTrackIRUITestsLaunchTests.swift
//  OpenTrackIRUITests
//
//  Created by Philip Brocoum on 3/21/26.
//

import XCTest

final class OpenTrackIRUITestsLaunchTests: XCTestCase {
    private let disableHardwareEnvironment = ["OTIR_DISABLE_TRACKIR_HARDWARE": "1"]

    override func setUpWithError() throws {
        continueAfterFailure = false
    }

    @MainActor
    func testLaunch() throws {
        let app = XCUIApplication()
        app.launchEnvironment.merge(disableHardwareEnvironment) { _, new in new }
        app.launch()
    }
}
