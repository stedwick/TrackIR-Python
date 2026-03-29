# Windows X-Keys Fast Mode Plan

## Current read

The X-Keys foot pedal does not look like the TrackIR transport problem.

In the existing macOS implementation, the pedal is matched as a HID device in [XKeysFootPedalMonitor.swift](C:/Users/phili/src/OpenTrackIR/mac/OpenTrackIR/XKeysFootPedalMonitor.swift):

- vendor `0x05F3`
- product `0x042C` or `0x0438`
- usage page `0x000C`
- usage `0x0001`

That is a strong signal that the Windows implementation should also start with the normal HID stack, not `WinUSB`.

## Short answer

My view is:

- no, we probably do **not** want `WinUSB` for X-Keys fast mode
- yes, it is very likely safer and simpler to use Windows HID APIs
- this should be much lower risk than the TrackIR transport

## Why

Microsoft’s HID docs describe the normal user-mode path as:

- enumerate HID collections
- open the HID device with `CreateFile`
- use `HidD_*` / `HidP_*` helpers and input reports

Sources:

- [Opening HID collections](https://learn.microsoft.com/en-us/windows-hardware/drivers/hid/opening-hid-collections)
- [Human Interface Devices (HID)](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/_hid/)

By contrast, `WinUSB` is the standard answer for vendor-specific USB devices that do not already sit cleanly on an inbox class driver. That is the TrackIR situation, not obviously the X-Keys situation.

Also, the X-Keys product pages describe the foot pedal as a **USB HID controller** and explicitly mention HID-level software access on Windows.

Sources:

- [P.I. Engineering X-keys Smart Foot Pedal – Fully Programmable USB HID Controller](https://piengineering.com/products/xkfootrear)
- [X-keys foot pedal product page](https://xkeys.com/xkfootrear.html)

## Main concern

The only real risk is device mode.

These pedals can emulate multiple things:

- keyboard
- mouse
- media
- game controller
- low-level HID / software mode

If the user has programmed the pedal into a pure keyboard or mouse emulation mode, the app may see ordinary input events instead of the specific HID report shape we expect.

So the Windows implementation should be written to prefer the same low-level HID report path the macOS app uses, and the UI should surface a clear "pedal not in expected mode" state if needed.

## Options

### Option 1: Native Windows HID monitor

Use Windows HID enumeration and report reads directly in the app/runtime.

Shape:

- enumerate HID devices with SetupDi APIs
- filter by vendor/product ID and usage page/usage
- open matching device with `CreateFile`
- read input reports asynchronously
- mirror the macOS report parsing rule for the middle pedal bit

Pros:

- most aligned with how the macOS code already works
- no driver replacement
- no Zadig
- likely the most stable and supportable path

Cons:

- requires some Windows HID enumeration/report code
- need to handle unplug/replug and device-mode mismatches carefully

## Option 2: Raw Input layer

Use Windows Raw Input if the pedal presents as a keyboard-like or HID input source that Raw Input can observe reliably.

Pros:

- can integrate well with a background utility app
- avoids some manual report parsing if the device is exposed in a convenient way

Cons:

- less direct than matching the actual HID reports
- weaker fit if we want the same product-specific pedal-state logic as macOS
- more dependent on how the pedal is configured by the user

My take: not the first choice.

## Option 3: Rely on X-Keys hardware mode and treat it like a hotkey source

Tell users to program the pedal itself to emit a keyboard shortcut, then let OpenTrackIR react to that shortcut.

Pros:

- very little code
- could work without touching HID report parsing

Cons:

- loses the clean integrated "pedal presence / idle / pressed" model we already have on macOS
- depends on user-side device programming
- fragile if the pedal mapping changes

My take: acceptable fallback, not the proper integrated feature.

## Option 4: WinUSB

Treat the pedal like a generic USB device and talk to it through `WinUSB`.

Pros:

- unified mental model with the TrackIR transport

Cons:

- likely unnecessary
- likely wrong for a device already designed to live on the HID stack
- could create avoidable driver/install complexity
- could interfere with normal X-Keys compatibility expectations

My take: avoid unless we discover the HID path cannot provide the report stream we need.

## Recommendation

Recommended path:

1. implement a Windows HID monitor for X-Keys fast mode
2. keep the pedal optional and isolated from the TrackIR runtime
3. do not use `WinUSB` unless HID proves insufficient

That keeps the architecture clean:

- TrackIR camera: `libusb` now, `WinUSB` later
- X-Keys foot pedal: HID stack

## First implementation path

This is the path we should try first.

Implement a native Windows HID monitor with:

- `SetupDi` for HID interface enumeration
- `CreateFile` for opening the device path
- `HidD_GetAttributes` for vendor/product ID checks
- `HidD_GetPreparsedData` plus `HidP_GetCaps` for usage page / usage checks
- `ReadFile` for input-report reads on the opened HID handle

That gives us the same overall shape as the macOS implementation:

- discover matching pedal
- attach to input reports
- parse pressed / not pressed state
- publish a small snapshot to the runtime

## Concrete Windows API shape

The monitor should work like this:

1. call `HidD_GetHidGuid`
2. call `SetupDiGetClassDevs` for present HID interfaces
3. iterate with `SetupDiEnumDeviceInterfaces`
4. get the device path with `SetupDiGetDeviceInterfaceDetail`
5. open the device path with `CreateFile`
6. call `HidD_GetAttributes` and keep only:
   - vendor `0x05F3`
   - product `0x042C` or `0x0438`
7. call `HidD_GetPreparsedData` and `HidP_GetCaps`
8. keep only:
   - usage page `0x000C`
   - usage `0x0001`
9. read reports from the handle with `ReadFile`
10. mirror the macOS parsing rule:
    - pressed when `report[2] & 0x04` is nonzero

The first implementation does not need full generic HID abstraction. It just needs to detect the expected pedal and read its reports reliably.

## Proposed Windows types

Use a small Windows-only implementation under `win/OpenTrackIR.WinUI/OpenTrackIR.WinUI/Runtime`:

- `XKeysFootPedalMonitor`
  - owns device discovery, handle lifetime, and background report reads
- `XKeysMonitorSnapshot`
  - `Disabled`
  - `NotDetected`
  - `Ready`
  - `Pressed`
- `XKeysHidInterop`
  - flat P/Invoke declarations for `SetupDi`, HID, and `CreateFile` calls
- `XKeysReportLogic`
  - pure helper for report parsing and device-match decisions

The pure logic should mirror the macOS rules exactly:

- vendor/product matching
- usage page / usage matching
- middle-pedal bit parsing
- effective fast-speed multiplier rules

## Runtime integration shape

`NativeTrackIRRuntimeController` should not talk to `SetupDi` or HID directly.

Instead:

- it owns an optional `XKeysFootPedalMonitor`
- it subscribes to pedal snapshot changes
- when fast mode is enabled and the pedal is pressed, it multiplies the effective mouse speed
- when the pedal is missing or fails, it leaves TrackIR and mouse movement running normally

This keeps the pedal optional and prevents it from destabilizing the main camera path.

## Proposed implementation plan

### Phase 1

Add a Windows `XKeysFootPedalMonitor` service under `win/OpenTrackIR.WinUI/OpenTrackIR.WinUI/Runtime`.

Responsibilities:

- enumerate HID devices with `SetupDi`
- detect matching vendor/product/usage with HID APIs
- expose snapshot state:
  - disabled
  - not detected
  - ready
  - pressed

### Phase 2

Mirror the current macOS parsing rules:

- same vendor/product matching
- same usage page / usage matching
- same report parsing rule for the middle pedal bit
- same fast-mode speed multiplier behavior

### Phase 3

Wire the monitor into `NativeTrackIRRuntimeController`:

- when X-Keys fast mode is enabled, start the HID monitor
- when disabled, stop it
- when pressed, multiply the effective mouse speed
- update the existing UI indicator with real states instead of the placeholder

### Phase 4

Handle lifecycle and recovery:

- unplug/replug
- app hide/show
- pedal disabled/enabled
- report/open failures

The failure mode should be graceful:

- disable only X-Keys fast mode
- do not break camera preview or mouse movement

## Acceptance criteria

This is done when:

- enabling X-Keys fast mode does not require Zadig or driver replacement
- the pedal is detected automatically on Windows
- the UI indicator shows real state
- pressing the pedal increases mouse speed while held
- unplugging the pedal does not crash or stall the app
- the TrackIR camera path remains independent

## Bottom line

I do not think X-Keys fast mode should push us toward `WinUSB`.

My recommendation is to treat it as a separate HID-device problem and keep the implementation independent from the TrackIR USB transport stack.
