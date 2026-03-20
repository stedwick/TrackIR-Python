# Repository Guidelines

## Project Structure & Module Organization
`trackir.py` is the main entry point and currently contains the `TrackIR` class, USB protocol logic, and the OpenCV loop. Project metadata lives in `pyproject.toml`, `uv.lock`, and `.python-version`. `linuxtrack/`, `CameraSDK/`, and `TrackIR_SDK_Small_Samples_2.0/` are reference materials for protocol research; avoid casual edits there and explain any updates in your PR. Notes and project context live in `README.md`, `CLAUDE.md`, and `WARP.md`.

## Build, Test, and Development Commands
- `uv sync` installs the pinned Python 3.12 environment.
- `uv run python trackir.py` starts the camera interface and OpenCV visualization.
- `python trackir.py` is the fallback when `uv` is unavailable.
- `system_profiler SPUSBDataType | grep -A5 -i naturalpoint` on macOS or `lsusb | grep -i naturalpoint` on Linux confirms the device is visible before debugging.

## Coding Style & Naming Conventions
Follow standard Python style: four-space indentation, `snake_case` for functions and variables, `PascalCase` for classes, and uppercase names for USB constants and bit masks. Keep hardware-facing code explicit and readable; small helper functions are preferred over deeply nested inline parsing. No formatter or linter is committed yet, so keep changes PEP 8-friendly and avoid unrelated style churn.

## Testing Guidelines
There is no formal automated test suite yet. The primary verification flow is a hardware smoke test: connect a TrackIR unit, run `uv run python trackir.py`, and confirm initialization logs, LED activation, packet flow, and clean shutdown with `q`. If you extract hardware-independent parsing logic, add focused unit tests under a new `tests/` directory and use descriptive names such as `test_parse_packet_handles_extended_vline`.

## Commit & Pull Request Guidelines
Recent commits use short, imperative subjects such as `Enhance frame processing and visualization in TrackIR`. Keep commits focused and atomic. Pull requests should summarize the protocol or visualization change, list manual test steps, note the exact hardware or OS used, and attach logs or screenshots when behavior changes are visible.

## Hardware & Permissions Notes
USB access is the main operational risk. Use a direct USB connection, ensure the NaturalPoint software is not holding the device, and call out any permission requirements such as `sudo` on macOS or `plugdev` membership on Linux when documenting repro steps.
