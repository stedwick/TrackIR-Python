#include "opentrackir/tir5_mouse.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define OTIR_TRACKIR_MOUSE_SHORT_SMOOTHING_MIN 1
#define OTIR_TRACKIR_MOUSE_SHORT_SMOOTHING_MAX 10
#define OTIR_TRACKIR_MOUSE_VERTICAL_GAIN 1.25
#define OTIR_TRACKIR_MOUSE_LONG_SMOOTHING_THRESHOLD 0.2
#define OTIR_TRACKIR_MOUSE_SHORT_SMOOTHING_THRESHOLD 0.5
#define OTIR_TRACKIR_MOUSE_DEADZONE_EXIT_SCALE 1.5
#define OTIR_TRACKIR_MOUSE_DEADZONE_EXIT_PADDING 0.01
#define OTIR_TRACKIR_MOUSE_TINY_REVERSAL_THRESHOLD 0.35

static double otir_trackir_mouse_squared_magnitude(otir_trackir_mouse_point point);
static double otir_trackir_mouse_clamp(double value, double low, double high);
static bool otir_trackir_mouse_latest_centroid(
    const otir_trackir_mouse_tracker_state *state,
    otir_trackir_mouse_point *out_centroid
);
static void otir_trackir_mouse_store_centroid(
    otir_trackir_mouse_tracker_state *state,
    otir_trackir_mouse_point centroid
);
static otir_trackir_mouse_point otir_trackir_mouse_filtered_centroid(
    const otir_trackir_mouse_tracker_state *state,
    otir_trackir_mouse_point current_centroid
);
static otir_trackir_mouse_point otir_trackir_mouse_average_recent_deltas(
    const otir_trackir_mouse_tracker_state *state,
    size_t window
);
static void otir_trackir_mouse_store_delta(
    otir_trackir_mouse_tracker_state *state,
    otir_trackir_mouse_point delta
);
static otir_trackir_mouse_point otir_trackir_mouse_selected_delta(
    otir_trackir_mouse_smoothing_mode mode,
    otir_trackir_mouse_point raw_delta,
    otir_trackir_mouse_point short_average_delta,
    otir_trackir_mouse_point long_average_delta
);
static otir_trackir_mouse_point otir_trackir_mouse_filter_reversal_point(
    otir_trackir_mouse_tracker_state *state,
    otir_trackir_mouse_point delta
);

otir_trackir_mouse_point otir_trackir_mouse_transform_delta(
    otir_trackir_mouse_point raw_delta,
    double speed,
    otir_trackir_mouse_transform transform
) {
    const double flipped_x = raw_delta.x * transform.scale_x;
    const double flipped_y = raw_delta.y * transform.scale_y;
    const double radians = transform.rotation_degrees * M_PI / 180.0;
    const double rotated_x = (flipped_x * cos(radians)) - (flipped_y * sin(radians));
    const double rotated_y = (flipped_x * sin(radians)) + (flipped_y * cos(radians));

    return (otir_trackir_mouse_point){
        .x = rotated_x * speed,
        .y = rotated_y * speed,
    };
}

otir_trackir_mouse_point otir_trackir_mouse_apply_vertical_gain(
    otir_trackir_mouse_point delta
) {
    return (otir_trackir_mouse_point){
        .x = delta.x,
        .y = delta.y * OTIR_TRACKIR_MOUSE_VERTICAL_GAIN,
    };
}

size_t otir_trackir_mouse_short_smoothing_window(double smoothing) {
    const long rounded = lround(smoothing);

    if (rounded < OTIR_TRACKIR_MOUSE_SHORT_SMOOTHING_MIN) {
        return OTIR_TRACKIR_MOUSE_SHORT_SMOOTHING_MIN;
    }
    if (rounded > OTIR_TRACKIR_MOUSE_SHORT_SMOOTHING_MAX) {
        return OTIR_TRACKIR_MOUSE_SHORT_SMOOTHING_MAX;
    }

    return (size_t)rounded;
}

size_t otir_trackir_mouse_long_smoothing_window(double smoothing) {
    return (otir_trackir_mouse_short_smoothing_window(smoothing) * 3) + 1;
}

