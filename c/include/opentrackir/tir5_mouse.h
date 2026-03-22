#ifndef OPENTRACKIR_TIR5_MOUSE_H
#define OPENTRACKIR_TIR5_MOUSE_H

#include <stdbool.h>
#include <stddef.h>

#include "tir5.h"

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

typedef struct otir_trackir_mouse_axis_reversal_state {
    int last_accepted_sign;
    int pending_reversal_sign;
    int pending_reversal_count;
} otir_trackir_mouse_axis_reversal_state;

typedef struct otir_trackir_mouse_axis_reversal_result {
    double filtered_delta;
    otir_trackir_mouse_axis_reversal_state next_state;
} otir_trackir_mouse_axis_reversal_result;

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
    int minimum_blob_area_points;
    otir_trackir_mouse_transform transform;
} otir_trackir_mouse_tracker_config;

typedef struct otir_trackir_mouse_tracker_state {
    bool has_previous_centroid;
    otir_trackir_mouse_point previous_centroid;
    otir_trackir_mouse_point centroid_history[3];
    size_t centroid_history_count;
    size_t centroid_history_write_index;
    otir_trackir_mouse_point delta_history[OTIR_TRACKIR_MOUSE_MAX_SMOOTHING_WINDOW];
    size_t delta_history_count;
    size_t delta_history_write_index;
    bool is_deadzone_latched;
    otir_trackir_mouse_axis_reversal_state x_reversal_state;
    otir_trackir_mouse_axis_reversal_state y_reversal_state;
} otir_trackir_mouse_tracker_state;

typedef struct otir_trackir_mouse_step {
    bool has_cursor_delta;
    otir_trackir_mouse_point cursor_delta;
    bool has_next_centroid;
    otir_trackir_mouse_point next_centroid;
} otir_trackir_mouse_step;

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
double otir_trackir_mouse_median3(double left, double middle, double right);
double otir_trackir_mouse_deadzone_exit_threshold(double deadzone);
bool otir_trackir_mouse_update_deadzone_latch(
    bool is_latched,
    otir_trackir_mouse_point short_average_delta,
    double deadzone
);
double otir_trackir_mouse_blob_confidence_score(
    int selected_blob_area_points,
    int selected_blob_brightness_sum,
    int minimum_blob_area_points
);
size_t otir_trackir_mouse_adjust_smoothing_window_for_confidence(
    size_t base_window,
    double confidence,
    bool is_long_window
);
otir_trackir_mouse_axis_reversal_result otir_trackir_mouse_filter_reversal_axis(
    double delta,
    otir_trackir_mouse_axis_reversal_state state,
    double threshold
);
bool otir_trackir_mouse_is_inside_deadzone(
    otir_trackir_mouse_point short_average_delta,
    double deadzone
);
bool otir_trackir_mouse_point_is_zero(otir_trackir_mouse_point point);
void otir_trackir_mouse_tracker_reset(otir_trackir_mouse_tracker_state *state);
otir_trackir_mouse_step otir_trackir_mouse_tracker_update(
    otir_trackir_mouse_tracker_state *state,
    bool has_current_centroid,
    otir_trackir_mouse_point current_centroid,
    int selected_blob_area_points,
    int selected_blob_brightness_sum,
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
