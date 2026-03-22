#ifndef OPENTRACKIR_TIR5_MOUSE_H
#define OPENTRACKIR_TIR5_MOUSE_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct otir_trackir_mouse_point {
    double x;
    double y;
} otir_trackir_mouse_point;

typedef struct otir_trackir_mouse_transform {
    double scale_x;
    double scale_y;
    double rotation_degrees;
} otir_trackir_mouse_transform;

typedef enum otir_trackir_mouse_smoothing_mode {
    OTIR_TRACKIR_MOUSE_SMOOTHING_MODE_LONG = 0,
    OTIR_TRACKIR_MOUSE_SMOOTHING_MODE_SHORT = 1,
    OTIR_TRACKIR_MOUSE_SMOOTHING_MODE_RAW = 2,
} otir_trackir_mouse_smoothing_mode;

enum {
    OTIR_TRACKIR_MOUSE_MAX_SMOOTHING_WINDOW = 31,
};

typedef struct otir_trackir_mouse_tracker_config {
    bool is_movement_enabled;
    double speed;
    double smoothing;
    double deadzone;
    bool avoid_mouse_jumps;
    double jump_threshold_pixels;
    bool use_adaptive_ema;
    bool use_alpha_beta_filter;
    bool use_quantization_residual_carry;
    otir_trackir_mouse_transform transform;
} otir_trackir_mouse_tracker_config;

typedef struct otir_trackir_mouse_tracker_state {
    bool has_previous_centroid;
    otir_trackir_mouse_point previous_centroid;
    otir_trackir_mouse_point delta_history[OTIR_TRACKIR_MOUSE_MAX_SMOOTHING_WINDOW];
    size_t delta_history_count;
    size_t delta_history_write_index;
    bool has_ema_filtered_delta;
    otir_trackir_mouse_point ema_filtered_delta;
    bool has_alpha_beta_state;
    otir_trackir_mouse_point alpha_beta_position;
    otir_trackir_mouse_point alpha_beta_velocity;
    otir_trackir_mouse_point quantization_residual;
    bool did_use_alpha_beta_filter;
} otir_trackir_mouse_tracker_state;

typedef struct otir_trackir_mouse_step {
    bool has_cursor_delta;
    otir_trackir_mouse_point cursor_delta;
    bool has_next_centroid;
    otir_trackir_mouse_point next_centroid;
} otir_trackir_mouse_step;

typedef struct otir_trackir_mouse_alpha_beta_result {
    otir_trackir_mouse_point position;
    otir_trackir_mouse_point velocity;
} otir_trackir_mouse_alpha_beta_result;

typedef struct otir_trackir_mouse_quantization_result {
    otir_trackir_mouse_point emitted_delta;
    otir_trackir_mouse_point residual;
} otir_trackir_mouse_quantization_result;

otir_trackir_mouse_point otir_trackir_mouse_transform_delta(
    otir_trackir_mouse_point raw_delta,
    double speed,
    otir_trackir_mouse_transform transform
);
otir_trackir_mouse_point otir_trackir_mouse_apply_vertical_gain(
    otir_trackir_mouse_point delta
);
size_t otir_trackir_mouse_short_smoothing_window(double smoothing);
size_t otir_trackir_mouse_long_smoothing_window(double smoothing);
otir_trackir_mouse_smoothing_mode otir_trackir_mouse_smoothing_mode_for_delta(
    otir_trackir_mouse_point raw_delta
);
bool otir_trackir_mouse_should_skip_jump(
    otir_trackir_mouse_point raw_delta,
    bool avoid_mouse_jumps,
    double jump_threshold_pixels
);
bool otir_trackir_mouse_is_inside_deadzone(
    otir_trackir_mouse_point short_average_delta,
    double deadzone
);
bool otir_trackir_mouse_point_is_zero(otir_trackir_mouse_point point);
double otir_trackir_mouse_adaptive_ema_alpha_for_delta(
    otir_trackir_mouse_point current_delta
);
otir_trackir_mouse_point otir_trackir_mouse_apply_adaptive_ema(
    otir_trackir_mouse_point previous_filtered_delta,
    otir_trackir_mouse_point current_delta
);
otir_trackir_mouse_alpha_beta_result otir_trackir_mouse_alpha_beta_update(
    otir_trackir_mouse_point previous_position,
    otir_trackir_mouse_point previous_velocity,
    otir_trackir_mouse_point measurement
);
otir_trackir_mouse_quantization_result
otir_trackir_mouse_apply_quantization_residual_carry(
    otir_trackir_mouse_point delta,
    otir_trackir_mouse_point residual
);
void otir_trackir_mouse_tracker_reset(otir_trackir_mouse_tracker_state *state);
otir_trackir_mouse_step otir_trackir_mouse_tracker_update(
    otir_trackir_mouse_tracker_state *state,
    bool has_current_centroid,
    otir_trackir_mouse_point current_centroid,
    otir_trackir_mouse_tracker_config config
);
otir_trackir_mouse_step otir_trackir_mouse_compute_step(
    bool has_previous_centroid,
    otir_trackir_mouse_point previous_centroid,
    bool has_current_centroid,
    otir_trackir_mouse_point current_centroid,
    bool is_movement_enabled,
    double speed,
    otir_trackir_mouse_transform transform
);

#ifdef __cplusplus
}
#endif

#endif
