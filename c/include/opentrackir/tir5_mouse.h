#ifndef OPENTRACKIR_TIR5_MOUSE_H
#define OPENTRACKIR_TIR5_MOUSE_H

#include <stdbool.h>

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
bool otir_trackir_mouse_point_is_zero(otir_trackir_mouse_point point);
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
