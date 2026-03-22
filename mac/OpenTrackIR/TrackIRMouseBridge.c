#include "TrackIRMouseBridge.h"

#include <ApplicationServices/ApplicationServices.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>

struct otir_mac_mouse_controller {
    pthread_mutex_t mutex;
    bool has_previous_centroid;
    otir_trackir_mouse_point previous_centroid;
    bool can_post_mouse_events;
    bool has_requested_post_event_access;
};

static bool otir_mac_mouse_controller_refresh_post_event_access_locked(
    otir_mac_mouse_controller *controller
);
static void otir_mac_mouse_controller_store_step_locked(
    otir_mac_mouse_controller *controller,
    otir_trackir_mouse_step step
);
static CGPoint otir_mac_clamped_cursor_position(CGPoint current_position, CGPoint delta);
static bool otir_mac_move_cursor_by_delta(otir_trackir_mouse_point delta);

otir_mac_mouse_controller *otir_mac_mouse_controller_create(void) {
    otir_mac_mouse_controller *controller = calloc(1, sizeof(*controller));

    if (controller == NULL) {
        return NULL;
    }
    if (pthread_mutex_init(&controller->mutex, NULL) != 0) {
        free(controller);
        return NULL;
    }

    return controller;
}

void otir_mac_mouse_controller_destroy(otir_mac_mouse_controller *controller) {
    if (controller == NULL) {
        return;
    }

    pthread_mutex_destroy(&controller->mutex);
    free(controller);
}

void otir_mac_mouse_controller_reset(otir_mac_mouse_controller *controller) {
    if (controller == NULL) {
        return;
    }

    pthread_mutex_lock(&controller->mutex);
    controller->has_previous_centroid = false;
    controller->previous_centroid = (otir_trackir_mouse_point){0};
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
    bool is_movement_enabled,
    double speed,
    otir_trackir_mouse_transform transform
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
    step = otir_trackir_mouse_compute_step(
        controller->has_previous_centroid,
        controller->previous_centroid,
        has_centroid,
        current_centroid,
        is_movement_enabled,
        speed,
        transform
    );
    otir_mac_mouse_controller_store_step_locked(controller, step);
    can_post_mouse_events = controller->can_post_mouse_events;
    pthread_mutex_unlock(&controller->mutex);

    if (!step.has_cursor_delta || !can_post_mouse_events) {
        return false;
    }

    return otir_mac_move_cursor_by_delta(step.cursor_delta);
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

static void otir_mac_mouse_controller_store_step_locked(
    otir_mac_mouse_controller *controller,
    otir_trackir_mouse_step step
) {
    if (!step.has_next_centroid) {
        controller->has_previous_centroid = false;
        controller->previous_centroid = (otir_trackir_mouse_point){0};
        return;
    }

    controller->has_previous_centroid = true;
    controller->previous_centroid = step.next_centroid;
}

static CGPoint otir_mac_clamped_cursor_position(CGPoint current_position, CGPoint delta) {
    const CGRect display_bounds = CGDisplayBounds(CGMainDisplayID());
    const CGPoint unclamped_position = CGPointMake(
        current_position.x + delta.x,
        current_position.y + delta.y
    );

    return CGPointMake(
        fmin(fmax(unclamped_position.x, display_bounds.origin.x), CGRectGetMaxX(display_bounds)),
        fmin(fmax(unclamped_position.y, display_bounds.origin.y), CGRectGetMaxY(display_bounds))
    );
}

static bool otir_mac_move_cursor_by_delta(otir_trackir_mouse_point delta) {
    CGEventRef current_event = CGEventCreate(NULL);
    CGEventRef mouse_event = NULL;
    CGPoint next_position;
    bool did_move = false;

    if (current_event == NULL) {
        return false;
    }

    next_position = otir_mac_clamped_cursor_position(
        CGEventGetLocation(current_event),
        CGPointMake(delta.x, delta.y)
    );
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