otir_trackir_mouse_smoothing_mode otir_trackir_mouse_smoothing_mode_for_delta(
    otir_trackir_mouse_point raw_delta
) {
    const double squared_magnitude = otir_trackir_mouse_squared_magnitude(raw_delta);

    if (squared_magnitude < OTIR_TRACKIR_MOUSE_LONG_SMOOTHING_THRESHOLD) {
        return OTIR_TRACKIR_MOUSE_SMOOTHING_MODE_LONG;
    }
    if (squared_magnitude < OTIR_TRACKIR_MOUSE_SHORT_SMOOTHING_THRESHOLD) {
        return OTIR_TRACKIR_MOUSE_SMOOTHING_MODE_SHORT;
    }

    return OTIR_TRACKIR_MOUSE_SMOOTHING_MODE_RAW;
}

bool otir_trackir_mouse_should_skip_jump(
    otir_trackir_mouse_point raw_delta,
    bool avoid_mouse_jumps,
    double jump_threshold_pixels
) {
    if (!avoid_mouse_jumps || jump_threshold_pixels <= 0.0) {
        return false;
    }

    return fabs(raw_delta.x) > jump_threshold_pixels ||
        fabs(raw_delta.y) > jump_threshold_pixels;
}

double otir_trackir_mouse_median3(double left, double middle, double right) {
    if (left > middle) {
        double swap = left;

        left = middle;
        middle = swap;
    }
    if (middle > right) {
        double swap = middle;

        middle = right;
        right = swap;
    }
    if (left > middle) {
        middle = left;
    }

    return middle;
}

double otir_trackir_mouse_deadzone_exit_threshold(double deadzone) {
    return fmax(
        deadzone * OTIR_TRACKIR_MOUSE_DEADZONE_EXIT_SCALE,
        deadzone + OTIR_TRACKIR_MOUSE_DEADZONE_EXIT_PADDING
    );
}

bool otir_trackir_mouse_update_deadzone_latch(
    bool is_latched,
    otir_trackir_mouse_point short_average_delta,
    double deadzone
) {
    const double magnitude = sqrt(
        otir_trackir_mouse_squared_magnitude(short_average_delta)
    );

    if (deadzone <= 0.0) {
        return false;
    }
    if (is_latched) {
        return magnitude > 0.0 &&
            magnitude < otir_trackir_mouse_deadzone_exit_threshold(deadzone);
    }

    return magnitude > 0.0 && magnitude < deadzone;
}

double otir_trackir_mouse_blob_confidence_score(
    int selected_blob_area_points,
    int selected_blob_brightness_sum,
    int minimum_blob_area_points
) {
    double area_score = 0.0;
    double brightness_score = 0.0;
    const double normalized_minimum_area = (double)(
        minimum_blob_area_points > 0 ? minimum_blob_area_points : 1
    ) * 2.0;

    if (selected_blob_area_points > 0) {
        area_score = otir_trackir_mouse_clamp(
            (double)selected_blob_area_points / normalized_minimum_area,
            0.0,
            1.0
        );
        brightness_score = otir_trackir_mouse_clamp(
            (double)selected_blob_brightness_sum /
                ((double)selected_blob_area_points * (double)OTIR_TIR5V3_DEFAULT_THRESHOLD),
            0.0,
            1.0
        );
    }

    return (area_score + brightness_score) * 0.5;
}

size_t otir_trackir_mouse_adjust_smoothing_window_for_confidence(
    size_t base_window,
    double confidence,
    bool is_long_window
) {
    size_t increase = 0;

    if (confidence < 0.35) {
        increase = is_long_window ? 6 : 2;
    } else if (confidence < 0.60) {
        increase = is_long_window ? 3 : 1;
    }

    if (base_window + increase > OTIR_TRACKIR_MOUSE_MAX_SMOOTHING_WINDOW) {
        return OTIR_TRACKIR_MOUSE_MAX_SMOOTHING_WINDOW;
    }

    return base_window + increase;
}

