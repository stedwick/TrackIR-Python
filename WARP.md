# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project Overview

TrackIR-Python is a reverse engineering project that attempts to read data directly from TrackIR hardware using Python, bypassing the need for NaturalPoint's proprietary drivers. The project uses USB communication to interface with the TrackIR camera and decode binary image data.

**Current Status**: Experimental - Can turn on the camera and receive binary data, but image format decoding is still in progress.

## Tech Stack

- **Python 3.12** (specified in `.python-version`)
- **USB Communication**: pyusb for low-level USB device interaction
- **Computer Vision**: OpenCV for image processing and display
- **Package Management**: uv for dependency management
- **Hardware Target**: TrackIR camera (USB Vendor ID: 0x131d, Product ID: 0x0155)

## Development Commands

### Environment Setup
```bash
# Ensure Python 3.12 is installed (project requirement)
python --version  # Should be 3.12.x

# Install dependencies using uv (recommended)
uv sync

# Alternative: Install dependencies using pip
pip install opencv-python pyusb
```

### Hardware Preparation
```bash
# On macOS, you may need to grant USB permissions
# The script will automatically attempt to detach kernel drivers when needed

# Check if TrackIR device is connected
lsusb | grep -i naturalpoint  # Linux
system_profiler SPUSBDataType | grep -A 5 -i naturalpoint  # macOS
```

### Running the Application
```bash
# Run the main TrackIR interface (with OpenCV visualization)
python trackir.py

# The application will:
# 1. Initialize the USB device
# 2. Set up LED controls
# 3. Start data streaming
# 4. Display real-time visualization in OpenCV window
# 5. Press 'q' to quit
```

### Development Workflow
```bash
# No formal test suite yet - testing is done via hardware interaction
python trackir.py  # Primary testing method

# For development iteration:
# 1. Make code changes
# 2. Run trackir.py to test with hardware
# 3. Monitor USB communication via print statements
# 4. Adjust protocol commands based on LinuxTrack reference
```

## Project Structure

```
TrackIR-Python/
├── trackir.py          # Main module with TrackIR class and protocol implementation
├── pyproject.toml      # Project configuration and dependencies
├── uv.lock            # Locked dependency versions (uv package manager)
├── .python-version    # Python version specification (3.12)
├── .gitmodules        # Git submodule configuration for LinuxTrack reference
├── linuxtrack/        # Empty submodule directory (reference implementation)
└── README.md          # Basic project description
```

## Core Architecture

### TrackIR Class (`trackir.py`)

The main `TrackIR` class handles all USB communication and data processing:

**Key Components:**
- **USB Device Management**: Device discovery, endpoint configuration, kernel driver handling
- **Protocol Implementation**: Command sequences based on LinuxTrack reverse engineering
- **Data Streaming**: Real-time frame capture and parsing
- **LED Control**: IR and status LED management
- **Visualization**: ASCII art and OpenCV-based display of tracking data

**USB Communication Flow:**
1. **Device Initialization**: Find device (VID:0x131d, PID:0x0155), reset, configure endpoints
2. **Command Sequence**: Send initialization commands based on LinuxTrack protocol
3. **LED Activation**: Turn on IR LEDs for tracking illumination
4. **Data Streaming**: Continuous reading of 4-byte data packets representing pixel coordinates
5. **Frame Parsing**: Convert raw USB data into visualization-ready format

### Data Format

Based on reverse engineering, the TrackIR uses this data structure:
- **4-byte packets**: `[vline, hstart, hstop, delimiter]`
- **vline**: Vertical line number (0-95, with extension bit for values > 255)
- **hstart/hstop**: Horizontal pixel start/end positions for detected features
- **delimiter**: Control byte with extension flags

### Visualization Pipeline

1. **Data Collection**: Read 4-byte USB packets in real-time
2. **Coordinate Mapping**: Convert packet data to pixel coordinates
3. **Image Generation**: Create OpenCV image with detected features as lines/pixels
4. **Display**: Real-time OpenCV window with FPS counter and scaling

## LinuxTrack Integration

The project references the LinuxTrack open-source implementation:
- **Git Submodule**: `https://github.com/uglyDwarf/linuxtrack.git`
- **Protocol Reference**: Command sequences and data formats derived from LinuxTrack's TrackIR support
- **Initialization Commands**: Based on LinuxTrack's device setup procedures

## Development Guidelines

### USB Device Handling
- Always call `usb.util.dispose_resources()` in cleanup
- Handle kernel driver detachment gracefully
- Use appropriate timeouts for USB operations (typically 100-1000ms)
- Implement proper error handling for USB exceptions

### Protocol Experimentation
- Log all USB communication (commands and responses) in hex format
- Use LinuxTrack as reference but expect variations
- Test LED control separately from data streaming
- Monitor device behavior with USB protocol analyzers when available

### Visualization Development
- Use OpenCV for real-time display (press 'q' to quit)
- Implement ASCII visualization for debugging without GUI
- Scale coordinates appropriately for display resolution
- Add FPS monitoring for performance assessment

## Hardware Requirements

- **TrackIR Camera**: Compatible with TrackIR 3, 4, or 5 models
- **USB Connection**: Direct USB connection (no hubs recommended for development)
- **Permissions**: USB device access permissions (may require sudo on some systems)
- **Platform**: Tested on macOS, should work on Linux with appropriate USB permissions

## Troubleshooting

### Device Not Found
```bash
# Check USB connection and device recognition
lsusb                           # Linux
system_profiler SPUSBDataType  # macOS

# Verify vendor/product ID (should be 131d:0155)
# Try different USB ports
# Ensure TrackIR software is not running (may claim device)
```

### USB Permission Issues
```bash
# Linux: Add user to appropriate groups
sudo usermod -a -G plugdev $USER  # Then logout/login

# macOS: May require running with elevated permissions
sudo python trackir.py
```

### No Data Reception
- Verify LED initialization commands succeeded
- Check that device responds to initialization sequence
- Monitor USB communication logs for error patterns
- Ensure proper endpoint configuration (IN/OUT endpoints)

### Display Issues
- Verify OpenCV installation: `python -c "import cv2; print(cv2.__version__)"`
- Check if GUI backend is available for OpenCV
- Try running without visualization (modify main() function)

## Development Workflow

### Adding New Protocol Commands
1. Reference LinuxTrack implementation for command sequences
2. Add command to appropriate method in TrackIR class
3. Log both sent commands and received responses
4. Test with hardware and verify expected behavior
5. Document command purpose and expected response format

### Debugging USB Communication
- Enable verbose USB logging via print statements
- Use hex format for all USB data display
- Compare with LinuxTrack's expected sequences
- Test commands individually before combining into sequences

### Performance Optimization
- Monitor frame rates and USB transfer times
- Optimize data parsing loops for real-time performance
- Consider buffering strategies for smooth visualization
- Profile memory usage during extended operation

This project is experimental and AI-generated, focusing on reverse engineering and protocol analysis rather than production use.