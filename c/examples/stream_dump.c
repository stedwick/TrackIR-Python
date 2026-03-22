#include <opentrackir/tir5.h>
#include <opentrackir/tir5_tooling.h>

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static volatile sig_atomic_t g_stop_requested = 0;

static void handle_signal(int signal_number) {
    (void)signal_number;
    g_stop_requested = 1;
}

static void fail(otir_status status, const char *message) {
    fprintf(stderr, "%s: %s\n", message, otir_status_string(status));
    exit(1);
}

static void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s [--fps <value>|--fps=<value>]\n", program_name);
}

int main(int argc, char **argv) {
    otir_tir5v3_device *device = NULL;
    otir_tir5v3_status statuses[OTIR_TIR5V3_INIT_STATUS_COUNT];
    otir_tir5v3_device_summary summary;
    size_t status_count = 0;
    uint64_t frame_index = 0;
    double maximum_frames_per_second = 0.0;
    double last_processed_time_seconds = 0.0;
    bool has_last_processed_time = false;
    otir_status status;

    signal(SIGINT, handle_signal);
    status = otir_cli_read_maximum_frames_per_second(
        argc,
        (const char *const *)argv,
        &maximum_frames_per_second
    );
    if (status != OTIR_STATUS_OK) {
        print_usage(argv[0]);
        fail(status, "Invalid FPS argument");
    }

    status = otir_tir5v3_open(&device);
    if (status != OTIR_STATUS_OK) {
        fail(status, "Failed to open TrackIR device");
    }

    status = otir_tir5v3_get_device_summary(device, &summary);
    if (status != OTIR_STATUS_OK) {
        otir_tir5v3_close(device);
        fail(status, "Failed to read device summary");
    }

    printf(
        "device %04x:%04x in=0x%02x out=0x%02x max_packet=%u\n",
        summary.vendor_id,
        summary.product_id,
        summary.endpoint_in,
        summary.endpoint_out,
        summary.max_packet_size
    );

    status = otir_tir5v3_initialize(device, statuses, OTIR_TIR5V3_INIT_STATUS_COUNT, &status_count);
    if (status != OTIR_STATUS_OK) {
        otir_tir5v3_close(device);
        fail(status, "Initialization failed");
    }

    for (size_t index = 0; index < status_count; ++index) {
        printf(
            "status[%zu] stage=0x%02x firmware_loaded=%d flag=0x%02x\n",
            index + 1,
            statuses[index].stage,
            statuses[index].firmware_loaded ? 1 : 0,
            statuses[index].status_flag
        );
    }

    status = otir_tir5v3_start_streaming(device);
    if (status != OTIR_STATUS_OK) {
        otir_tir5v3_close(device);
        fail(status, "Failed to start streaming");
    }

    if (maximum_frames_per_second > 0.0) {
        printf("Streaming at max %.0f fps. Press Ctrl+C to quit.\n", maximum_frames_per_second);
    } else {
        puts("Streaming uncapped. Press Ctrl+C to quit.");
    }
    while (!g_stop_requested) {
        otir_tir5v3_packet packet;
        otir_tir5v3_frame_stats stats;
        double current_time_seconds;

        status = otir_tir5v3_read_packet(device, 50, &packet);
        if (status == OTIR_STATUS_TIMEOUT) {
            continue;
        }
        if (status != OTIR_STATUS_OK) {
            otir_tir5v3_close(device);
            fail(status, "Failed to read packet");
        }
        if (packet.packet_type != 0x00 && packet.packet_type != 0x05) {
            continue;
        }
        current_time_seconds = otir_monotonic_time_seconds();
        if (!otir_tir5v3_should_process_frame(
            current_time_seconds,
            last_processed_time_seconds,
            has_last_processed_time,
            maximum_frames_per_second
        )) {
            continue;
        }

        last_processed_time_seconds = current_time_seconds;
        has_last_processed_time = true;
        frame_index += 1;
        otir_tir5v3_packet_stats(&packet, frame_index, &stats);
        if (stats.has_centroid) {
            printf(
                "frame=%llu packet=%d type=0x%02x stripes=%zu x=%.1f y=%.1f\n",
                (unsigned long long)stats.frame_index,
                stats.packet_no,
                stats.packet_type,
                stats.stripe_count,
                stats.centroid_x,
                stats.centroid_y
            );
        } else {
            printf(
                "frame=%llu packet=%d type=0x%02x stripes=%zu x=- y=-\n",
                (unsigned long long)stats.frame_index,
                stats.packet_no,
                stats.packet_type,
                stats.stripe_count
            );
        }
        fflush(stdout);
    }

    status = otir_tir5v3_stop_streaming(device);
    if (status != OTIR_STATUS_OK) {
        otir_tir5v3_close(device);
        fail(status, "Failed to stop streaming");
    }

    otir_tir5v3_close(device);
    return 0;
}
