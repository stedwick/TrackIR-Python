#include "TrackIRMouseBridge.h"

#include <ApplicationServices/ApplicationServices.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>

struct otir_mac_mouse_controller {
    pthread_mutex_t mutex;
    otir_trackir_mouse_tracker_state tracker_state;
    bool can_post_mouse_events;
    bool has_requested_post_event_access;
    CGRect main_display_bounds;
    bool has_cached_display_bounds;
    bool has_registered_display_callback;
    int keep_awake_direction;
};

static bool otir_mac_mouse_controller_refresh_post_event_access_locked(
    otir_mac_mouse_controller *controller
);
static void otir_mac_mouse_controller_refresh_display_bounds_locked(
    otir_mac_mouse_controller *controller
);
static void otir_mac_mouse_display_reconfiguration_callback(
    CGDirectDisplayID display,
    CGDisplayChangeSummaryFlags flags,
    void *userInfo
);
static bool otir_mac_move_cursor_by_delta(
    otir_mac_mouse_controller *controller,
    otir_trackir_mouse_point delta
);

otir_mac_mouse_controller *otir_mac_mouse_controller_create(void) {
    otir_mac_mouse_controller *controller = calloc(1, sizeof(*controller));

    if (controller == NULL) {
        return NULL;
    }
    if (pthread_mutex_init(&controller->mutex, NULL) != 0) {
        free(controller);
        return NULL;
    }

    controller->keep_awake_direction = 1;
    pthread_mutex_lock(&controller->mutex);
    otir_mac_mouse_controller_refresh_display_bounds_locked(controller);
    pthread_mutex_unlock(&controller->mutex);
    if (CGDisplayRegisterReconfigurationCallback(
        otir_mac_mouse_display_reconfiguration_callback,
        controller
    ) == kCGErrorSuccess) {
        controller->has_registered_display_callback = true;
    }

    return controller;
}

void otir_mac_mouse_controller_destroy(otir_mac_mouse_controller *controller) {
    if (controller == NULL) {
        return;
    }

    if (controller->has_registered_display_callback) {
        CGDisplayRemoveReconfigurationCallback(
            otir_mac_mouse_display_reconfiguration_callback,
            controller
        );
    }
    pthread_mutex_destroy(&controller->mutex);
    free(controller);
}

void otir_mac_mouse_controller_reset(otir_mac_mouse_controller *controller) {
    if (controller == NULL) {
        return;
    }

    pthread_mutex_lock(&controller->mutex);
    otir_trackir_mouse_tracker_reset(&controller->tracker_state);
    controller->keep_awake_direction = 1;
    pthread_mutex_unlock(&controller->mutex);
}

void otir_mac_mouse_controller_prepare_post_event_access(
    otir_mac_mouse_controller *controller,
    bool should_request_post_event_access
) {
    if (controller == NULL || !should_request_post_event_access) {
        return;
    }

    pthread_mutex_lock(&controller->mutex);
    (void)otir_mac_mouse_controller_refresh_post_event_access_locked(controller);
    pthread_mutex_unlock(&controller->mutex);
}

bool otir_mac_mouse_controller_update(
    otir_mac_mouse_controller *controller,
    bool has_centroid,
    double centroid_x,
    double centroid_y,
    otir_trackir_mouse_tracker_config config
) {
    otir_trackir_mouse_step step;
    otir_trackir_mouse_point current_centroid = {
        .x = centroid_x,
        .y = centroid_y,
    };
    bool can_post_mouse_events = false;

    if (controller == NULL) {
        return false;
    }

    pthread_mutex_lock(&controller->mutex);
    step = otir_trackir_mouse_tracker_update(
        &controller->tracker_state,
        has_centroid,
        current_centroid,
        config
    );
    can_post_mouse_events = controller->can_post_mouse_events;
    pthread_mutex_unlock(&controller->mutex);

    if (!step.has_cursor_delta || !can_post_mouse_events) {
        return false;
    }

    return otir_mac_move_cursor_by_delta(controller, step.cursor_delta);
}

bool otir_mac_mouse_controller_nudge(otir_mac_mouse_controller *controller) {
    int keep_awake_direction = 1;
    bool can_post_mouse_events = false;

    if (controller == NULL) {
        return false;
    }

    pthread_mutex_lock(&controller->mutex);
    keep_awake_direction = controller->keep_awake_direction == 0
        ? 1
        : controller->keep_awake_direction;
    controller->keep_awake_direction = -keep_awake_direction;
    can_post_mouse_events = controller->can_post_mouse_events;
    pthread_mutex_unlock(&controller->mutex);

    if (!can_post_mouse_events) {
        return false;
    }

    if (otir_mac_move_cursor_by_delta(
        controller,
        (otir_trackir_mouse_point){.x = (double)keep_awake_direction, .y = 0.0}
    )) {
        return true;
    }

    return otir_mac_move_cursor_by_delta(
        controller,
        (otir_trackir_mouse_point){.x = (double)-keep_awake_direction, .y = 0.0}
    );
}

