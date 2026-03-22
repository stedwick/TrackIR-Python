# C++ Agent Notes

The `cpp/` directory is for native consumers of the C library. Treat it as a harness and integration layer, not the source of protocol truth.

## Focus areas

- `CMakeLists.txt`: C++ consumer targets.
- `opencv_preview/main.cpp`: simple desktop preview app for exercising the C API with OpenCV.
- Public C headers under `c/include/opentrackir/`: prefer shared tooling and rate-control helpers over duplicating harness logic in C++.

## C++-specific rules

- Keep protocol decoding in the C library. If the C++ app needs protocol knowledge, move that logic back into `c/`.
- Use the C++ code to validate library behavior, display frames, and inspect results.
- Keep the app simple and dependency-light beyond OpenCV and the C library.
- Reuse shared C helpers for argument parsing, rate limiting, and packet/frame stats when they already exist.
- Prefer small RAII wrappers for cleanup instead of scattered manual teardown.
- Do not duplicate centroid, packet parsing, or frame reconstruction logic here.

## Testing expectations

- Favor exercising the public C API rather than reaching into C internals.
- If UI behavior needs a helper, keep it small and deterministic where practical.
- Hardware validation belongs here, but pure protocol tests belong in `c/tests/`.

## Change strategy

- Preserve the role of this directory as a test harness and sample consumer.
- Keep the preview app straightforward enough to debug transport and protocol issues quickly.
- Avoid turning the preview app into a second implementation of the library.
