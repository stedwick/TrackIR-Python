Goal: Reverse engineer the TrackIR USB data stream well enough to assemble and display grayscale camera frames in real time using Python.

Acquisition & Protocol Recon
- Inventory current capture flow in `trackir.py` to document how `read_data` and `read_frame` operate, noting packet size assumptions, timeout handling, and the partial parsing already implemented around 4-byte tuples.
- Survey `linuxtrack/` (especially any `tir4` or `sensors` sources) plus `WARP.md` for hardcoded resolutions, endpoint IDs, LED commands, and any comments about sensor formats; jot down candidate packet headers, known control messages, and frame dimensions they expect.
- Capture multiple raw USB payloads (hex dumps already printed by `read_data`) into timestamped logs so we can diff patterns across frames, look for recurring headers, line numbers, and payload lengths.
- Map out device control flow: order of initialization commands, expected ACKs, and when streaming truly begins; confirm whether extra configuration (exposure/gain) influences payload length.

Frame Structure Hypothesis
- Based on collected dumps, propose the sensor resolution (e.g., 320×240 vs. 160×120) and pixel depth (likely 8-bit grayscale); align this with Linuxtrack constants if available.
- Identify packet header bytes (first 2 bytes currently skipped), meaning of vertical line counter (`vline`), and any bit flags (e.g., 0x20 for high bit) to determine per-line packet sequencing.
- Determine how many packets constitute a full frame by tracking when `vline` resets or monotonically increases; record expected line count range and per-line payload length.
- Investigate remaining bytes in each 4-byte tuple: infer whether they represent intensity values, x/y coordinates, or raw pixel run data; catalog multiple hypotheses to validate experimentally.
- Check for frame-level markers (frame ID, checksum) by inspecting bytes preceding reset of `vline`; prepare to validate once decoding loop runs.

Parsing Strategy
- Define a buffer manager that accumulates incoming USB transfers (`ep_in.read`) into a deque or bytearray, tagging each packet with arrival timestamp for debugging.
- Construct a state machine: `SEARCH_HEADER` (scan for recognizable frame start), `ACCUMULATE_LINES` (append per-line payloads), `FRAME_COMPLETE` (trigger downstream processing), and `DESYNC` (if unexpected header/line jump).
- Establish rules for packet acceptance: verify ascending `vline`, enforce max gap tolerance, and discard or log packets that violate sequence.
- Plan logic to reconstruct the full 2D array: map each line’s payload to a row in a NumPy array, applying any necessary bit unpacking if pixels are packed (e.g., 1-bit monochrome).
- Include resilience features: timeout handler that resets to `SEARCH_HEADER`, counters for dropped frames, and structured logs summarizing anomalies for later review.

Data Interpretation Experiments
- After first pass assembly, run diagnostic transforms on the image buffer: compute min/max/mean intensity, histogram of pixel values, and visualize binary masks to confirm dynamic range.
- If raw bytes appear packed bits, implement a test routine to unpack each byte into 8 pixels and visualize; likewise test for 5- or 6-bit values if histogram suggests limited range.
- Compare reconstructed frames to expected patterns (e.g., LED blob) and adjust mapping (endianess, row order, column stride) until alignment matches manual observations.
- Cross-reference with Linuxtrack functions to verify scaling factors, coordinate systems, or calibration constants.

Real-Time Loop Design
- Outline main acquisition loop: initialize device, start streaming, discard warm-up frames, then continuously call packet assembler to fetch complete frames.
- Insert periodic status prints (frame rate, dropped packet count, current exposure) throttled to once per second to avoid log spam.
- Implement graceful shutdown triggered by keyboard input (`q` via OpenCV window or terminal) ensuring USB resources released (`usb.util.dispose_resources`).
- Provide hooks for optional recording: e.g., press `s` to save current frame as PNG for offline analysis.

Visualization Plan
- Use OpenCV for display: convert assembled NumPy frame to `uint8`, apply `cv2.normalize` or CLAHE if contrast needs stretching, and show via `cv2.imshow` in resizable window (`cv2.WINDOW_NORMAL`).
- Overlay debug HUD using `cv2.putText` (frame ID, timestamp, FPS, packet stats) for quick instrumentation.
- Allow toggling visualization modes (raw, thresholded, edge-detected) through keyboard shortcuts to help inspection without code changes.
- Ensure GUI loop respects USB read rate: use `cv2.waitKey(1)` and decouple processing so display can keep pace with capture.

Verification & Iteration
- After first working run, log several consecutive frames to disk and inspect manually to confirm stability.
- Update plan with insights about protocol gaps, and schedule deeper reverse-engineering tasks (e.g., decoding metadata frames 0x10/0x40).
- Document findings in `WARP.md` (packet structure tables, command meanings) so future work builds on confirmed knowledge before expanding to multi-LED tracking.
