# Windows USB And Mouse Plan

## Scope

This note records the current plan for the Windows backend that will eventually sit behind the WinUI app.

The UI remains separate from the runtime/backend. Device I/O, frame processing, smoothing, and mouse output should stay off the UI thread.

## USB Plan

### Short term

Use `libusb-1.0` on Windows first so we can get the TrackIR working with the existing shared C transport shape and validate packet behavior quickly.

This is acceptable as an initial bring-up path because:

- the repo already uses `libusb` in the shared native path
- it reduces the amount of new Windows-specific code required up front
- it lets us prove the Windows app/runtime split before rewriting transport

### Long term

Prefer a native Windows transport built on `WinUSB`.

That should be the target architecture once the device behavior is stable on Windows because:

- `WinUSB` is the standard Windows path for a vendor-specific USB device
- it removes a permanent dependency on `libusb` for the Windows adapter
- it gives us a clean OS-specific transport layer while keeping shared protocol logic in `c/`

### Transport split

The intended separation is:

- shared C code in `c/` keeps protocol parsing, frame reconstruction, centroid math, smoothing, session logic, and reusable tracking state
- Windows transport owns device enumeration, open/close, control transfers, bulk reads, and runtime thread management
- the WinUI layer receives coarse runtime snapshots and never talks to USB directly

### Practical direction

Implement Windows transport in two phases:

1. `libusb` transport adapter to get the device working on Windows with the fewest moving parts
2. `WinUSB` transport adapter later if we want a more native Windows implementation or if deployment/driver behavior makes that worthwhile

The rest of the runtime should not care which transport implementation is active.

## Mouse Plan

### Language/runtime

The real mouse bridge should be native Windows code, not Python. We can do the equivalent of the `ctypes` example directly in C.

That means:

- a thin Windows mouse bridge in C
- imported User32 APIs such as `GetCursorPos` and `SendInput`
- no UI-thread cursor work

### Relative motion

Use relative mouse movement on Windows.

Reason:

- the user may also be moving the pointer with a normal mouse or trackpad
- absolute positioning would fight with that and override the user unexpectedly
- relative movement lets TrackIR contribute motion without owning the cursor position outright

### Output path

Use `SendInput` with relative mouse deltas for the first real Windows mouse backend.

Notes:

- do not use Python
- do not route mouse output through WinUI
- do not call deprecated `mouse_event`
- do not use `SetCursorPos` for the main tracking path, because that would force absolute positioning

### Runtime behavior

The runtime should:

- process one tracking update per frame
- coalesce to one mouse output event per processed frame
- keep smoothing, dead-zone, and jump filtering in shared C where possible
- publish only coarse status to the UI
- keep the hot path off the UI thread so `75 fps` tracking does not stall the app window

### Cursor state

The runtime may occasionally sample the current cursor position if needed for bookkeeping, but it should not depend on `GetCursorPos` every frame unless that proves necessary.

The primary control path is:

- TrackIR frame arrives
- shared tracking logic computes filtered delta
- Windows mouse bridge emits relative motion with `SendInput`

## Open questions for implementation

- whether Windows pointer acceleration needs to be compensated for or disabled in practice
- whether the final driver/install story is acceptable with `libusb` on Windows or if `WinUSB` should happen sooner
- whether we want a runtime option to switch between relative and absolute cursor output for testing
