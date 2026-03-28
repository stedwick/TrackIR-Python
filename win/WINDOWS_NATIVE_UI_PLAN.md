# Windows Native UI Plan

## Goal

Move the Windows app away from a SwiftUI-shaped port and toward a UI that feels at home on Windows 11.

The Windows app should keep the same functionality and architectural boundaries, but the presentation should follow Windows conventions instead of copying the macOS visual language.

## Current problem

The current shell is too visually derived from the macOS app:

- oversized rounded rectangles everywhere
- soft glassy card treatment that reads more like a direct port than a Windows app
- status chips and sections that feel custom instead of native
- preview and settings composition that does not yet use familiar Windows information density

## Design direction

The target look should feel like a Windows utility app:

- cleaner geometry
- tighter spacing
- less ornamental rounding
- more use of native WinUI control patterns
- more emphasis on hierarchy, panels, and sectioning than on custom cards

This should still look polished, but the polish should come from alignment, typography, spacing, and standard Windows materials rather than from custom macOS-style surfaces.

## Visual principles

### 1. Use Windows structure, not macOS cards

Replace the current “stack of rounded cards” approach with a more Windows-native layout:

- top command/status area
- main content split into preview and settings regions
- advanced settings in a clearly secondary container
- more use of `Grid`, `NavigationView`-like sectioning, `Expander`, `GroupBox`-style framing, and pane-like surfaces

### 2. Reduce corner radii

Adopt smaller radii consistent with Windows 11 controls.

Current direction:

- large outer shells: reduce substantially
- cards/panels: use modest radii
- status indicators: dots, badges, or subtle pills instead of oversized capsule chips

### 3. Use Windows materials more sparingly

Keep the app background and shell aligned with Windows 11:

- Mica or subtle theme background at the window level
- solid or lightly elevated panels inside
- avoid full-page custom gradients as the primary identity
- avoid translucent white-on-color surfaces that fight dark mode

### 4. Increase information density

Windows utility apps usually tolerate more density than the current mock shell.

Adjust toward:

- tighter vertical spacing
- more compact headers
- less oversized padding inside setting panels
- status and telemetry that scan quickly

### 5. Favor native control affordances

Prefer controls that look like they belong to WinUI:

- standard `ToggleSwitch`
- `NumberBox`
- `Slider`
- `Expander`
- buttons and secondary text using stock spacing and typography

Avoid drawing custom UI where a native control pattern already exists.

## Layout plan

## Top area

Replace the current large title plus custom chips with a more Windows-native header:

- app title on the left
- one-line summary/status beneath or beside it
- refresh/settings/help actions on the right as normal buttons
- compact status row using indicator dots and short labels instead of large chip cards

Possible structure:

- row 1: title + actions
- row 2: TrackIR, Video, Mouse, Runtime state

## Main area

Use a two-column desktop layout by default:

- left: preview and telemetry
- right: controls

On narrow widths, stack vertically.

This will feel more like a Windows desktop utility than the current sequential “full-width section stack”.

## Preview section

Make the preview region feel more like a device monitor pane:

- fixed-size preview area
- compact caption and status line
- telemetry beneath in a denser grid or definition-list style
- less decorative framing around the preview itself

The preview should look like an operational tool, not a showcase panel.

## Controls section

Break controls into grouped settings panels instead of one big generic “Controls” card.

Suggested grouping:

- Runtime
  - Enable TrackIR
  - Show Video
  - Enable Mouse Movement
  - TrackIR FPS
- Mouse
  - Mouse Speed
  - Mouse Toggle Hotkey
- Video
  - Flip Horizontal
  - Flip Vertical
  - Rotate

This will feel much closer to Windows settings/task-pane patterns.

## Advanced area

Keep advanced settings collapsed by default in an `Expander`, but style it as a proper secondary settings region rather than another oversized card.

Suggested grouping inside advanced:

- Filtering
  - Smoothing
  - Dead Zone
  - Avoid Mouse Jumps
  - Jump Threshold
- Session
  - Keep Awake
  - Timeout
- Detection
  - Minimum Blob Area
  - Centroid mode text
- Optional devices
  - X-keys Fast Mode
  - indicator light and status

## Status treatment

Current top status cards should be replaced by something more compact.

Preferred direction:

- small colored indicator dot
- label
- value

Examples:

- `TrackIR  On`
- `Video  Visible`
- `Mouse  On`
- `Runtime  Background`

These should read like utility status labels, not callouts.

## Typography plan

Use Windows-native type scale and weight more conservatively:

- smaller title than the current large hero header
- clearer distinction between section headers and control labels
- more use of secondary foreground brushes for helper text
- avoid oversized numbers unless they are genuinely operationally important

## Color plan

Keep color restrained and functional:

- neutral backgrounds
- accent color used for active indicators, selection, and focused controls
- status colors only where they communicate state
- no large brand-colored surfaces

Dark mode and light mode should both rely on Windows theme brushes first, with only a small set of app-specific overrides.

## Icon/app identity plan

Keep the shared OpenTrackIR icon, but let it be the primary brand signal instead of pushing branding into the page background or custom panel styling.

## Interaction plan

The redesign should also align interactions with Windows expectations:

- compact commands in the header
- clear keyboard focus behavior
- consistent tab order
- settings controls aligned in predictable rows
- no hidden gestures or Mac-like UI metaphors

## Implementation plan

### Phase 1

Reshape the shell without changing runtime behavior:

- remove the large custom gradient-first presentation
- reduce radii and padding
- replace the status chips with compact status rows
- move to a two-column layout on desktop
- regroup controls into Windows-style panels

### Phase 2

Refine native polish:

- use more theme resources and fewer custom brushes
- tune spacing and typography against Windows 11 defaults
- make the preview pane and settings pane feel balanced
- simplify any card styling that still looks too custom

### Phase 3

Add Windows utility features once the visual shell feels right:

- tray entry points
- proper hotkey capture UI
- runtime state transitions in the header/status area

## Non-goals

This redesign should not:

- change the runtime/backend separation
- rework TrackIR processing
- add USB or mouse integration
- introduce a cross-platform design system
- try to make the Windows app visually match macOS

## Acceptance criteria

The redesign is successful when:

- the app no longer reads like a direct macOS port
- light and dark mode both look native on Windows
- the main screen feels like a Windows desktop utility
- the UI remains responsive and easy to scan
- the layout still supports future runtime, tray, hotkey, and device integration cleanly
