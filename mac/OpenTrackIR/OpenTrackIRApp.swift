//
//  OpenTrackIRApp.swift
//  OpenTrackIR
//
//  Created by Philip Brocoum on 3/21/26.
//

import KeyboardShortcuts
import SwiftUI

@main
struct OpenTrackIRApp: App {
    @State private var mouseMovementHotkeyController = MouseMovementHotkeyController()

    var body: some Scene {
        WindowGroup {
            ContentView()
        }
    }
}

@MainActor
final class MouseMovementHotkeyController {
    private let userDefaults: UserDefaults

    init(userDefaults: UserDefaults = .standard) {
        self.userDefaults = userDefaults

        KeyboardShortcuts.removeHandler(for: .toggleMouseMovement)
        KeyboardShortcuts.onKeyUp(for: .toggleMouseMovement) { [userDefaults] in
            let isMouseMovementEnabled = userDefaults.bool(forKey: ControlPreferenceKey.mouseMovementEnabled.rawValue)
            userDefaults.set(
                toggledMouseMovementState(isEnabled: isMouseMovementEnabled),
                forKey: ControlPreferenceKey.mouseMovementEnabled.rawValue
            )
        }
    }
}
