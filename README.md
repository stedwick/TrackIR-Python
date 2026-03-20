# TrackIR-Python

A Python implementation for reverse engineering the TrackIR protocol to read camera data directly from TrackIR hardware without requiring NaturalPoint's proprietary drivers.

## 🎯 Project Overview

This project aims to provide an open-source alternative to NaturalPoint's TrackIR software by directly interfacing with TrackIR hardware via USB communication. The implementation is based on reverse engineering efforts and references the LinuxTrack open-source project.

**Current Status**: Experimental - Successfully initializes the camera, controls LEDs, and receives binary data streams. Image format decoding and tracking algorithms are in active development.

## ✨ Features

- **Direct USB Communication**: Bypasses proprietary drivers using pyusb
- **Real-time Data Streaming**: Captures and processes tracking data in real-time
- **LED Control**: Manages IR and status LEDs for optimal tracking
- **OpenCV Visualization**: Real-time display of tracking data with FPS monitoring
- **Protocol Analysis**: Comprehensive logging of USB communication for debugging
- **Cross-platform Support**: Works on macOS, Linux, and Windows

## 🛠️ Tech Stack

- **Python 3.12+** - Core runtime
- **pyusb 1.3.0+** - Low-level USB device communication
- **OpenCV 4.10.0+** - Computer vision and visualization
- **NumPy 2.2.1+** - Numerical computing for image processing
- **uv** - Modern Python package management

## 📋 Hardware Requirements

- **TrackIR Camera**: Compatible with TrackIR 3, 4, or 5 models
- **USB Connection**: Direct USB connection (avoid hubs for development)
- **System Permissions**: USB device access permissions

### Supported Hardware

- **Vendor ID**: 0x131d (NaturalPoint)
- **Product ID**: 0x0155 (TrackIR)

## 🚀 Quick Start

### Prerequisites

