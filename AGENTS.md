# Repository Guidelines

## Project Structure & Module Organization
`trackir.py` is the main entry point and currently contains the `TrackIR` class, USB protocol logic, and the OpenCV loop. Project metadata lives in `pyproject.toml`, `uv.lock`, and `.python-version`. `linuxtrack/`, `CameraSDK/`, and `TrackIR_SDK_Small_Samples_2.0/` are reference materials for protocol research; avoid casual edits there and explain any updates in your PR. Notes and project context live in `README.md`, `CLAUDE.md`, and `WARP.md`.

## Build, Test, and Development Commands
- `uv sync` installs the pinned Python 3.12 environment.
- `uv run python trackir.py` starts the camera interface and OpenCV visualization.
- `uv run python trackir_tir5v3.py preview` starts the current TIR5V3 preview workflow.
- `uv run python -m py_compile tir5v3.py trackir_tir5v3.py tests/test_tir5v3.py` performs a fast syntax and import-time compile check without running hardware code.
- `uv run python -m unittest discover -s tests` runs the Python unit tests.
- `python trackir.py` is the fallback when `uv` is unavailable.
- `system_profiler SPUSBDataType | grep -A5 -i naturalpoint` on macOS or `lsusb | grep -i naturalpoint` on Linux confirms the device is visible before debugging.
- `uv` does not provide a direct `package.json`-style arbitrary scripts table. The closest built-in option is `[project.scripts]` in `pyproject.toml` for Python entry points; for multi-command development recipes, prefer explicit `uv run ...` commands or a small task runner such as `make` or `just`.

## Coding Style & Naming Conventions
Follow standard Python style: four-space indentation, `snake_case` for functions and variables, `PascalCase` for classes, and uppercase names for USB constants and bit masks. Use the strongest practical type hints when writing Python: annotate function parameters and returns, dataclass fields, container element types, and `None` cases explicitly. Keep hardware-facing code explicit and readable; small helper functions are preferred over deeply nested inline parsing. No formatter or linter is committed yet, so keep changes PEP 8-friendly and avoid unrelated style churn.

## Testing Guidelines
There is now a small Python unit test suite under `tests/` for hardware-independent parsing logic; run it with `uv run python -m unittest discover -s tests`. Use `uv run python -m py_compile ...` as a fast preflight check before hardware testing. The main verification flow is still a hardware smoke test: connect a TrackIR unit, run `uv run python trackir_tir5v3.py preview`, and confirm initialization logs, LED activation, packet flow, and clean shutdown with `q`. Use descriptive test names such as `test_parse_packet_handles_extended_vline`.

## Commit & Pull Request Guidelines
Recent commits use short, imperative subjects such as `Enhance frame processing and visualization in TrackIR`. Keep commits focused and atomic. Pull requests should summarize the protocol or visualization change, list manual test steps, note the exact hardware or OS used, and attach logs or screenshots when behavior changes are visible.

## Hardware & Permissions Notes
USB access is the main operational risk. Use a direct USB connection, ensure the NaturalPoint software is not holding the device, and call out any permission requirements such as `sudo` on macOS or `plugdev` membership on Linux when documenting repro steps.
