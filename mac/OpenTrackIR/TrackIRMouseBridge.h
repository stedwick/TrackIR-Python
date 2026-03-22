#ifndef OPENTRACKIR_TRACKIR_MOUSE_BRIDGE_H
#define OPENTRACKIR_TRACKIR_MOUSE_BRIDGE_H

#include <stdbool.h>

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
bool otir_mac_mouse_controller_update(
    otir_mac_mouse_controller *controller,
    bool has_centroid,
    double centroid_x,
    double centroid_y,
    bool is_movement_enabled,
    double speed,
    otir_trackir_mouse_transform transform
);

#ifdef __cplusplus
}
#endif

#endif
