#include "opentrackir/tir5_session.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define OTIR_TRACKIR_SESSION_MAX_PREVIEW_FRAMES_PER_SECOND 30.0
#define OTIR_TRACKIR_SESSION_LOW_POWER_FRAMES_PER_SECOND 2.0

typedef struct trackir_session_runtime_config {
    bool video_enabled;
    double maximum_tracking_frames_per_second;
    int minimum_blob_area_points;
    bool scaled_hull_enabled;
    bool low_power_mode_enabled;
} trackir_session_runtime_config;

struct otir_trackir_session {
    pthread_mutex_t mutex;
    pthread_t worker;
    bool worker_started;
    bool worker_exited;
    bool stop_requested;
    bool video_enabled;
    double maximum_tracking_frames_per_second;
    int minimum_blob_area_points;
    bool scaled_hull_enabled;
    bool low_power_mode_enabled;
    otir_trackir_session_snapshot snapshot;
    uint8_t preview_frame[OTIR_TRACKIR_SESSION_FRAME_BYTES];
};

static void *trackir_session_worker_main(void *context);
static void trackir_session_reset_snapshot_locked(otir_trackir_session *session);
static void trackir_session_clear_preview_locked(otir_trackir_session *session);
static void trackir_session_set_error_locked(otir_trackir_session *session, const char *message);
static void trackir_session_set_failure_locked(
    otir_trackir_session *session,
    otir_status status,
    const char *operation
);
static void trackir_session_join_worker_if_exited(otir_trackir_session *session);
static bool trackir_session_stop_requested(otir_trackir_session *session);
static trackir_session_runtime_config trackir_session_runtime_config_snapshot(
    otir_trackir_session *session
);
static void trackir_copy_string(char *destination, size_t capacity, const char *source);
static void trackir_session_format_failure_message(
    otir_status status,
    const char *operation,
    char *buffer,
    size_t capacity
);
static double trackir_session_now_seconds(void);

otir_trackir_session *otir_trackir_session_create(void) {
    otir_trackir_session *session = calloc(1, sizeof(*session));

    if (session == NULL) {
        return NULL;
    }
    if (pthread_mutex_init(&session->mutex, NULL) != 0) {
        free(session);
        return NULL;
    }

    pthread_mutex_lock(&session->mutex);
    session->video_enabled = true;
    session->maximum_tracking_frames_per_second =
        otir_tir5v3_normalize_maximum_frames_per_second(0.0);
    session->minimum_blob_area_points =
        otir_tir5v3_default_blob_tracking_config().minimum_area_points;
    session->scaled_hull_enabled =
        otir_tir5v3_default_blob_tracking_config().use_scaled_hull_centroid;
    session->low_power_mode_enabled = false;
    trackir_session_reset_snapshot_locked(session);
    pthread_mutex_unlock(&session->mutex);
    return session;
}

void otir_trackir_session_destroy(otir_trackir_session *session) {
    if (session == NULL) {
        return;
    }

    otir_trackir_session_stop(session, true);
    pthread_mutex_destroy(&session->mutex);
    free(session);
}

otir_status otir_trackir_session_start(otir_trackir_session *session) {
    if (session == NULL) {
        return OTIR_STATUS_INVALID_ARGUMENT;
    }

    trackir_session_join_worker_if_exited(session);

    pthread_mutex_lock(&session->mutex);

    if (session->worker_started) {
        pthread_mutex_unlock(&session->mutex);
        return OTIR_STATUS_OK;
    }

    trackir_session_reset_snapshot_locked(session);
    session->snapshot.phase = OTIR_TRACKIR_SESSION_PHASE_STARTING;
    session->stop_requested = false;
    session->worker_exited = false;

    if (pthread_create(&session->worker, NULL, trackir_session_worker_main, session) != 0) {
        trackir_session_set_failure_locked(session, OTIR_STATUS_IO, "Start session");
        pthread_mutex_unlock(&session->mutex);
        return OTIR_STATUS_IO;
    }

    session->worker_started = true;
    pthread_mutex_unlock(&session->mutex);
    return OTIR_STATUS_OK;
}