otir_trackir_mouse_axis_reversal_result otir_trackir_mouse_filter_reversal_axis(
    double delta,
    otir_trackir_mouse_axis_reversal_state state,
    double threshold
) {
    otir_trackir_mouse_axis_reversal_result result = {
        .filtered_delta = delta,
        .next_state = state,
    };
    const double magnitude = fabs(delta);
    const int sign = delta > 0.0 ? 1 : (delta < 0.0 ? -1 : 0);

    if (sign == 0) {
        result.next_state.pending_reversal_sign = 0;
        result.next_state.pending_reversal_count = 0;
        result.filtered_delta = 0.0;
        return result;
    }

    if (magnitude >= threshold || state.last_accepted_sign == 0 || sign == state.last_accepted_sign) {
        result.next_state.last_accepted_sign = sign;
        result.next_state.pending_reversal_sign = 0;
        result.next_state.pending_reversal_count = 0;
        return result;
    }

    if (state.pending_reversal_sign == sign) {
        result.next_state.pending_reversal_count += 1;
    } else {
        result.next_state.pending_reversal_sign = sign;
        result.next_state.pending_reversal_count = 1;
    }

    if (result.next_state.pending_reversal_count < 2) {
        result.filtered_delta = 0.0;
        return result;
    }

    result.next_state.last_accepted_sign = sign;
    result.next_state.pending_reversal_sign = 0;
    result.next_state.pending_reversal_count = 0;
    return result;
}

bool otir_trackir_mouse_is_inside_deadzone(
    otir_trackir_mouse_point short_average_delta,
    double deadzone
) {
    const double magnitude = sqrt(
        otir_trackir_mouse_squared_magnitude(short_average_delta)
    );

    return magnitude > 0.0 && magnitude < deadzone;
}

bool otir_trackir_mouse_point_is_zero(otir_trackir_mouse_point point) {
    return point.x == 0.0 && point.y == 0.0;
}

void otir_trackir_mouse_tracker_reset(otir_trackir_mouse_tracker_state *state) {
    if (state == NULL) {
        return;
    }

    *state = (otir_trackir_mouse_tracker_state){0};
}

otir_trackir_mouse_step otir_trackir_mouse_tracker_update(
    otir_trackir_mouse_tracker_state *state,
    bool has_current_centroid,
    otir_trackir_mouse_point current_centroid,
    int selected_blob_area_points,
    int selected_blob_brightness_sum,
    otir_trackir_mouse_tracker_config config
) {
    otir_trackir_mouse_step step = {0};
    otir_trackir_mouse_point filtered_centroid;
    otir_trackir_mouse_point previous_raw_centroid = {0};
    otir_trackir_mouse_point raw_jump_delta = {0};
    otir_trackir_mouse_point raw_delta;
    otir_trackir_mouse_point short_average_delta;
    otir_trackir_mouse_point long_average_delta;
    otir_trackir_mouse_point selected_delta;
    otir_trackir_mouse_smoothing_mode smoothing_mode;
    double confidence;
    size_t short_window;
    size_t long_window;

    if (state == NULL) {
        return step;
    }

    if (!config.is_movement_enabled || !has_current_centroid) {
        otir_trackir_mouse_tracker_reset(state);
        return step;
    }

    if (otir_trackir_mouse_latest_centroid(state, &previous_raw_centroid)) {
        raw_jump_delta = (otir_trackir_mouse_point){
            .x = current_centroid.x - previous_raw_centroid.x,
            .y = current_centroid.y - previous_raw_centroid.y,
        };
    }
    otir_trackir_mouse_store_centroid(state, current_centroid);
    filtered_centroid = otir_trackir_mouse_filtered_centroid(state, current_centroid);

    if (!state->has_previous_centroid) {
        state->has_previous_centroid = true;
        state->previous_centroid = filtered_centroid;
        return step;
    }

    raw_delta = (otir_trackir_mouse_point){
        .x = filtered_centroid.x - state->previous_centroid.x,
        .y = filtered_centroid.y - state->previous_centroid.y,
    };
    state->previous_centroid = filtered_centroid;

    if (otir_trackir_mouse_should_skip_jump(
        raw_jump_delta,
        config.avoid_mouse_jumps,
        config.jump_threshold_pixels
    )) {
        return step;
    }

    otir_trackir_mouse_store_delta(state, raw_delta);
    confidence = otir_trackir_mouse_blob_confidence_score(
        selected_blob_area_points,
        selected_blob_brightness_sum,
        config.minimum_blob_area_points
    );
    short_window = otir_trackir_mouse_adjust_smoothing_window_for_confidence(
        otir_trackir_mouse_short_smoothing_window(config.smoothing),
        confidence,
        false
    );
    long_window = otir_trackir_mouse_adjust_smoothing_window_for_confidence(
        otir_trackir_mouse_long_smoothing_window(config.smoothing),
        confidence,
        true
    );
    short_average_delta = otir_trackir_mouse_average_recent_deltas(
        state,
        short_window
    );
    long_average_delta = otir_trackir_mouse_average_recent_deltas(
        state,
        long_window
    );
    smoothing_mode = otir_trackir_mouse_smoothing_mode_for_delta(raw_delta);

    state->is_deadzone_latched = otir_trackir_mouse_update_deadzone_latch(
        state->is_deadzone_latched,
        short_average_delta,
        config.deadzone
    );
    if (state->is_deadzone_latched) {
        return step;
    }

    selected_delta = otir_trackir_mouse_selected_delta(
        smoothing_mode,
        raw_delta,
        short_average_delta,
        long_average_delta
    );
    selected_delta = otir_trackir_mouse_filter_reversal_point(state, selected_delta);
    selected_delta = otir_trackir_mouse_apply_vertical_gain(
        otir_trackir_mouse_transform_delta(
            selected_delta,
            config.speed,
            config.transform
        )
    );

    if (!otir_trackir_mouse_point_is_zero(selected_delta)) {
        step.has_cursor_delta = true;
        step.cursor_delta = selected_delta;
    }

    return step;
}

