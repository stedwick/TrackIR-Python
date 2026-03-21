# OpenTrackIR

OpenTrackIR is a reverse-engineering workspace for NaturalPoint TrackIR hardware, with the explicit goal of removing the dependency on NaturalPoint's proprietary SDK and making device support work cross-platform.

The repo is organized by implementation target:

- `python/`: active protocol exploration, USB transport work, packet decoding, logging, and preview tooling.
- `c/`: reusable cross-platform C library for TrackIR protocol, frame reconstruction, and device control.
- `cpp/`: native C++ consumers and harnesses for the C library, including the OpenCV preview app.
- `mac/`: SwiftUI macOS app shell and future native Apple-platform integration.
- `win/`, `nix/`: platform-specific notes, adapters, or future integration work.
- `tmp/`: scratch output and temporary artifacts.

## Project goals

- Identify and document the TrackIR device protocol.
- Reproduce initialization, streaming, and shutdown behavior without vendor SDKs.
- Build portable code paths that can be shared across macOS, Linux, and Windows.
- Keep the reverse-engineered behavior testable with small, isolated units.

## Current Python work

The Python implementation currently contains:

- USB device discovery and transport helpers for the TrackIR 5 v3 hardware path.
- Packet extraction and decoding helpers for the sensor stream.
- A small CLI for identification, packet dumping, and preview rendering.
- Unit tests around transport encoding, packet recovery, stripe decoding, centroid math, and shutdown sequencing.

The Python workbench remains the fastest place to validate protocol ideas before porting stable behavior into the native library.

Useful Python CLI paths:

```sh
cd python
uv run python trackir_tir5v3.py opencv --log tmp/logs/opencv-manual.log
uv run python trackir_tir5v3.py log --log tmp/logs/log-manual.log
```

- `opencv`: live OpenCV preview window with centroid overlay.
- `log`: no OpenCV window; prints `x` and `y` once per second to the terminal.

## Working principles

- Prefer small, reversible changes over broad rewrites.
- Keep protocol knowledge in pure functions where possible.
- Add a focused unit test for each meaningful piece of business logic.
- Preserve raw observations in logs so assumptions can be checked against device behavior later.
- Avoid introducing dependencies on vendor SDKs, closed headers, or platform-locked assumptions.

## Status

This repository is an active reverse-engineering project, not a finished end-user product. Expect experimental code, incomplete platform parity, and evolving protocol understanding.

## Native Workbench

The native port now lives in:

- `CMakeLists.txt`: top-level native build entrypoint that wires the C and C++ subprojects together.
- `c/CMakeLists.txt`: C library, C tests, and C streaming harness targets.
- `cpp/CMakeLists.txt`: C++ consumer targets, including the OpenCV preview app.
- `c/include/opentrackir/tir5.h`: public C API for protocol helpers, frame reconstruction, and device control.
- `c/src/`: protocol/frame implementation plus the `libusb` device backend.
- `c/examples/stream_dump.c`: simple C-only stream dumper that prints frame, packet, and centroid data.
- `c/tests/test_tir5.c`: unit tests for the pure parsing and centroid/frame logic.
- `cpp/opencv_preview/main.cpp`: simple C++ OpenCV preview app that consumes the C API and serves as the first native hardware test harness.
- `mac/OpenTrackIR/ContentView.swift`: SwiftUI macOS shell with a responsive preview panel placeholder and UI-only controls for future TrackIR integration.

The intended native split is:

- The C library owns protocol parsing, centroid math, frame reconstruction, and hardware transport.
- The C++ app owns preview rendering and native test-harness concerns.
- The macOS app owns native Apple UI and desktop integration on top of the shared library.
- OpenCV stays out of the C library and out of the macOS app.

The intended build flow is:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

This is one native project with C and C++ subprojects under a shared top-level build tree.

## Native dependencies

- `libusb-1.0` is required for the native C library and device layer.
- OpenCV is required only for the C++ preview app.
- CMake is the supported native build entrypoint.

## macOS app status

The macOS project is currently a UI-only SwiftUI shell. It includes:

- a native preview panel placeholder for the future camera feed
- a toggle to show or hide video
- a toggle to enable or disable TrackIR
- a toggle to enable or disable future mouse movement

It does not yet talk to the C library, move the mouse, or stream real video frames. When that integration work starts, use native Apple image/video APIs in the macOS app rather than OpenCV.

## macOS app build and run

Open the app in Xcode:

```sh
open mac/OpenTrackIR.xcodeproj
```

Then select the `OpenTrackIR` scheme and press Run.

To build from the terminal:

```sh
cd /Users/philip/src/OpenTrackIR
xcodebuild -project mac/OpenTrackIR.xcodeproj -scheme OpenTrackIR -destination 'platform=macOS' build
```

To run the built app from Finder, use Xcode's Product > Show Build Folder, then open `OpenTrackIR.app`.

If you want to launch it from the terminal after building:

```sh
open ~/Library/Developer/Xcode/DerivedData/OpenTrackIR-*/Build/Products/Debug/OpenTrackIR.app
```

If `libusb-1.0` and OpenCV are available, the preview target is built alongside the C library and tests.

## Native run commands

Run the C unit tests:

```sh
cd /Users/philip/src/OpenTrackIR
cmake -S . -B build
cmake --build build --target test_tir5
./build/c/test_tir5
```

Run the C text streaming harness:

```sh
cd /Users/philip/src/OpenTrackIR
cmake -S . -B build
cmake --build build --target opentrackir_stream_dump
./build/c/opentrackir_stream_dump
```

Run the C++ OpenCV preview harness:

```sh
cd /Users/philip/src/OpenTrackIR
cmake -S . -B build
cmake --build build --target opentrackir_preview
./build/cpp/opentrackir_preview
```
