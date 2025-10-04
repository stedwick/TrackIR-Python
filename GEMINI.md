# Gemini Code Assistant Context

## Project Overview

This project is a Python-based implementation for reverse-engineering the TrackIR protocol. The goal is to directly interface with TrackIR hardware (cameras) using standard USB libraries, bypassing the need for NaturalPoint's proprietary drivers. The project captures raw data from the TrackIR camera and provides a real-time visualization of the data using OpenCV.

The core of the project is the `trackir.py` file, which contains the `TrackIR` class. This class encapsulates the logic for:
- Discovering and initializing the TrackIR device via its USB vendor and product IDs.
- Sending control commands to the device to start/stop data streaming and control the LEDs.
- Reading data from the device's input endpoint.
- Parsing the received data to reconstruct the image from the camera.
- Visualizing the camera feed in real-time using OpenCV.

The project also includes a `linuxtrack` submodule, which is a C-based open-source implementation for TrackIR on Linux. This is likely used as a reference for the protocol reverse-engineering effort.

## Building and Running

### Dependencies

The project's dependencies are listed in `pyproject.toml` and can be installed using `uv` or `pip`.

- `opencv-python`: Used for image processing and real-time visualization of the camera feed.
- `pyusb`: Used for low-level USB communication with the TrackIR device.

### Running the Application

The main application can be run directly from the command line:

```bash
python trackir.py
```

This will:
1. Initialize the TrackIR device.
2. Start streaming data from the camera.
3. Open an OpenCV window to display the real-time camera feed.
4. Print debug information about the USB communication to the console.

## Development Conventions

- The code is written in Python 3.12+.
- The `TrackIR` class in `trackir.py` is the main entry point for interacting with the hardware.
- The `main()` function in `trackir.py` serves as a testbed and example of how to use the `TrackIR` class.
- The project relies on the `linuxtrack` project as a reference for the TrackIR protocol.
- The `README.md` file is well-maintained and provides a good starting point for understanding the project.
