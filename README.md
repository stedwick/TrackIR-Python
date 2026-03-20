# TrackIR-Python

A Python implementation for reverse engineering the TrackIR protocol to read camera data directly from TrackIR hardware without requiring NaturalPoint's proprietary drivers.

## 🎯 Project Overview

This project aims to provide an open-source alternative to NaturalPoint's TrackIR software by directly interfacing with TrackIR hardware via USB communication. The implementation is based on reverse engineering efforts and references the LinuxTrack open-source project.

**Current Status**: Experimental but working for TrackIR 5 TIR5V3 hardware. The active Python path initializes the camera, turns on the LEDs, decodes the compressed stripe stream, previews the detected blob, and computes its centroid.

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

- **Verified Vendor ID**: 0x131d (NaturalPoint)
- **Verified Product ID**: 0x0159 (TrackIR 5 / TIR5V3 path)
- Older TrackIR experiments are archived under `z_old-v1-research/`

## 🚀 Quick Start

### Prerequisites

1. **Python 3.12+** installed
2. **TrackIR hardware** connected via USB
3. **USB permissions** configured (see [Installation](#installation))

### Installation

```bash
# Clone the repository
git clone https://github.com/yourusername/TrackIR-Python.git
cd TrackIR-Python/python-v2

# Install dependencies using uv (recommended)
uv sync

# Alternative: Install with pip
pip install opencv-python pyusb
```

### Running the Application

```bash
# Start the current TIR5V3 preview workflow
cd python-v2
uv run python trackir_tir5v3.py preview
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
├── python-v2/         # Active TIR5V3 Python implementation
│   ├── trackir_tir5v3.py
│   ├── tir5v3.py
│   ├── tests/
│   ├── pyproject.toml
│   ├── uv.lock
│   └── .python-version
├── z_old-v1-research/ # Archived v1 experiments, SDKs, notes, and references
├── .gitmodules        # Git submodule configuration
├── README.md          # Root project documentation
└── AGENTS.md          # Contributor guidelines
```

## 🔧 Core Architecture

### Active Modules

- `python-v2/tir5v3.py` handles USB transport, device initialization, packet extraction, and stripe parsing.
- `python-v2/trackir_tir5v3.py` provides the interactive `identify`, `dump`, and `preview` workflows.
- `python-v2/tests/test_tir5v3.py` covers pure parsing helpers and centroid math.

#### Packet Flow

- **Device Initialization**: Claim the `131d:0159` device, negotiate the TIR5V3 transport, and enable the LEDs.
- **Stream Reassembly**: Recover complete packets from arbitrarily split USB bulk reads.
- **Type-5 Stripe Parsing**: Decode the compressed stripe payload into horizontal hits.
- **Centroid Calculation**: Convert stripe sums into a weighted blob `x,y` position.
- **Preview Rendering**: Draw the stripes and centroid overlay in OpenCV.

#### USB Communication Flow

1. **Device Initialization**: Find device, reset, configure endpoints
2. **Command Sequence**: Send initialization commands based on LinuxTrack protocol
3. **LED Activation**: Turn on IR LEDs for tracking illumination
4. **Data Streaming**: Continuous reading of 4-byte data packets
5. **Frame Parsing**: Convert raw USB data into visualization-ready format

### Data Format

The verified TIR5V3 path does not emit simple grayscale frames. It emits compressed type-5 stripe packets that describe bright hits in the sensor image. The preview reconstructs those stripes into a sparse image and computes the blob centroid from the packet payload.

## 🔍 Development

### Fast Validation Commands

```bash
# Syntax and import-time compile check
cd python-v2
uv run python -m py_compile tir5v3.py trackir_tir5v3.py tests/test_tir5v3.py

# Unit tests for hardware-independent parsing logic
uv run python -m unittest discover -s tests
```

`py_compile` is a quick way to catch syntax and indentation problems without running the hardware workflow.

### uv Workflow Notes

`uv` does not have a direct `package.json`-style arbitrary `scripts` section. The closest built-in option is `[project.scripts]` in `pyproject.toml`, which is good for Python entry points. For development recipes like tests, compile checks, and preview runs, prefer explicit `uv run ...` commands, or add a lightweight task runner such as `make` or `just` if the command list grows.

### USB Communication Debugging

The implementation includes comprehensive logging:

- All USB commands and responses logged in hex format
- Device information and endpoint details
- Real-time data stream analysis
- Error handling with detailed USB error codes

### Protocol Reference

The active TIR5V3 commands are based on `z_old-v1-research/linuxtrack` and observed device behavior:

```python
# High-level stages
identify_device()
wait_for_firmware_ready()
start_streaming()
decode_type_5_packets()
```

### Visualization Pipeline

1. **Data Collection**: Read USB bulk packets in real-time.
2. **Packet Recovery**: Resynchronize on valid TIR5V3 packet headers.
3. **Stripe Rendering**: Convert type-5 stripes into a sparse preview image.
4. **Display**: Draw the centroid overlay and packet stats in OpenCV.

## 🐛 Troubleshooting

### Device Not Found

```bash
# Check USB connection
lsusb                           # Linux
system_profiler SPUSBDataType  # macOS

# Verify vendor/product ID (currently verified path is 131d:0159)
# Try different USB ports
# Ensure TrackIR software is not running
```

### USB Permission Issues

```bash
# Linux: Add user to appropriate groups
sudo usermod -a -G plugdev $USER  # Then logout/login

# macOS: May require elevated permissions
cd python-v2 && sudo python trackir_tir5v3.py preview
```

### No Data Reception

- Verify LED initialization commands succeeded
- Check device response to initialization sequence
- Monitor USB communication logs for error patterns
- Ensure proper endpoint configuration

## 🤝 Contributing

This project is experimental and welcomes contributions:

1. **Protocol Analysis**: Compare new device behavior against the archived SDK and `linuxtrack` references.
2. **Tracking Algorithms**: Improve centroid smoothing and higher-level cursor control.
3. **Cross-platform Support**: Test and fix platform-specific USB behavior.
4. **Documentation**: Improve setup guides and troubleshooting.

### Development Workflow

```bash
# Make changes

# Run the fast compile check
cd python-v2
uv run python -m py_compile tir5v3.py trackir_tir5v3.py tests/test_tir5v3.py

# Run unit tests
uv run python -m unittest discover -s tests

# Test with hardware
uv run python trackir_tir5v3.py preview
```

When writing Python, prefer complete type hints for public helpers and new parsing logic: annotate parameters, return values, dataclass fields, collections, and optional values wherever practical.

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
