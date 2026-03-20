# SwiftUI macOS App Plan

## Current Context

- `python-v2/` is the active, working Python implementation.
- The Python path already does the important hardware work:
  - initializes the TrackIR 5 / TIR5V3 device
  - turns on the LEDs
  - starts streaming
  - decodes packets
  - computes the blob centroid
- The non-UI Python pipeline is fast enough for the target use case:
  - preview with OpenCV window: about 45 fps
  - parse-only / non-display path: about 124 fps
- `z_old-v1-research/` is archived reference material and should not be the default edit target.

## Goal

Build a native macOS SwiftUI app, using Xcode for project creation, signing, and builds, while having Codex do most of the source-code work.

## Recommended Approach

Do this in phases. Keep `python-v2/` intact as the known-good reference while the Swift app is built.

### Phase 1: Create the Native App Shell

Human step:

1. Open Xcode.
2. Create a new project:
   - Platform: `macOS`
   - Template: `App`
   - Interface: `SwiftUI`
   - Language: `Swift`
   - Tests: enabled
3. Create it in a new repo folder, for example `swift-macos-v1/`.
4. Verify the blank app builds successfully once.

After that, report back with:

- the folder path Xcode created
- the app name
- whether the blank app built successfully

### Phase 2: Keep the First Native Milestone Small

The first SwiftUI version should only do this:

- open a window
- show connection state
- show logs / status text
- show the current centroid `x,y`
- provide start / stop controls

Do not start with cursor movement yet.

### Phase 3: Bridge to Python First

Recommended first architecture:

- SwiftUI frontend
- Python backend subprocess
- simple JSON line protocol between them

Why:

- it reuses the working TrackIR code immediately
- it keeps USB reverse engineering separate from UI work
- it gives the Swift port a known-good oracle

### Phase 4: Port Pure Logic to Swift

After the UI shell is working, port the hardware-independent logic first:

- packet models
- packet extraction / resynchronization
- stripe decoding
- centroid math

These should become pure Swift functions with unit tests using captured packet fixtures.

### Phase 5: Port the USB Layer Last

Only after the Swift parser matches Python behavior:

- port device initialization
- port LED / streaming commands
- replace the Python subprocess with native Swift USB access

This is the highest-risk part and should not be the first milestone.

## Working Principle

Use Xcode where Apple tooling is strongest. Use Codex for most coding, refactoring, testing, and iteration. Keep changes incremental and keep `python-v2/` as the validation reference until the native path fully replaces it.