otir_trackir_mouse_step otir_trackir_mouse_compute_step(
    bool has_previous_centroid,
    otir_trackir_mouse_point previous_centroid,
    bool has_current_centroid,
    otir_trackir_mouse_point current_centroid,
    bool is_movement_enabled,
    double speed,
    otir_trackir_mouse_transform transform
) {
    otir_trackir_mouse_step step = {0};
    otir_trackir_mouse_point raw_delta;
    otir_trackir_mouse_point transformed_delta;

    if (!is_movement_enabled || !has_current_centroid) {
        return step;
    }

    if (!has_previous_centroid) {
        step.has_next_centroid = true;
        step.next_centroid = current_centroid;
        return step;
    }

    raw_delta = (otir_trackir_mouse_point){
        .x = current_centroid.x - previous_centroid.x,
        .y = current_centroid.y - previous_centroid.y,
    };
    transformed_delta = otir_trackir_mouse_apply_vertical_gain(
        otir_trackir_mouse_transform_delta(raw_delta, speed, transform)
    );

    if (!otir_trackir_mouse_point_is_zero(transformed_delta)) {
        step.has_cursor_delta = true;
        step.cursor_delta = transformed_delta;
    }

    step.has_next_centroid = true;
    step.next_centroid = current_centroid;
    return step;
}

static double otir_trackir_mouse_squared_magnitude(otir_trackir_mouse_point point) {
    return (point.x * point.x) + (point.y * point.y);
}