void otir_trackir_session_stop(otir_trackir_session *session, bool wait_for_shutdown) {
    if (session == NULL) {
        return;
    }

    pthread_t worker = (pthread_t){0};
    bool should_join = false;

    pthread_mutex_lock(&session->mutex);
    if (session->worker_started) {
        session->stop_requested = true;
        worker = session->worker;
        should_join = wait_for_shutdown;
    }
    pthread_mutex_unlock(&session->mutex);

    if (should_join) {
        pthread_join(worker, NULL);

        pthread_mutex_lock(&session->mutex);
        session->worker_started = false;
        session->worker_exited = false;
        trackir_session_reset_snapshot_locked(session);
        pthread_mutex_unlock(&session->mutex);
        return;
    }

    trackir_session_join_worker_if_exited(session);

    pthread_mutex_lock(&session->mutex);
    if (!session->worker_started) {
        trackir_session_reset_snapshot_locked(session);
    }
    pthread_mutex_unlock(&session->mutex);
}

void otir_trackir_session_set_video_enabled(otir_trackir_session *session, bool enabled) {
    if (session == NULL) {
        return;
    }

    pthread_mutex_lock(&session->mutex);
    session->video_enabled = enabled;
    if (!enabled) {
        trackir_session_clear_preview_locked(session);
    }
    pthread_mutex_unlock(&session->mutex);
}

void otir_trackir_session_set_maximum_tracking_frames_per_second(
    otir_trackir_session *session,
    double maximum_frames_per_second
) {
    if (session == NULL) {
        return;
    }

    pthread_mutex_lock(&session->mutex);
    session->maximum_tracking_frames_per_second =
        otir_tir5v3_normalize_maximum_frames_per_second(maximum_frames_per_second);
    pthread_mutex_unlock(&session->mutex);
}

void otir_trackir_session_set_minimum_blob_area_points(
    otir_trackir_session *session,
    int minimum_blob_area_points
) {
    if (session == NULL) {
        return;
    }

    pthread_mutex_lock(&session->mutex);
    session->minimum_blob_area_points =
        otir_tir5v3_normalize_minimum_blob_area_points(minimum_blob_area_points);
    pthread_mutex_unlock(&session->mutex);
}

void otir_trackir_session_set_scaled_hull_enabled(
    otir_trackir_session *session,
    bool enabled
) {
    if (session == NULL) {
        return;
    }

    pthread_mutex_lock(&session->mutex);
    session->scaled_hull_enabled = enabled;
    pthread_mutex_unlock(&session->mutex);
}

void otir_trackir_session_set_low_power_mode_enabled(
    otir_trackir_session *session,
    bool enabled
) {
    if (session == NULL) {
        return;
    }

    pthread_mutex_lock(&session->mutex);
    session->low_power_mode_enabled = enabled;
    session->snapshot.is_low_power_mode = enabled;
    pthread_mutex_unlock(&session->mutex);
}

otir_trackir_session_processing_mode otir_trackir_session_select_processing_mode(
    bool low_power_mode_enabled,
    bool should_process_tracking,
    bool should_publish_preview,
    bool tracking_is_rate_limited
) {
    if (low_power_mode_enabled) {
        return OTIR_TRACKIR_SESSION_PROCESSING_MODE_LOW_POWER;
    }
    if (!should_process_tracking && should_publish_preview) {
        return OTIR_TRACKIR_SESSION_PROCESSING_MODE_PREVIEW_ONLY;
    }
    if (tracking_is_rate_limited) {
        return OTIR_TRACKIR_SESSION_PROCESSING_MODE_REDUCED_TRACKING;
    }
    return OTIR_TRACKIR_SESSION_PROCESSING_MODE_FULL_TRACKING;
}

