# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

TrackIR-Python is a reverse engineering project that interfaces directly with TrackIR hardware via USB, bypassing NaturalPoint's proprietary drivers. The project reads binary data streams from the camera and visualizes tracking information using OpenCV.

**Status**: Experimental - Camera initialization, LED control, and data streaming work; image decoding and tracking algorithms are in active development.

## Development Commands

```bash
# Install dependencies (Python 3.12 required)
uv sync                          # Preferred method
pip install opencv-python pyusb  # Alternative

# Run the TrackIR interface
python trackir.py                # Press 'q' to quit OpenCV window

# Verify TrackIR device is connected (Vendor: 0x131d, Product: 0x0155)
system_profiler SPUSBDataType | grep -A5 -i naturalpoint  # macOS
lsusb | grep -i naturalpoint                               # Linux
```

## Architecture

### Single-File Design
All code lives in [trackir.py](trackir.py) which contains:
- `TrackIR` class: Handles USB communication, protocol implementation, LED control, and data streaming
- `main()` function: Runs the OpenCV visualization loop
- USB constants: `VENDOR_ID = 0x131d`, `PRODUCT_ID = 0x0155`

### USB Communication Flow
1. **Device initialization** - Find device, reset, configure endpoints, detach kernel driver if needed
2. **Command sequence** - Send initialization commands based on LinuxTrack protocol (see [trackir.py:232-242](trackir.py#L232-L242))
3. **LED activation** - Turn on IR and status LEDs for tracking illumination
4. **Data streaming** - Continuous reading of 4-byte packets: `[vline, hstart, hstop, delimiter]`
5. **Frame parsing** - Convert raw USB data into visualization-ready format

### Data Format
- **4-byte packets**: `[vline, hstart, hstop, delimiter]`
  - `vline`: Vertical line number (0-95, extended >255 via bit 0x20 in delimiter byte)
  - `hstart`/`hstop`: Horizontal pixel start/end positions for detected features
  - `delimiter`: Control byte with extension flags
- **Sensor resolution**: 128x96 pixels
- **Frame detection**: New frame starts when vline decreases from previous packet

### LED Control Constants
```python
TIR_LED_MSGID = 0x10
TIR_IR_LED_BIT_MASK = 0x80
TIR_GREEN_LED_BIT_MASK = 0x20
```

## LinuxTrack Reference

The [linuxtrack/](linuxtrack/) submodule provides the reference implementation:
- Protocol command sequences derived from LinuxTrack's TrackIR support
- Key reference files: `linuxtrack/python_prototype/tir4.py` and `linuxtrack/python_prototype/linux_tir4_prototype.py`
- All USB commands and responses are logged in hex format for debugging

## Hardware & Permissions

- **TrackIR models**: Compatible with TrackIR 3, 4, and 5
- **USB setup**: Direct USB connection recommended (avoid hubs during development)
- **Permissions**:
  - macOS: May require `sudo python trackir.py`
  - Linux: Add user to `plugdev` group: `sudo usermod -a -G plugdev $USER` (logout/login required)
- **Kernel driver**: Code automatically detaches kernel driver when active (see [trackir.py:56-58](trackir.py#L56-L58))

## Development Guidelines

### USB Protocol Debugging
- All USB communication is logged to console in hex format
- Compare command sequences with LinuxTrack reference implementation
- Test LED commands separately from data streaming
- Use appropriate timeouts: 100-1000ms for most operations

### Adding Protocol Commands
1. Reference LinuxTrack's command sequences in `linuxtrack/python_prototype/tir4.py`
2. Add commands to `init_device()` method or create new command methods
3. Log both sent commands and responses in hex format
4. Test with hardware and verify LED/sensor behavior
5. Document command purpose and expected response format

### Visualization Development
- Sensor resolution: 128x96 pixels (see [trackir.py:395-396](trackir.py#L395-L396))
- Display window: 640x480 with aspect ratio maintained
- Frame rate: Targets ~10 FPS with sleep control
- FPS counter displayed in top-left corner
- Press 'q' to quit OpenCV window

### Testing Approach
No formal test suite exists. Hardware testing workflow:
1. Connect TrackIR device via USB
2. Run `python trackir.py`
3. Verify initialization logs, LED activation, and frame counts in console
4. Check OpenCV window for visualization
5. Monitor USB communication logs for errors

### Cleanup & Resource Management
Always call `usb.util.dispose_resources()` in cleanup (see [trackir.py:481](trackir.py#L481)). The main loop handles LED shutdown and resource disposal in the `finally` block.

## Common Issues

**Device not found**: Ensure TrackIR is connected, NaturalPoint software isn't running, and device appears in USB listings with correct VID:PID (131d:0155)

**No data reception**: Verify LED initialization succeeded, check device responses to init sequence, ensure endpoints are configured correctly

**Permission errors**: Run with `sudo` (macOS) or add user to `plugdev` group (Linux)
