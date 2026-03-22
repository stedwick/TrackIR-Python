# OpenTrackIR Agent Notes

This repository is for reverse-engineering NaturalPoint TrackIR hardware so it can run without the proprietary SDK and eventually support multiple operating systems through open implementations.

## Primary objective

When making changes here, optimize for protocol clarity and cross-platform portability first. The code should help answer questions like:

- How does the device initialize?
- What packet formats are present on the wire?
- Which behaviors are hardware-specific versus host-side assumptions?
- What can be expressed as reusable logic across Python, C, C++, and platform adapters?

## Repo expectations

- Make the smallest defensible change.
- Reuse existing helpers before adding new abstractions.
- Put business logic in pure functions whenever practical.
- Add one unit test for each logical piece of business logic you introduce.
- Do not hide protocol errors behind broad exception handling.
- Do not revert unrelated work from other contributors or agents.

## Documentation standard

- Capture reverse-engineering findings in code comments or docs only when they add concrete value.
- Prefer naming that reflects observed protocol behavior instead of speculative intent.
- If a constant or packet field is not fully understood, say so directly.
- Keep logs, dumps, and helper names aligned so a packet observed in one tool can be traced in another.

## Cross-platform standard

- Avoid baking OS-specific assumptions into shared protocol code.
- Keep hardware communication, parsing, and visualization separated.
- If platform-specific behavior is required, isolate it in the platform directory or a clearly named adapter layer.
- Keep OpenCV, windowing, and app event-loop concerns out of shared C protocol code.
- Keep reusable mouse-tracker math and smoothing in shared C; keep OS event posting and permissions in platform bridges.
- Keep the macOS app on native Apple UI/media APIs rather than introducing OpenCV into `mac/`.

## Validation

- For large changes, run the relevant linter, build, and tests.
- For small doc-only changes, no extra validation is required.
- For the macOS app, prefer `xcodebuild` from the shell when full Xcode is selected so Swift build/test failures can be checked directly.
- For the macOS app, default to build plus unit-test coverage only.
- Do not run scheme-wide macOS test commands that include `OpenTrackIRUITests` unless the user explicitly asks for UI testing.
- Prefer targeted macOS validation commands such as `xcodebuild build` and `xcodebuild test -only-testing:OpenTrackIRTests` so the app is not repeatedly relaunched during normal work.

## Release process

- Start releases from a clean `main` worktree that is already pushed to `origin/main`.
- Follow the existing versioning pattern: Git tag `v0.2`, GitHub release title `0.2`, and asset name `OpenTrackIR-0.2-macOS-26-Tahoe-Apple-Silicon.zip`.
- For the macOS app, prioritize Developer ID code signing and notarization over Git tag signing; the goal is a build that launches without Gatekeeper malware warnings on another Mac.
- The Xcode project is already configured with `Developer ID Application: Philip Brocoum (WP9BPQYC6P)`. Preserve that unless the signing identity changes intentionally.
- Bump `MARKETING_VERSION` and `CURRENT_PROJECT_VERSION` in `mac/OpenTrackIR.xcodeproj/project.pbxproj` before cutting the release so the app bundle version matches the GitHub release.
- A plain Release build from Xcode produced notarization failures for `0.2` because the signature was missing a secure timestamp and the binary still carried `com.apple.security.get-task-allow`.
- The successful release build used `xcodebuild -project mac/OpenTrackIR.xcodeproj -scheme OpenTrackIR -destination 'platform=macOS' -configuration Release -derivedDataPath /Users/philip/src/OpenTrackIR/tmp/DerivedData-0.2-release CODE_SIGN_INJECT_BASE_ENTITLEMENTS=NO OTHER_CODE_SIGN_FLAGS=--timestamp build`.
- Validate the signed app before notarization with `codesign -dv --verbose=4` and `codesign --verify --deep --strict`; the release app should show a Developer ID signature with a secure `Timestamp=` and no embedded entitlements.
- Use the existing `OpenTrackIRNotary` keychain profile with `xcrun notarytool submit ... --keychain-profile OpenTrackIRNotary --wait` to notarize a zip that contains the signed `.app`.
- After Apple accepts the submission, staple the ticket to the `.app` with `xcrun stapler staple`, then rebuild the final GitHub zip from the stapled app so the distributed asset already contains the notarization ticket.
- Verify the stapled app with `spctl -a -vv`; the accepted result should say `source=Notarized Developer ID`.
- Check the previous release metadata with `gh release view v0.1` before drafting notes or naming assets so naming stays consistent.
- Publish the release with `gh release create`, attach the notarized zip, and keep the release notes explicit about platform/build assumptions such as the macOS version, architecture, notarization status, and whether `libusb` is statically linked.

## Directory guide

- `python/`: fastest place to iterate on USB transport, logging, decoding, and tests.
- `c/`: reusable C library for protocol logic, frame helpers, and device transport.
- `cpp/`: native consumers of the C library, including the OpenCV preview harness.
- `mac/`: SwiftUI macOS app, Quartz mouse bridge, and temporary Xcode-side bridge to the shared C sources.
- `win/`, `nix/`: platform-specific integration layers and notes.

## Native build layout

- Keep the root `CMakeLists.txt` as the native build entrypoint.
- Keep native targets split into `c/CMakeLists.txt` and `cpp/CMakeLists.txt`.
- Prefer the top-level `build/` tree for documented CMake commands unless a subproject has a stronger reason to diverge.
- Treat `libusb-1.0` as required for the native C hardware library.
- Treat OpenCV as optional and scoped only to the C++ preview harness.

## Native direction

- Treat `python/` as the reference workbench for rapid reverse-engineering and behavior discovery.
- Move stable protocol behavior into `c/` once it is understood and testable.
- Keep `c/` responsible for protocol parsing, centroid math, frame reconstruction, session orchestration, shared mouse-tracker logic, and transport.
- Keep `cpp/` responsible for consuming the C API and validating it in native desktop flows.
- Keep `mac/` responsible for native app presentation, preview/image conversion, app lifecycle, and platform event bridges on top of the shared C layer.
- Keep `mac/OpenTrackIR/TrackIRNativeSources.c` thin until the planned transport split removes the temporary source-inclusion bridge.
- If protocol logic appears in C++ app code, that is usually a sign it belongs back in `c/`.

@README.md
