# Repository Guidelines

## Project Structure & Module Organization
`python-v2/` contains the active Python implementation: `trackir_tir5v3.py` is the current preview and dump entry point, `tir5v3.py` holds the USB protocol and parsing logic, and `tests/` contains hardware-independent unit tests. Project metadata for the active code now lives in `python-v2/pyproject.toml`, `python-v2/uv.lock`, and `python-v2/.python-version`. `z_old-v1-research/` archives the earlier protocol experiments, reference SDKs, screenshots, and the `linuxtrack` submodule; treat it as research material, not the default edit target. Root-level docs live in `README.md` and `AGENTS.md`.

## Build, Test, and Development Commands
- `cd python-v2 && uv sync` installs the pinned Python 3.12 environment for the active code.
- `cd python-v2 && uv run python trackir_tir5v3.py preview` starts the current TIR5V3 preview workflow.
- `cd python-v2 && uv run python trackir_tir5v3.py dump --seconds 3` records a short packet dump for debugging.
- `cd python-v2 && uv run python -m py_compile tir5v3.py trackir_tir5v3.py tests/test_tir5v3.py` performs a fast syntax and import-time compile check without running hardware code.
- `cd python-v2 && uv run python -m unittest discover -s tests` runs the Python unit tests.
- `cd python-v2 && python trackir_tir5v3.py preview` is the fallback when `uv` is unavailable.
- `system_profiler SPUSBDataType | grep -A5 -i naturalpoint` on macOS or `lsusb | grep -i naturalpoint` on Linux confirms the device is visible before debugging.
- `uv` does not provide a direct `package.json`-style arbitrary scripts table. The closest built-in option is `[project.scripts]` in `pyproject.toml` for Python entry points; for multi-command development recipes, prefer explicit `uv run ...` commands or a small task runner such as `make` or `just`.

## Coding Style & Naming Conventions
Follow standard Python style: four-space indentation, `snake_case` for functions and variables, `PascalCase` for classes, and uppercase names for USB constants and bit masks. Use the strongest practical type hints when writing Python: annotate function parameters and returns, dataclass fields, container element types, and `None` cases explicitly. Keep hardware-facing code explicit and readable; small helper functions are preferred over deeply nested inline parsing. No formatter or linter is committed yet, so keep changes PEP 8-friendly and avoid unrelated style churn.

## Testing Guidelines
There is now a small Python unit test suite under `python-v2/tests/` for hardware-independent parsing logic; run it with `cd python-v2 && uv run python -m unittest discover -s tests`. Use `cd python-v2 && uv run python -m py_compile ...` as a fast preflight check before hardware testing. The main verification flow is still a hardware smoke test: connect a TrackIR unit, run `cd python-v2 && uv run python trackir_tir5v3.py preview`, and confirm initialization logs, LED activation, packet flow, centroid overlay updates, and clean shutdown with `q`. Use descriptive test names such as `test_parse_packet_handles_extended_vline`.

## Commit & Pull Request Guidelines
Recent commits use short, imperative subjects such as `Enhance frame processing and visualization in TrackIR`. Keep commits focused and atomic. Pull requests should summarize the protocol or visualization change, list manual test steps, note the exact hardware or OS used, and attach logs or screenshots when behavior changes are visible.

## Hardware & Permissions Notes
USB access is the main operational risk. Use a direct USB connection, ensure the NaturalPoint software is not holding the device, and call out any permission requirements such as `sudo` on macOS or `plugdev` membership on Linux when documenting repro steps.
