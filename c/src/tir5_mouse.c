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
#define OTIR_TRACKIR_MOUSE_ADAPTIVE_EMA_MIN_ALPHA 0.18
#define OTIR_TRACKIR_MOUSE_ADAPTIVE_EMA_MAX_ALPHA 0.78
#define OTIR_TRACKIR_MOUSE_ADAPTIVE_EMA_FULL_RESPONSE_MAGNITUDE 1.0
#define OTIR_TRACKIR_MOUSE_ALPHA_BETA_ALPHA 0.80
#define OTIR_TRACKIR_MOUSE_ALPHA_BETA_BETA 0.20

static double otir_trackir_mouse_squared_magnitude(otir_trackir_mouse_point point);
static double otir_trackir_mouse_clamp(double value, double minimum, double maximum);
static otir_trackir_mouse_point otir_trackir_mouse_point_add(
    otir_trackir_mouse_point lhs,
    otir_trackir_mouse_point rhs
);
static otir_trackir_mouse_point otir_trackir_mouse_point_subtract(
    otir_trackir_mouse_point lhs,
    otir_trackir_mouse_point rhs
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

double otir_trackir_mouse_adaptive_ema_alpha_for_delta(
    otir_trackir_mouse_point current_delta
) {
    const double magnitude = sqrt(
        otir_trackir_mouse_squared_magnitude(current_delta)
    );
    const double normalized_magnitude = otir_trackir_mouse_clamp(
        magnitude / OTIR_TRACKIR_MOUSE_ADAPTIVE_EMA_FULL_RESPONSE_MAGNITUDE,
        0.0,
        1.0
    );

    return OTIR_TRACKIR_MOUSE_ADAPTIVE_EMA_MIN_ALPHA +
        ((OTIR_TRACKIR_MOUSE_ADAPTIVE_EMA_MAX_ALPHA -
            OTIR_TRACKIR_MOUSE_ADAPTIVE_EMA_MIN_ALPHA) *
            normalized_magnitude);
}

otir_trackir_mouse_point otir_trackir_mouse_apply_adaptive_ema(
    otir_trackir_mouse_point previous_filtered_delta,
    otir_trackir_mouse_point current_delta
) {
    const double alpha = otir_trackir_mouse_adaptive_ema_alpha_for_delta(
        current_delta
    );

    return (otir_trackir_mouse_point){
        .x = previous_filtered_delta.x +
            (alpha * (current_delta.x - previous_filtered_delta.x)),
        .y = previous_filtered_delta.y +
            (alpha * (current_delta.y - previous_filtered_delta.y)),
    };
}

otir_trackir_mouse_alpha_beta_result otir_trackir_mouse_alpha_beta_update(
    otir_trackir_mouse_point previous_position,
    otir_trackir_mouse_point previous_velocity,
    otir_trackir_mouse_point measurement
) {
    const otir_trackir_mouse_point predicted_position =
        otir_trackir_mouse_point_add(previous_position, previous_velocity);
    const otir_trackir_mouse_point residual =
        otir_trackir_mouse_point_subtract(measurement, predicted_position);

    return (otir_trackir_mouse_alpha_beta_result){
        .position = {
            .x = predicted_position.x +
                (OTIR_TRACKIR_MOUSE_ALPHA_BETA_ALPHA * residual.x),
            .y = predicted_position.y +
                (OTIR_TRACKIR_MOUSE_ALPHA_BETA_ALPHA * residual.y),
        },
        .velocity = {
            .x = previous_velocity.x +
                (OTIR_TRACKIR_MOUSE_ALPHA_BETA_BETA * residual.x),
            .y = previous_velocity.y +
                (OTIR_TRACKIR_MOUSE_ALPHA_BETA_BETA * residual.y),
        },
    };
}

otir_trackir_mouse_quantization_result
otir_trackir_mouse_apply_quantization_residual_carry(
    otir_trackir_mouse_point delta,
    otir_trackir_mouse_point residual
) {
    const otir_trackir_mouse_point carried_delta = otir_trackir_mouse_point_add(
        delta,
        residual
    );
    double integral_x = 0.0;
    double integral_y = 0.0;
    const double residual_x = modf(carried_delta.x, &integral_x);
    const double residual_y = modf(carried_delta.y, &integral_y);

    return (otir_trackir_mouse_quantization_result){
        .emitted_delta = {
            .x = integral_x,
            .y = integral_y,
        },
        .residual = {
            .x = residual_x,
            .y = residual_y,
        },
    };
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
    otir_trackir_mouse_tracker_config config
) {
    otir_trackir_mouse_step step = {0};
    otir_trackir_mouse_point raw_delta;
    otir_trackir_mouse_point short_average_delta;
    otir_trackir_mouse_point long_average_delta;
    otir_trackir_mouse_point selected_delta;
    otir_trackir_mouse_point filtered_centroid;
    otir_trackir_mouse_smoothing_mode smoothing_mode;
    bool did_toggle_alpha_beta_filter = false;
    otir_trackir_mouse_alpha_beta_result alpha_beta_result;
    otir_trackir_mouse_quantization_result quantization_result;

    if (state == NULL) {
        return step;
    }

    if (!config.is_movement_enabled || !has_current_centroid) {
        otir_trackir_mouse_tracker_reset(state);
        return step;
    }

    if (state->did_use_alpha_beta_filter != config.use_alpha_beta_filter) {
        did_toggle_alpha_beta_filter = true;
    }
    state->did_use_alpha_beta_filter = config.use_alpha_beta_filter;

    if (!config.use_adaptive_ema) {
        state->has_ema_filtered_delta = false;
    }

    if (!config.use_quantization_residual_carry) {
        state->quantization_residual = (otir_trackir_mouse_point){0};
    }

    if (!config.use_alpha_beta_filter) {
        state->has_alpha_beta_state = false;
        state->alpha_beta_position = (otir_trackir_mouse_point){0};
        state->alpha_beta_velocity = (otir_trackir_mouse_point){0};
    }

    if (did_toggle_alpha_beta_filter) {
        state->has_previous_centroid = true;
        state->previous_centroid = current_centroid;
        state->delta_history_count = 0;
        state->delta_history_write_index = 0;
        state->has_ema_filtered_delta = false;
        state->quantization_residual = (otir_trackir_mouse_point){0};
        state->has_alpha_beta_state = config.use_alpha_beta_filter;
        state->alpha_beta_position = current_centroid;
        state->alpha_beta_velocity = (otir_trackir_mouse_point){0};
        return step;
    }

    filtered_centroid = current_centroid;
    if (config.use_alpha_beta_filter) {
        if (!state->has_alpha_beta_state) {
            state->has_alpha_beta_state = true;
            state->alpha_beta_position = current_centroid;
            state->alpha_beta_velocity = (otir_trackir_mouse_point){0};
        } else {
            alpha_beta_result = otir_trackir_mouse_alpha_beta_update(
                state->alpha_beta_position,
                state->alpha_beta_velocity,
                current_centroid
            );
            state->alpha_beta_position = alpha_beta_result.position;
            state->alpha_beta_velocity = alpha_beta_result.velocity;
        }
        filtered_centroid = state->alpha_beta_position;
    }

    if (!state->has_previous_centroid) {
        state->has_previous_centroid = true;
        state->previous_centroid = filtered_centroid;
        return step;
    }

    raw_delta = otir_trackir_mouse_point_subtract(
        filtered_centroid,
        state->previous_centroid
    );
    state->previous_centroid = filtered_centroid;

    if (otir_trackir_mouse_should_skip_jump(
        raw_delta,
        config.avoid_mouse_jumps,
        config.jump_threshold_pixels
    )) {
        return step;
    }

    otir_trackir_mouse_store_delta(state, raw_delta);
    short_average_delta = otir_trackir_mouse_average_recent_deltas(
        state,
        otir_trackir_mouse_short_smoothing_window(config.smoothing)
    );
    long_average_delta = otir_trackir_mouse_average_recent_deltas(
        state,
        otir_trackir_mouse_long_smoothing_window(config.smoothing)
    );
    smoothing_mode = otir_trackir_mouse_smoothing_mode_for_delta(raw_delta);

    if (otir_trackir_mouse_is_inside_deadzone(short_average_delta, config.deadzone)) {
        return step;
    }

    selected_delta = otir_trackir_mouse_selected_delta(
        smoothing_mode,
        raw_delta,
        short_average_delta,
        long_average_delta
    );

    if (config.use_adaptive_ema) {
        if (!state->has_ema_filtered_delta) {
            state->has_ema_filtered_delta = true;
            state->ema_filtered_delta = selected_delta;
        } else {
            state->ema_filtered_delta = otir_trackir_mouse_apply_adaptive_ema(
                state->ema_filtered_delta,
                selected_delta
            );
        }
        selected_delta = state->ema_filtered_delta;
    }

    selected_delta = otir_trackir_mouse_apply_vertical_gain(
        otir_trackir_mouse_transform_delta(
            selected_delta,
            config.speed,
            config.transform
        )
    );

    if (config.use_quantization_residual_carry) {
        quantization_result = otir_trackir_mouse_apply_quantization_residual_carry(
            selected_delta,
            state->quantization_residual
        );
        state->quantization_residual = quantization_result.residual;
        selected_delta = quantization_result.emitted_delta;
    }

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

static double otir_trackir_mouse_clamp(
    double value,
    double minimum,
    double maximum
) {
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }

    return value;
}

static otir_trackir_mouse_point otir_trackir_mouse_point_add(
    otir_trackir_mouse_point lhs,
    otir_trackir_mouse_point rhs
) {
    return (otir_trackir_mouse_point){
        .x = lhs.x + rhs.x,
        .y = lhs.y + rhs.y,
    };
}

static otir_trackir_mouse_point otir_trackir_mouse_point_subtract(
    otir_trackir_mouse_point lhs,
    otir_trackir_mouse_point rhs
) {
    return (otir_trackir_mouse_point){
        .x = lhs.x - rhs.x,
        .y = lhs.y - rhs.y,
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