void otir_trackir_session_copy_snapshot(
    otir_trackir_session *session,
    otir_trackir_session_snapshot *out_snapshot
) {
    if (session == NULL || out_snapshot == NULL) {
        return;
    }

    trackir_session_join_worker_if_exited(session);

    pthread_mutex_lock(&session->mutex);
    memcpy(out_snapshot, &session->snapshot, sizeof(*out_snapshot));
    pthread_mutex_unlock(&session->mutex);
}

bool otir_trackir_session_copy_preview_frame(
    otir_trackir_session *session,
    uint8_t *out_frame,
    size_t capacity,
    uint64_t *out_generation
) {
    if (session == NULL || out_frame == NULL || capacity < OTIR_TRACKIR_SESSION_FRAME_BYTES) {
        return false;
    }

    pthread_mutex_lock(&session->mutex);
    if (!session->snapshot.has_preview_frame) {
        pthread_mutex_unlock(&session->mutex);
        return false;
    }

    memcpy(out_frame, session->preview_frame, OTIR_TRACKIR_SESSION_FRAME_BYTES);
    if (out_generation != NULL) {
        *out_generation = session->snapshot.preview_frame_generation;
    }
    pthread_mutex_unlock(&session->mutex);
    return true;
}

static void *trackir_session_worker_main(void *context) {
    otir_trackir_session *session = context;
    otir_tir5v3_device *device = NULL;
    void *blob_workspace = malloc(otir_tir5v3_blob_workspace_bytes());
    otir_tir5v3_packet *packet = malloc(sizeof(*packet));
    otir_status status;
    uint64_t frame_index = 0;
    double measured_source_frame_rate = 0.0;
    bool has_source_frame_rate = false;
    int sampled_source_frame_count = 0;
    double source_sample_start_time = trackir_session_now_seconds();
    double last_tracking_process_time = 0.0;
    bool has_tracking_process_time = false;
    double last_preview_publish_time = 0.0;
    bool has_preview_publish_time = false;
    bool has_previous_selected_centroid = false;
    double previous_selected_centroid_x = 0.0;
    double previous_selected_centroid_y = 0.0;
    uint8_t *frame_buffer = malloc(OTIR_TRACKIR_SESSION_FRAME_BYTES);

    if (blob_workspace == NULL || packet == NULL || frame_buffer == NULL) {
        pthread_mutex_lock(&session->mutex);
        trackir_session_set_failure_locked(session, OTIR_STATUS_IO, "Allocate buffers");
        session->worker_exited = true;
        pthread_mutex_unlock(&session->mutex);
        free(blob_workspace);
        free(packet);
        free(frame_buffer);
        return NULL;
    }

    status = otir_tir5v3_open(&device);
    if (status != OTIR_STATUS_OK || device == NULL) {
        pthread_mutex_lock(&session->mutex);
        trackir_session_set_failure_locked(session, status, "Open");
        session->worker_exited = true;
        pthread_mutex_unlock(&session->mutex);
        free(blob_workspace);
        free(packet);
        free(frame_buffer);
        return NULL;
    }

    pthread_mutex_lock(&session->mutex);
    session->snapshot.phase = OTIR_TRACKIR_SESSION_PHASE_STARTING;
    pthread_mutex_unlock(&session->mutex);

    do {
        otir_tir5v3_status init_statuses[OTIR_TIR5V3_INIT_STATUS_COUNT];
        size_t init_status_count = 0;

        status = otir_tir5v3_initialize(
            device,
            init_statuses,
            OTIR_TIR5V3_INIT_STATUS_COUNT,
            &init_status_count
        );
        if (status != OTIR_STATUS_OK) {
            break;
        }

        status = otir_tir5v3_start_streaming(device);
        if (status != OTIR_STATUS_OK) {
            break;
        }

        pthread_mutex_lock(&session->mutex);
        session->snapshot.phase = OTIR_TRACKIR_SESSION_PHASE_STREAMING;
        session->snapshot.status = OTIR_STATUS_OK;
        session->snapshot.has_error_message = false;
        pthread_mutex_unlock(&session->mutex);

        while (!trackir_session_stop_requested(session)) {
            otir_tir5v3_blob_result blob_result;
            otir_tir5v3_blob_tracking_config blob_config = otir_tir5v3_default_blob_tracking_config();
            trackir_session_runtime_config runtime_config;
            otir_trackir_session_processing_mode processing_mode;
            double centroid_x = 0.0;
            double centroid_y = 0.0;
            double effective_tracking_frames_per_second = 0.0;
            bool has_centroid = false;
            bool has_previous_blob_centroid = false;
            bool should_process_tracking = false;
            bool should_publish_preview = false;
            bool tracking_is_rate_limited = false;
            double now;
            double elapsed_since_last_tracking_process = 0.0;
            double elapsed_since_last_publish = 0.0;

            status = otir_tir5v3_read_packet(device, 50, packet);

            if (status == OTIR_STATUS_TIMEOUT) {
                continue;
            }
            if (status != OTIR_STATUS_OK) {
                break;
            }
            if (packet->packet_type != 0x00 && packet->packet_type != 0x05) {
                continue;
            }

            sampled_source_frame_count += 1;
            now = trackir_session_now_seconds();

            if (now - source_sample_start_time >= 0.25) {
                measured_source_frame_rate =
                    (double)sampled_source_frame_count / (now - source_sample_start_time);
                has_source_frame_rate = true;
                sampled_source_frame_count = 0;
                source_sample_start_time = now;
            }

            if (has_tracking_process_time) {
                elapsed_since_last_tracking_process = now - last_tracking_process_time;
            }
            runtime_config = trackir_session_runtime_config_snapshot(session);
            effective_tracking_frames_per_second = runtime_config.low_power_mode_enabled
                ? OTIR_TRACKIR_SESSION_LOW_POWER_FRAMES_PER_SECOND
                : runtime_config.maximum_tracking_frames_per_second;
            should_process_tracking = !has_tracking_process_time ||
                effective_tracking_frames_per_second <= 0.0 ||
                otir_tir5v3_should_publish_frame(
                    elapsed_since_last_tracking_process,
                    effective_tracking_frames_per_second
                );
            if (runtime_config.video_enabled) {
                if (has_preview_publish_time) {
                    elapsed_since_last_publish = now - last_preview_publish_time;
                }
                should_publish_preview = !has_preview_publish_time ||
                    otir_tir5v3_should_publish_frame(
                        elapsed_since_last_publish,
                        OTIR_TRACKIR_SESSION_MAX_PREVIEW_FRAMES_PER_SECOND
                    );
            }
            if (!should_process_tracking && !should_publish_preview) {
                continue;
            }

            tracking_is_rate_limited = !runtime_config.low_power_mode_enabled &&
                effective_tracking_frames_per_second > 0.0 &&
                has_source_frame_rate &&
                effective_tracking_frames_per_second < measured_source_frame_rate;
            processing_mode = otir_trackir_session_select_processing_mode(
                runtime_config.low_power_mode_enabled,
                should_process_tracking,
                should_publish_preview,
                tracking_is_rate_limited
            );

            if (processing_mode != OTIR_TRACKIR_SESSION_PROCESSING_MODE_PREVIEW_ONLY) {
                frame_index += 1;
                if (processing_mode != OTIR_TRACKIR_SESSION_PROCESSING_MODE_LOW_POWER) {
                    blob_config.minimum_area_points = runtime_config.minimum_blob_area_points;
                    blob_config.use_scaled_hull_centroid = runtime_config.scaled_hull_enabled;
                    has_previous_blob_centroid = has_previous_selected_centroid;
                    blob_result = (otir_tir5v3_blob_result){0};
                    if (otir_tir5v3_compute_blob_result_with_workspace(
                        packet->stripes,
                        packet->stripe_count,
                        blob_config,
                        has_previous_blob_centroid,
                        previous_selected_centroid_x,
                        previous_selected_centroid_y,
                        blob_workspace,
                        otir_tir5v3_blob_workspace_bytes(),
                        &blob_result
                    )) {
                        has_previous_selected_centroid = true;
                        previous_selected_centroid_x = blob_result.centroid_x;
                        previous_selected_centroid_y = blob_result.centroid_y;
                    } else {
                        has_previous_selected_centroid = false;
                    }
                    if (blob_result.has_centroid) {
                        centroid_x = blob_result.centroid_x;
                        centroid_y = blob_result.centroid_y;
                        has_centroid = true;
                    }
                }

                pthread_mutex_lock(&session->mutex);
                session->snapshot.phase = OTIR_TRACKIR_SESSION_PHASE_STREAMING;
                session->snapshot.status = OTIR_STATUS_OK;
                session->snapshot.frame_index = frame_index;
                session->snapshot.has_frame_rate = has_source_frame_rate;
                session->snapshot.frame_rate = measured_source_frame_rate;
                session->snapshot.has_centroid = has_centroid;
                session->snapshot.centroid_x = centroid_x;
                session->snapshot.centroid_y = centroid_y;
                session->snapshot.has_packet_type = true;
                session->snapshot.packet_type = packet->packet_type;
                session->snapshot.is_low_power_mode =
                    processing_mode == OTIR_TRACKIR_SESSION_PROCESSING_MODE_LOW_POWER;
                pthread_mutex_unlock(&session->mutex);

                last_tracking_process_time = now;
                has_tracking_process_time = true;
            } else {
                pthread_mutex_lock(&session->mutex);
                session->snapshot.phase = OTIR_TRACKIR_SESSION_PHASE_STREAMING;
                session->snapshot.status = OTIR_STATUS_OK;
                session->snapshot.has_frame_rate = has_source_frame_rate;
                session->snapshot.frame_rate = measured_source_frame_rate;
                session->snapshot.has_packet_type = true;
                session->snapshot.packet_type = packet->packet_type;
                session->snapshot.is_low_power_mode = false;
                pthread_mutex_unlock(&session->mutex);
            }

            if (!should_publish_preview) {
                continue;
            }

            otir_tir5v3_build_frame(packet, frame_buffer, OTIR_TIR5V3_FRAME_WIDTH);

            pthread_mutex_lock(&session->mutex);
            memcpy(session->preview_frame, frame_buffer, OTIR_TRACKIR_SESSION_FRAME_BYTES);
            session->snapshot.has_preview_frame = true;
            session->snapshot.preview_width = OTIR_TIR5V3_FRAME_WIDTH;
            session->snapshot.preview_height = OTIR_TIR5V3_FRAME_HEIGHT;
            session->snapshot.preview_frame_generation += 1;
            pthread_mutex_unlock(&session->mutex);

            last_preview_publish_time = now;
            has_preview_publish_time = true;
        }
    } while (0);

    if (status != OTIR_STATUS_OK && !trackir_session_stop_requested(session)) {
        pthread_mutex_lock(&session->mutex);
        trackir_session_set_failure_locked(session, status, "TrackIR");
        pthread_mutex_unlock(&session->mutex);
    }

    (void)otir_tir5v3_stop_streaming(device);
    otir_tir5v3_close(device);
    free(blob_workspace);
    free(packet);
    free(frame_buffer);

    pthread_mutex_lock(&session->mutex);
    if (session->stop_requested) {
        trackir_session_reset_snapshot_locked(session);
    }
    session->worker_exited = true;
    pthread_mutex_unlock(&session->mutex);
    return NULL;
}

