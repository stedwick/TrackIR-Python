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
