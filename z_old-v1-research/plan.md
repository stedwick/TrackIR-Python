Goal: get to a reliable single-blob `x,y` signal with the smallest possible amount of reverse engineering.

What the research says
- The official TrackIR SDK is not a raw camera SDK. It only exposes processed head-pose data through `NPClient`, so it is useful for validation but not for pulling frames from the camera.
- `linuxtrack/src/tir_img.c` already supports multiple wire formats:
  - TIR3/TIR4 style stripe packets, where each record is a bright run on one scanline.
  - TIR5 style stripe packets, where each record is an 8-byte compressed bright run.
  - One packet type is commented as “most probably B/W data from camera without any preprocessing”.
- `trackir.py` currently assumes a TIR4-like stripe format and a `128x96` display. That is almost certainly not ground truth.
- `trackir.py` hardcodes `0x131d:0x0155`. In `linuxtrack/src/libusb_ifc.c`, `0x0155` maps to a TIR3-era branch, not TIR5. That branch uses output endpoint `0x02` and `Video_on = {0x14, 0x00}`.

Working hypothesis
- The bytes you are already receiving may not be “bad image data”. They may already be the device’s thresholded stripe/blob stream.
- If that is true, raw grayscale frames are optional. For a single reflective sticker, stripe data is enough to compute centroid `x,y`.
- Before doing any new decoding work, we need to verify the actual hardware identity, endpoint layout, and packet type on your Mac.

Phase 1: establish ground truth
1. Verify the device identity from macOS and from the existing Python script output.
2. Capture one logged run of the current script while intentionally moving a single reflector left, right, up, and down.
3. Decide whether the current stream is:
   - stripe data we can use now, or
   - raw-ish grayscale data that needs a different parser, or
   - garbage caused by using the wrong endpoint / wrong model family.

Run this first
```bash
system_profiler SPUSBDataType | sed -n '/NaturalPoint/,+20p'
mkdir -p tmp/logs
uv run python trackir.py 2>&1 | tee "tmp/logs/trackir-$(date +%Y%m%d-%H%M%S).log"
```

While it is running:
- Use only one reflective point in view.
- Move it slowly left/right, then up/down.
- Note whether the OpenCV window follows the motion, looks mirrored, looks vertically scrambled, or shows unrelated static.
- Tell me the printed endpoints and the log filename.

Phase 2: classify the wire format
- If the stream is mostly 4-byte runs with rising line numbers and frame breaks on line reset, treat it as stripe data first.
- If packets are large and contain a different type marker, compare them against the “raw B/W” candidate path in `linuxtrack/src/tir_img.c`.
- If the device reports `0x0155`, stop using TIR5 assumptions until proven otherwise.

Phase 3: build the minimum useful tool
- Write one interactive probe script with modes:
  - `identify`: print VID/PID, endpoints, and guessed model family.
  - `init`: apply only the minimal model-correct startup sequence.
  - `dump`: save raw packets to `tmp/logs/`.
  - `stripe-view`: render stripe data into a debug image.
  - `raw-view`: attempt grayscale reconstruction if a raw packet type is found.
  - `centroid`: compute and overlay blob `x,y`.

Phase 4: choose the shortest path to success
- Fast path: if stripe mode is real, skip raw-image reverse engineering and go straight to centroid extraction.
- Slow path: only chase raw grayscale if stripe data is unstable, missing, or insufficient for your use case.

Success criteria
1. LEDs can be turned on repeatably.
2. Camera streaming can be started repeatably.
3. One run produces a saved log and a visible response to reflector motion.
4. We can classify the stream format with confidence.
5. We can compute one stable blob centroid from that format.