static void trackir_session_reset_snapshot_locked(otir_trackir_session *session) {
    memset(&session->snapshot, 0, sizeof(session->snapshot));
    session->snapshot.phase = OTIR_TRACKIR_SESSION_PHASE_IDLE;
    session->snapshot.status = OTIR_STATUS_OK;
    session->snapshot.is_low_power_mode = false;
}

static void trackir_session_clear_preview_locked(otir_trackir_session *session) {
    session->snapshot.has_preview_frame = false;
    session->snapshot.preview_width = 0;
    session->snapshot.preview_height = 0;
}

static void trackir_session_set_error_locked(otir_trackir_session *session, const char *message) {
    if (message == NULL || message[0] == '\0') {
        session->snapshot.has_error_message = false;
        session->snapshot.error_message[0] = '\0';
        return;
    }

    session->snapshot.has_error_message = true;
    trackir_copy_string(
        session->snapshot.error_message,
        sizeof(session->snapshot.error_message),
        message
    );
}

static void trackir_session_set_failure_locked(
    otir_trackir_session *session,
    otir_status status,
    const char *operation
) {
    char message[OTIR_TRACKIR_SESSION_ERROR_MESSAGE_LENGTH];

    trackir_session_format_failure_message(status, operation, message, sizeof(message));
    trackir_session_reset_snapshot_locked(session);
    session->snapshot.status = status;
    session->snapshot.phase = status == OTIR_STATUS_NOT_FOUND
        ? OTIR_TRACKIR_SESSION_PHASE_UNAVAILABLE
        : OTIR_TRACKIR_SESSION_PHASE_FAILED;
    trackir_session_set_error_locked(session, message);
}

