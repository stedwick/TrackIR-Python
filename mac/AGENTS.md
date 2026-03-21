# macOS Agent Notes

The `mac/` directory contains the SwiftUI macOS application shell. Treat it as a native app layer that consumes the shared library and presents desktop UI, not as a second implementation of the TrackIR protocol.

## Current structure

- `OpenTrackIR/OpenTrackIRApp.swift`: app entrypoint and window scene.
- `OpenTrackIR/ContentView.swift`: current top-level SwiftUI view. It now contains the first responsive desktop shell with a native preview panel placeholder and UI-only toggles.
- `OpenTrackIRTests/`: unit tests using Swift Testing (`import Testing`).
- `OpenTrackIRUITests/`: UI tests using `XCTest`.
- `OpenTrackIR.xcodeproj`: Xcode project for the macOS app target.

## macOS-specific rules

- Keep protocol parsing, centroid math, and device transport in the shared C library whenever possible.
- Do not reimplement TrackIR packet logic in SwiftUI views.
- Keep SwiftUI views thin and driven by app state, observable models, or adapter objects.
- If the macOS app needs native integration code for the C library, isolate it in a clearly named bridge or adapter layer.
- Treat `ContentView.swift` as presentation, not business logic.
- Do not depend on OpenCV in the macOS app. Use native Apple image, video, and rendering APIs when the preview is wired up.
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
  - native app presentation and platform integration in `mac/`
- Make the smallest defensible UI change.
- Prefer evolving the blank SwiftUI shell into a focused desktop control/preview app rather than adding speculative architecture up front.
- While the app is still UI-only, prefer clear placeholder states and explicit labels over fake backend behavior.
