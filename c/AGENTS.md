# C Agent Notes

The `c/` directory contains the native TrackIR library. Treat it as the reusable cross-platform core, not as an app layer.

## Focus areas

- `CMakeLists.txt`: C native targets and tests.
- `include/opentrackir/tir5.h`: public C API surface.
- `src/tir5_protocol.c`: packet extraction, packet parsing, stripe decoding, and centroid math.
- `src/tir5_frame.c`: frame reconstruction and frame stats helpers.
- `src/tir5_device.c`: hardware transport and device lifecycle, currently through `libusb`.
- `examples/`: small native C harnesses that exercise the public library API.
- `tests/test_tir5.c`: unit tests for pure logic and protocol behavior boundaries.

## C-specific rules

- Keep protocol parsing and math in pure functions whenever possible.
- Do not move OpenCV or UI concerns into the C library.
- Keep USB and platform dependencies isolated in the device layer.
- Treat `libusb-1.0` as a required dependency for this native library.
- Prefer plain structs and caller-owned buffers over hidden heap allocation.
- If a new behavior can be tested without hardware, put it in a pure helper first.
- Name protocol fields after observed behavior, not guessed intent.

## Testing expectations

- Every new logical helper should get one focused unit test.
- Favor synthetic packets and exact field assertions over broad smoke tests.
- Keep tests independent from physical hardware unless the change is explicitly in transport code.
- When changing the public API, update tests to cover the new contract.

## Change strategy

- Preserve the separation between:
  - pure protocol/frame logic
  - hardware I/O
- Make the smallest defensible API change.
- Avoid broad refactors unless they clearly improve portability or protocol clarity.
