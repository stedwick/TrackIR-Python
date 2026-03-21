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
- Keep the macOS app on native Apple UI/media APIs rather than introducing OpenCV into `mac/`.

## Validation

- For large changes, run the relevant linter, build, and tests.
- For small doc-only changes, no extra validation is required.
- For the macOS app, prefer `xcodebuild` from the shell when full Xcode is selected so Swift build/test failures can be checked directly.
- For the macOS app, default to build plus unit-test coverage only.
- Do not run scheme-wide macOS test commands that include `OpenTrackIRUITests` unless the user explicitly asks for UI testing.
- Prefer targeted macOS validation commands such as `xcodebuild build` and `xcodebuild test -only-testing:OpenTrackIRTests` so the app is not repeatedly relaunched during normal work.

## Directory guide

- `python/`: fastest place to iterate on USB transport, logging, decoding, and tests.
- `c/`: reusable C library for protocol logic, frame helpers, and device transport.
- `cpp/`: native consumers of the C library, including the OpenCV preview harness.
- `mac/`: SwiftUI macOS app shell and future native platform integration for the shared library.
- `mac/`, `win/`, `nix/`: platform-specific integration layers and notes.

## Native build layout

- Keep the root `CMakeLists.txt` as the native build entrypoint.
- Keep native targets split into `c/CMakeLists.txt` and `cpp/CMakeLists.txt`.
- Prefer native build directories under `c/` or `cpp/`, with `c/build` as the default documented location.
- Treat `libusb-1.0` as required for the native C hardware library.
- Treat OpenCV as optional and scoped only to the C++ preview harness.

## Native direction

- Treat `python/` as the reference workbench for rapid reverse-engineering and behavior discovery.
- Move stable protocol behavior into `c/` once it is understood and testable.
- Keep `c/` responsible for protocol parsing, centroid math, frame reconstruction, and transport.
- Keep `cpp/` responsible for consuming the C API and validating it in native desktop flows.
- Keep `mac/` responsible for native app presentation and Apple-platform integration on top of the shared C layer.
- If protocol logic appears in C++ app code, that is usually a sign it belongs back in `c/`.

@README.md