static double otir_trackir_mouse_clamp(double value, double low, double high) {
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

static bool otir_trackir_mouse_latest_centroid(
    const otir_trackir_mouse_tracker_state *state,
    otir_trackir_mouse_point *out_centroid
) {
    size_t latest_index;

    if (state == NULL || out_centroid == NULL || state->centroid_history_count == 0) {
        return false;
    }

    latest_index = (state->centroid_history_write_index + 2) % 3;
    *out_centroid = state->centroid_history[latest_index];
    return true;
}

static void otir_trackir_mouse_store_centroid(
    otir_trackir_mouse_tracker_state *state,
    otir_trackir_mouse_point centroid
) {
    if (state == NULL) {
        return;
    }

    state->centroid_history[state->centroid_history_write_index] = centroid;
    state->centroid_history_write_index = (state->centroid_history_write_index + 1) % 3;
    if (state->centroid_history_count < 3) {
        state->centroid_history_count += 1;
    }
}

static otir_trackir_mouse_point otir_trackir_mouse_filtered_centroid(
    const otir_trackir_mouse_tracker_state *state,
    otir_trackir_mouse_point current_centroid
) {
    otir_trackir_mouse_point left;
    otir_trackir_mouse_point middle;
    otir_trackir_mouse_point right;
    size_t newest_index;
    size_t previous_index;
    size_t oldest_index;

    if (state == NULL || state->centroid_history_count < 3) {
        return current_centroid;
    }

    newest_index = (state->centroid_history_write_index + 2) % 3;
    previous_index = (state->centroid_history_write_index + 1) % 3;
    oldest_index = state->centroid_history_write_index % 3;
    left = state->centroid_history[oldest_index];
    middle = state->centroid_history[previous_index];
    right = state->centroid_history[newest_index];
    return (otir_trackir_mouse_point){
        .x = otir_trackir_mouse_median3(left.x, middle.x, right.x),
        .y = otir_trackir_mouse_median3(left.y, middle.y, right.y),
    };
}

static otir_trackir_mouse_point otir_trackir_mouse_average_recent_deltas(
    const otir_trackir_mouse_tracker_state *state,
    size_t window
) {
    otir_trackir_mouse_point sum = {0};
    size_t count = 0;
    size_t index = 0;
    size_t offset = 0;

    if (state == NULL || state->delta_history_count == 0 || window == 0) {
        return sum;
    }

    count = state->delta_history_count < window ? state->delta_history_count : window;
    for (offset = 0; offset < count; ++offset) {
        index = (state->delta_history_write_index +
            OTIR_TRACKIR_MOUSE_MAX_SMOOTHING_WINDOW - 1 - offset) %
            OTIR_TRACKIR_MOUSE_MAX_SMOOTHING_WINDOW;
        sum.x += state->delta_history[index].x;
        sum.y += state->delta_history[index].y;
    }

    return (otir_trackir_mouse_point){
        .x = sum.x / (double)count,
        .y = sum.y / (double)count,
    };
}

static void otir_trackir_mouse_store_delta(
    otir_trackir_mouse_tracker_state *state,
    otir_trackir_mouse_point delta
) {
    if (state == NULL) {
        return;
    }

    state->delta_history[state->delta_history_write_index] = delta;
    state->delta_history_write_index = (state->delta_history_write_index + 1) %
        OTIR_TRACKIR_MOUSE_MAX_SMOOTHING_WINDOW;
    if (state->delta_history_count < OTIR_TRACKIR_MOUSE_MAX_SMOOTHING_WINDOW) {
        state->delta_history_count += 1;
    }
}

static otir_trackir_mouse_point otir_trackir_mouse_selected_delta(
    otir_trackir_mouse_smoothing_mode mode,
    otir_trackir_mouse_point raw_delta,
    otir_trackir_mouse_point short_average_delta,
    otir_trackir_mouse_point long_average_delta
) {
    switch (mode) {
        case OTIR_TRACKIR_MOUSE_SMOOTHING_MODE_LONG:
            return long_average_delta;
        case OTIR_TRACKIR_MOUSE_SMOOTHING_MODE_SHORT:
            return short_average_delta;
        case OTIR_TRACKIR_MOUSE_SMOOTHING_MODE_RAW:
        default:
            return raw_delta;
    }
}

static otir_trackir_mouse_point otir_trackir_mouse_filter_reversal_point(
    otir_trackir_mouse_tracker_state *state,
    otir_trackir_mouse_point delta
) {
    otir_trackir_mouse_axis_reversal_result x_result;
    otir_trackir_mouse_axis_reversal_result y_result;

    if (state == NULL) {
        return delta;
    }

    x_result = otir_trackir_mouse_filter_reversal_axis(
        delta.x,
        state->x_reversal_state,
        OTIR_TRACKIR_MOUSE_TINY_REVERSAL_THRESHOLD
    );
    y_result = otir_trackir_mouse_filter_reversal_axis(
        delta.y,
        state->y_reversal_state,
        OTIR_TRACKIR_MOUSE_TINY_REVERSAL_THRESHOLD
    );
    state->x_reversal_state = x_result.next_state;
    state->y_reversal_state = y_result.next_state;
    return (otir_trackir_mouse_point){
        .x = x_result.filtered_delta,
        .y = y_result.filtered_delta,
    };
}
