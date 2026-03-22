#ifndef OPENTRACKIR_TRACKIR_MOUSE_BRIDGE_H
#define OPENTRACKIR_TRACKIR_MOUSE_BRIDGE_H

#include <stdbool.h>
#include <CoreGraphics/CoreGraphics.h>

#include "../../c/include/opentrackir/tir5_mouse.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct otir_mac_mouse_controller otir_mac_mouse_controller;

otir_mac_mouse_controller *otir_mac_mouse_controller_create(void);
void otir_mac_mouse_controller_destroy(otir_mac_mouse_controller *controller);
void otir_mac_mouse_controller_reset(otir_mac_mouse_controller *controller);
void otir_mac_mouse_controller_prepare_post_event_access(
    otir_mac_mouse_controller *controller,
    bool should_request_post_event_access
);
CGPoint otir_mac_quantized_cursor_position(CGPoint position);
CGPoint otir_mac_clamped_cursor_position_for_bounds(
    CGPoint current_position,
    CGPoint delta,
    CGRect display_bounds
);
bool otir_mac_should_post_cursor_move(CGPoint current_position, CGPoint next_position);
bool otir_mac_mouse_controller_update(
    otir_mac_mouse_controller *controller,
    bool has_centroid,
    double centroid_x,
    double centroid_y,
    int selected_blob_area_points,
    int selected_blob_brightness_sum,
    otir_trackir_mouse_tracker_config config
);
bool otir_mac_mouse_controller_nudge(otir_mac_mouse_controller *controller);

#ifdef __cplusplus
}
#endif

#endif
