#include "opentrackir/tir5.h"

#include <string.h>

static int clamp_int(int value, int low, int high) {
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

void otir_tir5v3_build_frame(
    const otir_tir5v3_packet *packet,
    uint8_t *frame,
    size_t stride
) {
    size_t row;
    size_t stripe_index;

    if (packet == NULL || frame == NULL || stride < OTIR_TIR5V3_FRAME_WIDTH) {
        return;
    }

    for (row = 0; row < OTIR_TIR5V3_FRAME_HEIGHT; ++row) {
        memset(frame + (row * stride), 0, stride);
    }

    for (stripe_index = 0; stripe_index < packet->stripe_count; ++stripe_index) {
        const otir_tir5v3_stripe *stripe = &packet->stripes[stripe_index];
        int x1;
        int x2;
        int x;
        int brightness;

        if (stripe->points <= 0 || stripe->vline < 0 || stripe->vline >= OTIR_TIR5V3_FRAME_HEIGHT) {
            continue;
        }

        x1 = clamp_int(stripe->hstart, 0, OTIR_TIR5V3_FRAME_WIDTH - 1);
        x2 = clamp_int(stripe->hstop, 0, OTIR_TIR5V3_FRAME_WIDTH - 1);
        if (x2 < x1) {
            continue;
        }

        brightness = (stripe->sum + (stripe->points / 2)) / stripe->points;
        brightness = clamp_int(brightness, 32, 255);
        for (x = x1; x <= x2; ++x) {
            frame[(size_t)stripe->vline * stride + (size_t)x] = (uint8_t)brightness;
        }
    }
}

void otir_tir5v3_packet_stats(
    const otir_tir5v3_packet *packet,
    uint64_t frame_index,
    otir_tir5v3_frame_stats *out_stats
) {
    if (packet == NULL || out_stats == NULL) {
        return;
    }

    memset(out_stats, 0, sizeof(*out_stats));
    out_stats->frame_index = frame_index;
    out_stats->packet_type = packet->packet_type;
    out_stats->stripe_count = packet->stripe_count;
    out_stats->packet_no = packet->packet_no;
    out_stats->has_centroid = otir_tir5v3_compute_weighted_centroid(
        packet->stripes,
        packet->stripe_count,
        &out_stats->centroid_x,
        &out_stats->centroid_y
    );
}
