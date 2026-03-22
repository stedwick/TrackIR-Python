#include "opentrackir/tir5_mouse.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

bool otir_trackir_mouse_point_is_zero(otir_trackir_mouse_point point) {
    return point.x == 0.0 && point.y == 0.0;
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
    transformed_delta = otir_trackir_mouse_transform_delta(raw_delta, speed, transform);

    if (!otir_trackir_mouse_point_is_zero(transformed_delta)) {
        step.has_cursor_delta = true;
        step.cursor_delta = transformed_delta;
    }

    step.has_next_centroid = true;
    step.next_centroid = current_centroid;
    return step;
}
