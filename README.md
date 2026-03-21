# OpenTrackIR

OpenTrackIR is a reverse-engineering workspace for NaturalPoint TrackIR hardware, with the explicit goal of removing the dependency on NaturalPoint's proprietary SDK and making device support work cross-platform.

The repo is organized by implementation target:

- `python/`: active protocol exploration, USB transport work, packet decoding, logging, and preview tooling.
- `c/`, `cpp/`: lower-level native implementations and experiments.
- `mac/`, `win/`, `nix/`: platform-specific notes, adapters, or future integration work.
- `tmp/`: scratch output and temporary artifacts.

## Project goals

- Identify and document the TrackIR device protocol.
- Reproduce initialization, streaming, and shutdown behavior without vendor SDKs.
- Build portable code paths that can be shared across macOS, Linux, and Windows.
- Keep the reverse-engineered behavior testable with small, isolated units.

## Current Python work

The Python implementation currently contains:

- USB device discovery and transport helpers for the TrackIR 5 v3 hardware path.
- Packet extraction and decoding helpers for the sensor stream.
- A small CLI for identification, packet dumping, and preview rendering.
- Unit tests around transport encoding, packet recovery, stripe decoding, centroid math, and shutdown sequencing.

## Working principles

- Prefer small, reversible changes over broad rewrites.
- Keep protocol knowledge in pure functions where possible.
- Add a focused unit test for each meaningful piece of business logic.
- Preserve raw observations in logs so assumptions can be checked against device behavior later.
- Avoid introducing dependencies on vendor SDKs, closed headers, or platform-locked assumptions.

## Status

This repository is an active reverse-engineering project, not a finished end-user product. Expect experimental code, incomplete platform parity, and evolving protocol understanding.