1. **Python 3.12+** installed
2. **TrackIR hardware** connected via USB
3. **USB permissions** configured (see [Installation](#installation))

### Installation

```bash
# Clone the repository
git clone https://github.com/yourusername/TrackIR-Python.git
cd TrackIR-Python

# Install dependencies using uv (recommended)
uv sync

# Alternative: Install with pip
pip install opencv-python pyusb
```

### Running the Application

```bash
# Start the TrackIR interface
python trackir.py
```

The application will:

1. 🔍 Detect and initialize the TrackIR device
2. 💡 Turn on IR LEDs for tracking illumination
3. 📡 Start continuous data streaming
4. 🖥️ Display real-time visualization in OpenCV window
5. ⏹️ Press 'q' to quit

## 📁 Project Structure

```
TrackIR-Python/
├── trackir.py          # Main TrackIR class and protocol implementation
├── pyproject.toml      # Project configuration and dependencies
├── uv.lock            # Locked dependency versions
├── .python-version    # Python version specification (3.12)
├── .gitmodules        # Git submodule configuration
├── linuxtrack/        # LinuxTrack reference implementation (submodule)
├── WARP.md           # Development documentation for WARP
└── README.md         # This file
```

## 🔧 Core Architecture

### TrackIR Class

The main `TrackIR` class handles all hardware interaction:

#### Key Components

- **USB Device Management**: Device discovery, endpoint configuration, kernel driver handling
- **Protocol Implementation**: Command sequences based on LinuxTrack reverse engineering
- **Data Streaming**: Real-time frame capture and parsing
- **LED Control**: IR and status LED management
- **Visualization**: ASCII art and OpenCV-based display

#### USB Communication Flow

1. **Device Initialization**: Find device, reset, configure endpoints
2. **Command Sequence**: Send initialization commands based on LinuxTrack protocol
3. **LED Activation**: Turn on IR LEDs for tracking illumination
4. **Data Streaming**: Continuous reading of 4-byte data packets
5. **Frame Parsing**: Convert raw USB data into visualization-ready format

### Data Format

Based on reverse engineering, the TrackIR uses this data structure:

- **4-byte packets**: `[vline, hstart, hstop, delimiter]`
- **vline**: Vertical line number (0-95, with extension bit for values > 255)
- **hstart/hstop**: Horizontal pixel start/end positions for detected features
- **delimiter**: Control byte with extension flags

## 🔍 Development

### USB Communication Debugging

The implementation includes comprehensive logging:

- All USB commands and responses logged in hex format
- Device information and endpoint details
- Real-time data stream analysis
- Error handling with detailed USB error codes

### Protocol Reference

Commands are based on LinuxTrack's implementation:

```python
# Example initialization sequence
init_sequence = [
    [0x12],           # Request version
    [0x14, 0x01],     # Initialize
    [0x14, 0x02],     # Start streaming
    [0x10, 0x02],     # Set data format
    [0x11, 0x80],     # Set exposure
    # ... more commands
]
```

### Visualization Pipeline

1. **Data Collection**: Read 4-byte USB packets in real-time
2. **Coordinate Mapping**: Convert packet data to pixel coordinates
3. **Image Generation**: Create OpenCV image with detected features
4. **Display**: Real-time window with FPS counter and scaling

## 🐛 Troubleshooting

### Device Not Found

```bash
# Check USB connection
lsusb                           # Linux
system_profiler SPUSBDataType  # macOS

# Verify vendor/product ID (should be 131d:0155)
# Try different USB ports
# Ensure TrackIR software is not running
```

### USB Permission Issues

```bash
# Linux: Add user to appropriate groups
sudo usermod -a -G plugdev $USER  # Then logout/login

# macOS: May require elevated permissions
sudo python trackir.py
```

### No Data Reception

- Verify LED initialization commands succeeded
- Check device response to initialization sequence
- Monitor USB communication logs for error patterns
- Ensure proper endpoint configuration

## 🤝 Contributing

This project is experimental and welcomes contributions:

1. **Protocol Analysis**: Help decode the binary data format
2. **Tracking Algorithms**: Implement head tracking from raw data
3. **Cross-platform Support**: Test and fix platform-specific issues
4. **Documentation**: Improve setup guides and troubleshooting

### Development Workflow

```bash
# Make changes to trackir.py
# Test with hardware
python trackir.py

# Monitor USB communication via print statements
# Adjust protocol commands based on results
```

## 📚 References

- **LinuxTrack**: Open-source TrackIR implementation used as reference
- **USB Protocol**: Reverse engineered from hardware analysis
- **NaturalPoint TrackIR**: Original hardware and software

## ⚠️ Disclaimer

This project is for educational and research purposes. It's an experimental reverse engineering effort and may not provide the same functionality as official NaturalPoint software. Use at your own risk.

## 📄 License

This project is open source. Please check the license file for details.

---

**Note**: This project was developed with AI assistance using Cursor. The implementation represents early-stage reverse engineering work and is actively being developed.

# Repository Guidelines

## Project Structure & Module Organization

`trackir.py` hosts the `TrackIR` class plus USB helpers and is the entry point for all runs. `pyproject.toml`, `uv.lock`, and `.python-version` pin the environment; update them together when adding dependencies. `WARP.md` holds reverse-engineering notes, while `linuxtrack/` is an optional reference submodule—keep it clean or document how to sync it in your PR. There is no `tests/` tree yet, so place any exploratory scripts next to the main module and mark them clearly.

## Build, Test, and Development Commands

- `uv sync` — install the locked dependency set into a local environment.
- `uv run python trackir.py` — start the camera session with OpenCV visualization (press `q` to exit).
- `python trackir.py` — fallback when uv is unavailable; confirm you are on Python 3.12.
- `system_profiler SPUSBDataType | grep -A5 -i naturalpoint` (macOS) / `lsusb | grep -i naturalpoint` (Linux) — verify the device before debugging USB errors.

## Coding Style & Naming Conventions

Stick to idiomatic Python: four-space indentation, descriptive lowercase_with_underscores names, and uppercase constants for USB IDs. Continue adding type hints, especially around byte buffers and command tables, so hardware interactions stay readable. Keep logging lightweight—use helper functions if you need to print structured USB traces.

## Testing Guidelines

Hardware runs are the acceptance test: connect a TrackIR unit, execute `uv run python trackir.py`, and confirm init logs, LED activity, and frame counts. Capture a short console excerpt or screenshot when filing issues. For pure parsing helpers, add quick unit-style functions that can run without hardware and document expected byte sequences inline.

## Commit & Pull Request Guidelines

Recent history favors concise, present-tense subjects (“Update README.md for hardware setup”); follow that format and add focused body bullets when context is needed. Reference issue IDs or hardware variations touched in the change. Pull requests should outline manual test steps, include relevant logs or imagery, and call out dependency or permission adjustments so reviewers can reproduce results.

## Hardware & Permissions Notes

The code detaches kernel drivers when needed; run with adequate permissions (Linux may require `sudo`) and clean up with `usb.util.dispose_resources()` if you add teardown logic. Prefer direct USB connections over hubs during protocol work, and document any alternate vendor/product IDs encountered so detection stays accurate.
