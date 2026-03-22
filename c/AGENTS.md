# C Agent Notes

The `c/` directory contains the native TrackIR library. Treat it as the reusable cross-platform core, not as an app layer.

## Focus areas

- `CMakeLists.txt`: C native targets and tests.
- `include/opentrackir/tir5.h`: public C API surface.
- `include/opentrackir/tir5_mouse.h`: shared mouse tracking, smoothing, and transform helpers.
- `include/opentrackir/tir5_session.h`: native session API for higher-level app consumers.
- `include/opentrackir/tir5_tooling.h`: shared CLI/FPS helpers for native harnesses.
- `src/tir5_protocol.c`: packet extraction, packet parsing, stripe decoding, and centroid math.
- `src/tir5_frame.c`: frame reconstruction and frame stats helpers.
- `src/tir5_mouse.c`: reusable centroid-to-cursor delta logic and adaptive smoothing.
- `src/tir5_device.c`: hardware transport and device lifecycle, currently through `libusb`.
- `src/tir5_session.c`: session lifecycle, snapshots, and shared polling state.
- `src/tir5_tooling.c`: small native CLI helpers shared by the C and C++ harnesses.
- `examples/`: small native C harnesses that exercise the public library API.
- `tests/test_tir5.c`: unit tests for pure logic and protocol behavior boundaries, including mouse and tooling helpers.

## C-specific rules

- Keep protocol parsing and math in pure functions whenever possible.
- Keep mouse tracking, smoothing, and session-side business rules reusable across native consumers.
- Do not move OpenCV or UI concerns into the C library.
- Do not move Quartz, AppKit, or other OS event-posting concerns into the C library.
- Keep USB and platform dependencies isolated in the device layer.
- Treat `libusb-1.0` as a required dependency for this native library.
- Prefer plain structs and caller-owned buffers over hidden heap allocation.
- If a new behavior can be tested without hardware, put it in a pure helper first.
- Name protocol fields after observed behavior, not guessed intent.

## Testing expectations

- Every new logical helper should get one focused unit test.
- Favor synthetic packets and exact field assertions over broad smoke tests.
- Favor unit coverage for mouse/session/tooling helpers when the behavior can be expressed without hardware.
- Keep tests independent from physical hardware unless the change is explicitly in transport code.
- When changing the public API, update tests to cover the new contract.

## Change strategy

- Preserve the separation between:
  - pure protocol/frame/mouse/session/tooling logic
  - hardware I/O
- Make the smallest defensible API change.
- Avoid broad refactors unless they clearly improve portability or protocol clarity.