static bool otir_mac_mouse_controller_refresh_post_event_access_locked(
    otir_mac_mouse_controller *controller
) {
    if (CGPreflightPostEventAccess()) {
        controller->can_post_mouse_events = true;
        return true;
    }

    controller->can_post_mouse_events = false;
    if (controller->has_requested_post_event_access) {
        return false;
    }

    controller->has_requested_post_event_access = true;
    controller->can_post_mouse_events = CGRequestPostEventAccess();
    return controller->can_post_mouse_events;
}

static void otir_mac_mouse_controller_refresh_display_bounds_locked(
    otir_mac_mouse_controller *controller
) {
    if (controller == NULL) {
        return;
    }

    controller->main_display_bounds = CGDisplayBounds(CGMainDisplayID());
    controller->has_cached_display_bounds = true;
}

static void otir_mac_mouse_display_reconfiguration_callback(
    CGDirectDisplayID display,
    CGDisplayChangeSummaryFlags flags,
    void *userInfo
) {
    otir_mac_mouse_controller *controller = userInfo;

    (void)display;
    (void)flags;
    if (controller == NULL) {
        return;
    }

    pthread_mutex_lock(&controller->mutex);
    otir_mac_mouse_controller_refresh_display_bounds_locked(controller);
    pthread_mutex_unlock(&controller->mutex);
}

CGPoint otir_mac_quantized_cursor_position(CGPoint position) {
    return CGPointMake(round(position.x), round(position.y));
}

CGPoint otir_mac_clamped_cursor_position_for_bounds(
    CGPoint current_position,
    CGPoint delta,
    CGRect display_bounds
) {
    const CGPoint quantized_current_position = otir_mac_quantized_cursor_position(
        current_position
    );
    const CGPoint quantized_target_position = otir_mac_quantized_cursor_position(
        CGPointMake(
            quantized_current_position.x + delta.x,
            quantized_current_position.y + delta.y
        )
    );

    return CGPointMake(
        fmin(
            fmax(quantized_target_position.x, display_bounds.origin.x),
            CGRectGetMaxX(display_bounds)
        ),
        fmin(
            fmax(quantized_target_position.y, display_bounds.origin.y),
            CGRectGetMaxY(display_bounds)
        )
    );
}

bool otir_mac_should_post_cursor_move(CGPoint current_position, CGPoint next_position) {
    return !CGPointEqualToPoint(
        otir_mac_quantized_cursor_position(current_position),
        otir_mac_quantized_cursor_position(next_position)
    );
}

static bool otir_mac_move_cursor_by_delta(
    otir_mac_mouse_controller *controller,
    otir_trackir_mouse_point delta
) {
    CGEventRef current_event = CGEventCreate(NULL);
    CGEventRef mouse_event = NULL;
    CGPoint current_position;
    CGPoint next_position;
    CGRect display_bounds = CGRectNull;
    bool did_move = false;

    if (current_event == NULL) {
        return false;
    }

    current_position = otir_mac_quantized_cursor_position(
        CGEventGetLocation(current_event)
    );
    pthread_mutex_lock(&controller->mutex);
    if (!controller->has_cached_display_bounds) {
        otir_mac_mouse_controller_refresh_display_bounds_locked(controller);
    }
    display_bounds = controller->main_display_bounds;
    pthread_mutex_unlock(&controller->mutex);
    next_position = otir_mac_clamped_cursor_position_for_bounds(
        current_position,
        CGPointMake(delta.x, delta.y),
        display_bounds
    );
    if (!otir_mac_should_post_cursor_move(current_position, next_position)) {
        CFRelease(current_event);
        return false;
    }
    mouse_event = CGEventCreateMouseEvent(
        NULL,
        kCGEventMouseMoved,
        next_position,
        kCGMouseButtonLeft
    );
    if (mouse_event != NULL) {
        CGEventPost(kCGHIDEventTap, mouse_event);
        did_move = true;
    }

    if (mouse_event != NULL) {
        CFRelease(mouse_event);
    }
    CFRelease(current_event);
    return did_move;
}
