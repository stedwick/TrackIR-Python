#ifndef OPENTRACKIR_TIR5_SESSION_H
#define OPENTRACKIR_TIR5_SESSION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "tir5.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OTIR_TRACKIR_SESSION_FRAME_BYTES (OTIR_TIR5V3_FRAME_WIDTH * OTIR_TIR5V3_FRAME_HEIGHT)
#define OTIR_TRACKIR_SESSION_ERROR_MESSAGE_LENGTH 160

typedef enum otir_trackir_session_phase {
    OTIR_TRACKIR_SESSION_PHASE_IDLE = 0,
    OTIR_TRACKIR_SESSION_PHASE_STARTING = 1,
    OTIR_TRACKIR_SESSION_PHASE_STREAMING = 2,
    OTIR_TRACKIR_SESSION_PHASE_UNAVAILABLE = 3,
    OTIR_TRACKIR_SESSION_PHASE_FAILED = 4
} otir_trackir_session_phase;

typedef enum otir_trackir_session_processing_mode {
    OTIR_TRACKIR_SESSION_PROCESSING_MODE_FULL_TRACKING = 0,
    OTIR_TRACKIR_SESSION_PROCESSING_MODE_REDUCED_TRACKING = 1,
    OTIR_TRACKIR_SESSION_PROCESSING_MODE_PREVIEW_ONLY = 2,
    OTIR_TRACKIR_SESSION_PROCESSING_MODE_LOW_POWER = 3
} otir_trackir_session_processing_mode;

typedef struct otir_trackir_session_source_rate_sample {
    double frame_rate;
    bool has_frame_rate;
    int sampled_frame_count;
    double sample_start_time;
} otir_trackir_session_source_rate_sample;

typedef struct otir_trackir_session_snapshot {
    otir_trackir_session_phase phase;
    otir_status status;
    uint64_t frame_index;
    bool has_frame_rate;
    double frame_rate;
    bool has_centroid;
    double centroid_x;
    double centroid_y;
    bool has_packet_type;
    uint8_t packet_type;
    bool has_preview_frame;
    uint64_t preview_frame_generation;
    uint16_t preview_width;
    uint16_t preview_height;
    bool is_low_power_mode;
    bool has_error_message;
    char error_message[OTIR_TRACKIR_SESSION_ERROR_MESSAGE_LENGTH];
} otir_trackir_session_snapshot;

typedef struct otir_trackir_session otir_trackir_session;

otir_trackir_session *otir_trackir_session_create(void);
void otir_trackir_session_destroy(otir_trackir_session *session);
otir_status otir_trackir_session_start(otir_trackir_session *session);
void otir_trackir_session_stop(otir_trackir_session *session, bool wait_for_shutdown);
void otir_trackir_session_set_maximum_tracking_frames_per_second(
    otir_trackir_session *session,
    double maximum_frames_per_second
);
void otir_trackir_session_set_video_enabled(otir_trackir_session *session, bool enabled);
void otir_trackir_session_set_minimum_blob_area_points(
    otir_trackir_session *session,
    int minimum_blob_area_points
);
void otir_trackir_session_set_scaled_hull_enabled(
    otir_trackir_session *session,
    bool enabled
);
void otir_trackir_session_set_low_power_mode_enabled(
    otir_trackir_session *session,
    bool enabled
);
otir_trackir_session_processing_mode otir_trackir_session_select_processing_mode(
    bool low_power_mode_enabled,
    bool should_process_tracking,
    bool should_publish_preview,
    bool tracking_is_rate_limited
);
otir_trackir_session_source_rate_sample otir_trackir_session_next_source_rate_sample(
    bool low_power_mode_enabled,
    otir_trackir_session_source_rate_sample current_sample,
    double now
);
void otir_trackir_session_copy_snapshot(
    otir_trackir_session *session,
    otir_trackir_session_snapshot *out_snapshot
);
bool otir_trackir_session_copy_preview_frame(
    otir_trackir_session *session,
    uint8_t *out_frame,
    size_t capacity,
    uint64_t *out_generation
);

#ifdef __cplusplus
}
#endif

#endif
