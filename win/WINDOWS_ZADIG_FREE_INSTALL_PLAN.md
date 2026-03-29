# Windows Zadig-Free Install Plan

## Goal

Make Windows installation simple enough that end users do not need to know what Zadig is or manually swap USB drivers.

The target experience is:

1. download OpenTrackIR
2. run the installer
3. plug in TrackIR
4. launch the app

## Current constraint

Today the Windows app works by talking to the TrackIR through `libusb`, and that requires the device to be bound to a compatible Windows USB driver such as `WinUSB`.

Zadig solves that manually, but it is only a developer-friendly stopgap. It is not the final install story.

## Important reality

There are two different goals here:

- no manual Zadig step
- no driver change at all

The first goal is realistic.

The second goal may not be realistic if the TrackIR firmware does not expose a standard Windows class interface that the app can use directly. If the hardware requires a vendor-specific interface, Windows still needs the device bound to a driver that OpenTrackIR can talk to.

So the practical plan is:

- remove Zadig from the user workflow
- keep driver setup inside the OpenTrackIR installer

## Recommended direction

### 1. Keep the current bring-up path short term

Use the existing Windows `libusb` path while the runtime and preview work stabilize.

This keeps the native protocol/session code moving without blocking on installer work too early.

### 2. Move the Windows transport to native `WinUSB`

Long term, the Windows runtime should use `WinUSB` directly instead of depending on `libusb`.

Benefits:

- more native Windows transport layer
- less third-party runtime baggage in the final app
- clearer installer and driver story
- easier support and documentation for Windows users

Important note:

Moving from `libusb` to `WinUSB` improves the runtime architecture, but it does not by itself remove the need to bind the device to a compatible driver.

### 3. Ship a signed driver package with OpenTrackIR

The installer should install a signed driver package that binds the TrackIR to `WinUSB` automatically.

That means:

- an `.inf` that associates the TrackIR hardware IDs with `WinUSB`
- a signed catalog for Windows installation
- installer logic that stages and installs the package

This replaces the manual Zadig step with a normal installer flow.

## Installer plan

The Windows installer should do three jobs:

1. install the OpenTrackIR app
2. install the TrackIR `WinUSB` driver package if needed
3. explain clearly if a reboot, replug, or app restart is required

Recommended flow:

1. detect whether a compatible TrackIR is connected
2. detect whether it is already bound to the expected driver
3. if not, install the OpenTrackIR driver package
4. prompt the user to unplug and replug the device if Windows needs that to re-enumerate
5. launch OpenTrackIR

## Packaging direction

The likely packaging path is:

- packaged WinUI app for the desktop app itself
- separate installer/bootstrapper for prerequisites and driver installation

The driver-installing bootstrapper is the important piece because the app package alone is not the right place to own low-level USB driver binding.

## Coexistence caveat

This needs to be documented honestly:

- if OpenTrackIR binds the TrackIR to its own `WinUSB` driver path, the proprietary NaturalPoint software may stop working with that device until the driver is changed back

That is not a Zadig problem. It is a device-driver ownership problem.

The installer and docs should say this clearly.

## Best-case alternative to investigate

The only real path to "no manual step and no custom driver install" would be if the TrackIR firmware already exposes something Windows can use with an in-box driver and without replacing the vendor binding.

That would require confirming one of these:

- the device already works cleanly through a standard HID path
- the device exposes compatible Microsoft OS descriptors that let `WinUSB` bind automatically
- there is another stable inbox Windows interface available without driver replacement

At the moment, this should be treated as unlikely until proven.

## Implementation phases

### Phase 1

Keep the current `libusb` runtime and document Zadig for developers only.

### Phase 2

Implement a native Windows `WinUSB` transport adapter behind the existing runtime boundary.

### Phase 3

Create and sign an OpenTrackIR Windows driver package that binds TrackIR hardware IDs to `WinUSB`.

### Phase 4

Add installer/bootstrapper support so the driver package is installed automatically for end users.

### Phase 5

Rewrite the public Windows setup docs so they say "run the installer" instead of "download Zadig."

## Release requirements

Before calling the Windows install story finished, OpenTrackIR should have:

- a stable native Windows transport path
- a signed driver package
- a repeatable installer flow on a clean Windows machine
- documentation for how to revert the driver if the user wants to return to NaturalPoint software

## Open questions

- what exact hardware IDs the Windows driver package should claim
- whether multiple TrackIR revisions need separate INF entries
- whether OpenTrackIR should offer a helper to restore the previous driver
- whether the app should warn if the TrackIR is still bound to the vendor driver
- whether shipping `WinUSB` directly makes enough sense to justify replacing the current `libusb` transport sooner