static void trackir_session_join_worker_if_exited(otir_trackir_session *session) {
    pthread_t worker = (pthread_t){0};
    bool should_join = false;

    pthread_mutex_lock(&session->mutex);
    if (session->worker_started && session->worker_exited) {
        worker = session->worker;
        should_join = true;
    }
    pthread_mutex_unlock(&session->mutex);

    if (!should_join) {
        return;
    }

    pthread_join(worker, NULL);

    pthread_mutex_lock(&session->mutex);
    session->worker_started = false;
    session->worker_exited = false;
    session->stop_requested = false;
    pthread_mutex_unlock(&session->mutex);
}

static bool trackir_session_stop_requested(otir_trackir_session *session) {
    bool stop_requested;

    pthread_mutex_lock(&session->mutex);
    stop_requested = session->stop_requested;
    pthread_mutex_unlock(&session->mutex);
    return stop_requested;
}

static trackir_session_runtime_config trackir_session_runtime_config_snapshot(
    otir_trackir_session *session
) {
    trackir_session_runtime_config runtime_config;

    pthread_mutex_lock(&session->mutex);
    runtime_config.video_enabled = session->video_enabled;
    runtime_config.maximum_tracking_frames_per_second =
        session->maximum_tracking_frames_per_second;
    runtime_config.minimum_blob_area_points = session->minimum_blob_area_points;
    runtime_config.scaled_hull_enabled = session->scaled_hull_enabled;
    runtime_config.low_power_mode_enabled = session->low_power_mode_enabled;
    pthread_mutex_unlock(&session->mutex);
    return runtime_config;
}

