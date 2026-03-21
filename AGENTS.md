# OpenTrackIR Agent Notes

This repository is for reverse-engineering NaturalPoint TrackIR hardware so it can run without the proprietary SDK and eventually support multiple operating systems through open implementations.

## Primary objective

When making changes here, optimize for protocol clarity and cross-platform portability first. The code should help answer questions like:

- How does the device initialize?
- What packet formats are present on the wire?
- Which behaviors are hardware-specific versus host-side assumptions?
- What can be expressed as reusable logic across Python, C, C++, and platform adapters?

## Repo expectations

- Make the smallest defensible change.
- Reuse existing helpers before adding new abstractions.
- Put business logic in pure functions whenever practical.
- Add one unit test for each logical piece of business logic you introduce.
- Do not hide protocol errors behind broad exception handling.
- Do not revert unrelated work from other contributors or agents.

## Documentation standard

- Capture reverse-engineering findings in code comments or docs only when they add concrete value.
- Prefer naming that reflects observed protocol behavior instead of speculative intent.
- If a constant or packet field is not fully understood, say so directly.
- Keep logs, dumps, and helper names aligned so a packet observed in one tool can be traced in another.

## Cross-platform standard

- Avoid baking OS-specific assumptions into shared protocol code.
- Keep hardware communication, parsing, and visualization separated.
- If platform-specific behavior is required, isolate it in the platform directory or a clearly named adapter layer.

## Validation

- For large changes, run the relevant linter, build, and tests.
- For small doc-only changes, no extra validation is required.

## Directory guide

- `python/`: fastest place to iterate on USB transport, logging, decoding, and tests.
- `c/`, `cpp/`: native implementations and performance-sensitive experiments.
- `mac/`, `win/`, `nix/`: platform-specific integration layers and notes.
