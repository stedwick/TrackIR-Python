# Jitter Follow-On Ideas

This document captures low-risk follow-ons that came out of the jitter brainstorm but are not part of the current implementation.

## Excluded On Purpose

Frame-rate tuning is intentionally excluded here. The current pass is focused on small mathematical filters that reduce jitter without changing the overall feel of the pointer. FPS changes are a separate tuning axis and should be evaluated independently so filter behavior is easier to judge.

## Candidate Follow-Ons

### Adaptive EMA

Use a lightweight exponential moving average instead of, or after, the current moving-average windows. The alpha can depend on delta magnitude or blob confidence so slow, low-confidence motion is smoothed more than strong motion.

### Alpha-Beta Filter

Use a simple constant-velocity predictor that keeps position and velocity state. This is much cheaper and easier to tune than a full Kalman filter and may help when a small number of frames are dropped.

### One-Frame Gap Bridging

If exactly one centroid sample is missing, carry forward the last velocity for a single frame or blend between adjacent good samples. This should stay limited to single-frame gaps so it does not invent motion during longer tracking loss.

### Sticky Centroid Selection Bias

Bias blob selection harder toward the previously selected centroid when two candidates are close in score. This could reduce blob flapping without changing the raw centroid math.

### Quantization Residual Carry

Track fractional cursor deltas across frames and only emit the integer portion each time. This may preserve tiny intended motion without increasing noise.

### Interpolation

Treat interpolation as a lower-priority experiment. It may help visible stutter from real frame drops, but it also risks smearing noisy centroid samples and changing the feel more than the simpler filters above.
