//
//  OpenTrackIRApp.swift
//  OpenTrackIR
//
//  Created by Philip Brocoum on 3/21/26.
//

import KeyboardShortcuts
import AppKit
import SwiftUI

@main
struct OpenTrackIRApp: App {
    @NSApplicationDelegateAdaptor(AppLifecycleController.self) private var appLifecycleController
    @StateObject private var runtimeController = TrackIRRuntimeController()
    @State private var mouseMovementHotkeyController = MouseMovementHotkeyController()

    init() {
        UserDefaults.standard.register(defaults: controlDefaultPreferences(controlDefaultValues()))
    }

    var body: some Scene {
        WindowGroup {
            ContentView(runtimeController: runtimeController)
                .onAppear {
                    appLifecycleController.runtimeController = runtimeController
                    mouseMovementHotkeyController.runtimeController = runtimeController
                }
        }
    }
}

@MainActor
final class AppLifecycleController: NSObject, NSApplicationDelegate {
    weak var runtimeController: TrackIRRuntimeController?

    func applicationWillTerminate(_ notification: Notification) {
        if shouldShutdownTrackIRRuntime(for: .appWillTerminate) {
            runtimeController?.shutdownAndWait()
        }
    }
}

@MainActor
final class MouseMovementHotkeyController {
    weak var runtimeController: TrackIRRuntimeController?

    init() {
        KeyboardShortcuts.removeHandler(for: .toggleMouseMovement)
        KeyboardShortcuts.onKeyUp(for: .toggleMouseMovement) { [weak self] in
            self?.runtimeController?.toggleMouseMovement()
        }
    }
}