static void trackir_copy_string(char *destination, size_t capacity, const char *source) {
    if (destination == NULL || capacity == 0) {
        return;
    }
    if (source == NULL) {
        destination[0] = '\0';
        return;
    }

    strncpy(destination, source, capacity - 1);
    destination[capacity - 1] = '\0';
}

static void trackir_session_format_failure_message(
    otir_status status,
    const char *operation,
    char *buffer,
    size_t capacity
) {
    const char *status_description;

    if (buffer == NULL || capacity == 0) {
        return;
    }
    if (status == OTIR_STATUS_NOT_FOUND) {
        trackir_copy_string(
            buffer,
            capacity,
            "TrackIR not found. Connect the device and try again."
        );
        return;
    }

    status_description = otir_status_string(status);
    if (operation == NULL || operation[0] == '\0') {
        snprintf(buffer, capacity, "%s.", status_description);
        return;
    }

    snprintf(buffer, capacity, "%s failed: %s.", operation, status_description);
}

static double trackir_session_now_seconds(void) {
    struct timespec time_spec;

    if (clock_gettime(CLOCK_MONOTONIC, &time_spec) != 0) {
        return 0.0;
    }

    return (double)time_spec.tv_sec + ((double)time_spec.tv_nsec / 1000000000.0);
}
