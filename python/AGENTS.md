# Python Agent Notes

The `python/` directory is the active reverse-engineering workbench for TrackIR protocol discovery. Treat it as the reference implementation for understanding device behavior, not just as a quick prototype.

## Focus areas

- `tir5v3.py`: protocol constants, USB transport behavior, packet extraction, decoding, and reusable math/helpers.
- `trackir_tir5v3.py`: CLI entrypoints, session logging, preview rendering, and human-facing diagnostics.
- `tests/test_tir5v3.py`: unit tests for pure logic and small behavior boundaries.

## Python-specific rules

- Keep packet parsing and transform logic in pure functions whenever possible.
- Put hardware I/O orchestration on the edges, and keep decoding independent from USB reads.
- Prefer dataclasses and explicit types when they clarify packet structure.
- If you learn a new packet rule, encode it in a helper plus a focused unit test.
- Do not mix OpenCV preview concerns into protocol parsing helpers.
- Do not add speculative protocol branches without a captured observation that justifies them.

## Testing expectations

- Every new business-rule helper should get one unit test.
- Favor tests that build synthetic packets and assert exact decoded fields.
- Keep tests deterministic and independent from physical hardware.
- If behavior depends on captured device output, isolate the transformation logic so the test can use byte fixtures.

## Logging and diagnostics

- Preserve raw bytes where that helps future analysis.
- Make log messages easy to correlate with packet numbers, packet types, and frame indices.
- Prefer adding small inspection helpers over scattering ad hoc print statements.

## Change strategy

- Start in Python when validating protocol ideas quickly.
- Once behavior is stable, keep the Python implementation clean enough to serve as the source model for C or C++ ports.
- Avoid large refactors unless they clearly improve protocol comprehension or testability.
