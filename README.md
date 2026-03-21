# OpenTrackIR

OpenTrackIR is a reverse-engineering workspace for NaturalPoint TrackIR hardware, with the explicit goal of removing the dependency on NaturalPoint's proprietary SDK and making device support work cross-platform.

The repo is organized by implementation target:

- `python/`: active protocol exploration, USB transport work, packet decoding, logging, and preview tooling.
- `c/`: reusable cross-platform C library for TrackIR protocol, frame reconstruction, and device control.
- `cpp/`: native C++ consumers and harnesses for the C library, including the OpenCV preview app.
- `mac/`, `win/`, `nix/`: platform-specific notes, adapters, or future integration work.
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

The intended native split is:

- The C library owns protocol parsing, centroid math, frame reconstruction, and hardware transport.
- The C++ app owns preview rendering and native test-harness concerns.
- OpenCV stays out of the C library.

The intended build flow is:

```sh
cmake -S . -B c/build
cmake --build c/build
ctest --test-dir c/build
```

## Native dependencies

- `libusb-1.0` is required for the native C library and device layer.
- OpenCV is required only for the C++ preview app.
- CMake is the supported native build entrypoint.

If `libusb-1.0` and OpenCV are available, the preview target is built alongside the C library and tests.

## Native run commands

Run the C unit tests:

```sh
cd /Users/philip/src/OpenTrackIR
cmake -S . -B c/build
cmake --build c/build --target test_tir5
./c/build/c/test_tir5
```

Run the C text streaming harness:

```sh
cd /Users/philip/src/OpenTrackIR
cmake -S . -B c/build
cmake --build c/build --target opentrackir_stream_dump
./c/build/c/opentrackir_stream_dump
```

Run the C++ OpenCV preview harness:

```sh
cd /Users/philip/src/OpenTrackIR
cmake -S . -B c/build
cmake --build c/build --target opentrackir_preview
./c/build/cpp/opentrackir_preview
```
