# macOS Agent Notes

The `mac/` directory contains the SwiftUI macOS application. Treat it as a native app layer that consumes the shared C code and presents desktop UI, not as a second implementation of the TrackIR protocol.

## Current structure

- `OpenTrackIR/OpenTrackIRApp.swift`: app entrypoint and window scene.
- `OpenTrackIR/ContentView.swift`: dashboard UI for preview, telemetry, controls, and shortcut configuration.
- `OpenTrackIR/TrackIRRuntimeController.swift`: persisted control state, lifecycle gating, refresh policy, and timeout handling.
- `OpenTrackIR/TrackIRCameraController.swift`: session ownership, polling policy, preview image publishing, and telemetry state.
- `OpenTrackIR/OpenTrackIR-Bridging-Header.h`: Swift bridge to the shared C headers and macOS mouse bridge.
- `OpenTrackIR/TrackIRNativeSources.c`: temporary Xcode-side inclusion of the shared C implementation files.
- `OpenTrackIR/TrackIRMouseBridge.c`: Quartz cursor posting, display-bounds handling, and permission checks.
- `OpenTrackIR/TrackIRMouseBridge.h`: C bridge surface used by Swift and shared mouse helpers.
- `OpenTrackIRTests/`: unit tests using Swift Testing (`import Testing`).
- `OpenTrackIRUITests/`: UI tests using `XCTest`.
- `OpenTrackIR.xcodeproj`: Xcode project for the macOS app target.

## macOS-specific rules

- Keep protocol parsing, centroid math, and device transport in the shared C library whenever possible.
- Keep reusable mouse tracking and smoothing logic in shared C whenever possible.
- Do not reimplement TrackIR packet logic in SwiftUI views.
- Keep SwiftUI views thin and driven by app state, observable models, or adapter objects.
- If the macOS app needs native integration code for the C library, isolate it in a clearly named bridge or adapter layer.
- Treat `ContentView.swift` as presentation, not business logic.
- Do not depend on OpenCV in the macOS app. Use native Apple image, video, and rendering APIs when the preview is wired up.
- Keep cursor posting, accessibility permission checks, and display-bound clamping in the macOS bridge layer, not in shared C.
- The current `TrackIRNativeSources.c` inclusion is a temporary bridge. Keep it minimal and aligned with the shared headers instead of letting it fork behavior.
- Prefer small pure Swift helpers for view formatting and state derivation when needed, with one focused test per new business-rule helper.

## Testing expectations

- Pure UI formatting or state helpers should get Swift unit tests in `OpenTrackIRTests/`.
- UI flow checks belong in `OpenTrackIRUITests/`.
- Hardware and protocol validation should primarily happen through the shared C library and its native harnesses, not only through the macOS app.
- When full Xcode is selected via `xcode-select`, prefer validating the macOS app with `xcodebuild` from the shell so build and test failures are visible without copying errors out of Xcode.
- Default to macOS app builds and unit tests only.
- Do not run scheme-wide macOS test commands that include `OpenTrackIRUITests` unless the user explicitly asks for UI testing.
- Prefer targeted commands such as `xcodebuild build` and `xcodebuild test -only-testing:OpenTrackIRTests` during normal macOS UI work.

## Change strategy

- Preserve the separation between:
  - shared protocol/device logic in `c/`
  - native app presentation, runtime policy, and Quartz integration in `mac/`
- Make the smallest defensible UI change.
- Preserve the existing live preview, telemetry, visibility-aware polling, and hotkey behavior unless the task explicitly changes them.
- Keep transport-migration work aligned with `PLAN-macOS-libusb-to-IOKit.md` so the app does not grow a second copy of device logic.
