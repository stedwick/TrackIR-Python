# IOKit Migration Plan for macOS TrackIR Transport

## Goal

Replace the current macOS `libusb` transport path with an Apple-native USB transport while keeping the existing cross-platform C protocol and frame logic intact.

This is a later-phase plan, not an implementation commit.

## What should stay the same

- Keep packet parsing in `c/src/tir5_protocol.c`.
- Keep frame reconstruction in `c/src/tir5_frame.c`.
- Keep the public protocol-facing API in `c/include/opentrackir/tir5.h` as stable as practical.
- Keep SwiftUI focused on app state, preview rendering, and user interaction.

## What should change

- Isolate the USB transport currently embedded in `c/src/tir5_device.c`.
- Introduce a transport boundary so the device lifecycle can be backed by:
  - `libusb` for existing native workbench flows
  - macOS IOKit-based USB transport for the Mac app
- Keep TrackIR protocol sequencing above that transport boundary so initialization and streaming behavior do not get reimplemented twice.

## Target architecture

The long-term split should be:

- Shared C protocol layer:
  - packet parsing
  - frame building
  - centroid math
  - device command sequencing
- Transport backends:
  - `libusb` backend for `c/` native tools and cross-platform bring-up
  - IOKit backend for macOS integration
- macOS app integration layer:
  - owns preview state
  - converts frame buffers into `CGImage` or `NSImage`
  - does not contain USB packet logic

## Recommended implementation order

### Phase 1: Split transport from protocol sequencing

Refactor the current C device layer so raw USB operations are abstracted behind a small transport interface.

Suggested transport responsibilities:

- open device
- close device
- claim/release interface if needed
- write control/output packets
- read input chunks
- report endpoint and packet-size metadata

Suggested non-transport responsibilities to keep above that layer:

- TrackIR initialize sequence
- start/stop streaming sequence
- threshold and IR LED commands
- stream parsing
- packet decoding

Suggested result:

- `tir5_device.c` becomes protocol/device orchestration
- `tir5_transport_libusb.c` owns `libusb`
- a future `tir5_transport_iokit.c` owns macOS USB access

### Phase 2: Define a backend-neutral device core

Introduce a private transport vtable or function table inside `c/src/` rather than exposing transport details in the public header.

The device core should depend on transport callbacks like:

- `open`
- `close`
- `write_packet`
- `read_chunk`
- `get_summary`

This keeps the public API centered on TrackIR behavior instead of OS-specific transport types.

### Phase 3: Build a minimal macOS IOKit transport harness

Before touching the app, create a small native harness that proves the IOKit path can:

- find the TrackIR device by vendor/product ID
- open the correct interface
- locate bulk or interrupt endpoints
- send the existing command packets
- receive the same chunk stream the `libusb` backend sees

This harness should be CLI-first and live outside SwiftUI so transport debugging stays simple.

Success criteria for this phase:

- device enumeration works reliably
- endpoint discovery matches the current `libusb` backend
- one initialization cycle succeeds
- one streaming session yields parsable packets

### Phase 4: Implement the IOKit backend in C

Once the harness is proven, implement `tir5_transport_iokit.c`.

Backend responsibilities:

- map vendor/product matching to IOKit matching dictionaries
- open the device and selected interface
- discover input/output endpoints
- perform synchronous writes and reads with explicit timeouts
- translate backend failures into existing `otir_status` values

Important constraint:

- Do not move packet parsing or frame generation into Objective-C or Swift.

### Phase 5: Integrate with the macOS app

After the C backend works, add a thin macOS bridge layer that:

- owns device lifecycle
- runs the read loop off the main thread
- emits preview-ready frame buffers and status updates
- converts grayscale frame bytes into a native image for SwiftUI

The app should consume:

- connection status
- initialization/streaming state
- latest frame image
- last recoverable error

## Sandbox strategy

IOKit does not automatically guarantee sandbox compatibility for vendor-specific USB device access.

Plan for two outcomes:

- Best case: the macOS app can access the device directly with the needed entitlements and the IOKit transport works in-process.
- Fallback: keep the app sandboxed and move the IOKit transport into a helper or XPC service if direct access is blocked.

Because of that uncertainty, the transport split in Phase 1 is mandatory even if the first IOKit prototype is in-process.

## Proposed file layout

Possible future layout:

- `c/src/tir5_device.c`
  - backend-neutral TrackIR sequencing
- `c/src/tir5_transport_libusb.c`
  - existing `libusb` implementation
- `c/src/tir5_transport_iokit.c`
  - macOS transport implementation
- `c/src/tir5_transport_internal.h`
  - private transport interface
- `mac/OpenTrackIR/`
  - Swift bridge and preview model only

## Risks and unknowns

- The TrackIR interface and endpoint access pattern may map awkwardly onto IOKit APIs.
- Sandboxed access may still fail even with Apple-native USB APIs.
- Timeout behavior may differ enough from `libusb` that the read loop needs small policy adjustments.
- Endpoint discovery and interface claiming may not behave identically across hardware revisions.
- The current app target may need entitlement or helper-process changes even after the transport swap.

## Validation plan

Validation should be staged:

1. C unit tests still pass after the transport split.
2. Existing `libusb` CLI behavior remains unchanged.
3. IOKit harness can initialize and stream from real hardware.
4. macOS app can display live frames without reimplementing protocol logic in Swift.
5. Preview stop/start behavior remains stable across repeated device sessions.

## Explicit non-goals

- Do not rewrite protocol parsing in Swift.
- Do not introduce OpenCV into `mac/`.
- Do not make the public C API macOS-specific.
- Do not optimize for Mac App Store submission before the transport path is proven.

## First concrete tasks for later

When we start this work, the first tasks should be:

1. Extract the `libusb` calls out of `c/src/tir5_device.c` into a private transport backend.
2. Preserve existing behavior with a focused native regression pass.
3. Build a tiny macOS IOKit probe tool for enumeration, open, read, and write.
4. Only after the probe succeeds, add the IOKit backend to the shared C library.
