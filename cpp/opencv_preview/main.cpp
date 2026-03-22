#include <opentrackir/tir5.h>
#include <opentrackir/tir5_tooling.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct DeviceCloser {
    otir_tir5v3_device *device = nullptr;

    ~DeviceCloser() {
        if (device != nullptr) {
            otir_tir5v3_close(device);
        }
    }
};

void fail(otir_status status, const std::string &message) {
    std::cerr << message << ": " << otir_status_string(status) << '\n';
    std::exit(1);
}

void print_usage(const char *program_name) {
    std::cerr << "Usage: " << program_name << " [--fps <value>|--fps=<value>]\n";
}

}  // namespace

int main(int argc, char **argv) {
    DeviceCloser handle;
    otir_tir5v3_status statuses[OTIR_TIR5V3_INIT_STATUS_COUNT];
    otir_tir5v3_device_summary summary;
    size_t status_count = 0;
    uint64_t frame_index = 0;
    double maximum_frames_per_second = 0.0;
    double last_processed_time_seconds = 0.0;
    bool has_last_processed_time = false;
    std::vector<uint8_t> frame(OTIR_TIR5V3_FRAME_WIDTH * OTIR_TIR5V3_FRAME_HEIGHT);
    otir_status status = otir_cli_read_maximum_frames_per_second(
        argc,
        const_cast<const char *const *>(argv),
        &maximum_frames_per_second
    );

    if (status != OTIR_STATUS_OK) {
        print_usage(argv[0]);
        fail(status, "Invalid FPS argument");
    }

    status = otir_tir5v3_open(&handle.device);

    if (status != OTIR_STATUS_OK) {
        fail(status, "Failed to open TrackIR device");
    }
    status = otir_tir5v3_get_device_summary(handle.device, &summary);
    if (status != OTIR_STATUS_OK) {
        fail(status, "Failed to read device summary");
    }

    std::cout
        << "device "
        << std::hex << summary.vendor_id << ':' << summary.product_id
        << " in=0x" << static_cast<int>(summary.endpoint_in)
        << " out=0x" << static_cast<int>(summary.endpoint_out)
        << " max_packet=0x" << summary.max_packet_size
        << std::dec << '\n';

    status = otir_tir5v3_initialize(handle.device, statuses, OTIR_TIR5V3_INIT_STATUS_COUNT, &status_count);
    if (status != OTIR_STATUS_OK) {
        fail(status, "Initialization failed");
    }

    for (size_t index = 0; index < status_count; ++index) {
        std::cout
            << "status[" << (index + 1) << "]"
            << " stage=0x" << std::hex << static_cast<int>(statuses[index].stage)
            << " firmware_loaded=" << std::dec << (statuses[index].firmware_loaded ? 1 : 0)
            << " flag=0x" << std::hex << static_cast<int>(statuses[index].status_flag)
            << std::dec << '\n';
    }

    status = otir_tir5v3_start_streaming(handle.device);
    if (status != OTIR_STATUS_OK) {
        fail(status, "Failed to start streaming");
    }

    cv::namedWindow("TrackIR TIR5V3 Preview", cv::WINDOW_NORMAL);
    if (maximum_frames_per_second > 0.0) {
        std::cout << "Controls: q to quit, max " << static_cast<int>(maximum_frames_per_second) << " fps\n";
    } else {
        std::cout << "Controls: q to quit, uncapped\n";
    }

    while (true) {
        otir_tir5v3_packet packet;
        otir_tir5v3_frame_stats stats;
        cv::Mat gray(OTIR_TIR5V3_FRAME_HEIGHT, OTIR_TIR5V3_FRAME_WIDTH, CV_8UC1, frame.data());
        cv::Mat image;
        std::ostringstream overlay;
        double current_time_seconds;

        status = otir_tir5v3_read_packet(handle.device, 50, &packet);
        if (status == OTIR_STATUS_TIMEOUT) {
            const int key = cv::waitKey(1) & 0xFF;
            if (key == 'q') {
                break;
            }
            continue;
        }
        if (status != OTIR_STATUS_OK) {
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
            const int key = cv::waitKey(1) & 0xFF;
            if (key == 'q') {
                break;
            }
            continue;
        }

        last_processed_time_seconds = current_time_seconds;
        has_last_processed_time = true;
        frame_index += 1;
        otir_tir5v3_build_frame(&packet, frame.data(), OTIR_TIR5V3_FRAME_WIDTH);
        otir_tir5v3_packet_stats(&packet, frame_index, &stats);

        cv::cvtColor(gray, image, cv::COLOR_GRAY2BGR);
        if (stats.has_centroid) {
            const int cx = std::clamp(static_cast<int>(std::lround(stats.centroid_x)), 0, OTIR_TIR5V3_FRAME_WIDTH - 1);
            const int cy = std::clamp(static_cast<int>(std::lround(stats.centroid_y)), 0, OTIR_TIR5V3_FRAME_HEIGHT - 1);
            cv::drawMarker(
                image,
                cv::Point(cx, cy),
                cv::Scalar(0, 255, 0),
                cv::MARKER_CROSS,
                12,
                1
            );
        }

        overlay
            << "frame=" << stats.frame_index
            << " type=0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(stats.packet_type)
            << std::dec << " packet=";
        if (stats.packet_no >= 0) {
            overlay << stats.packet_no;
        } else {
            overlay << '-';
        }
        overlay << " stripes=" << stats.stripe_count;
        if (stats.has_centroid) {
            overlay
                << std::fixed << std::setprecision(1)
                << " x=" << stats.centroid_x
                << " y=" << stats.centroid_y;
        } else {
            overlay << " x=- y=-";
        }

        cv::putText(
            image,
            overlay.str(),
            cv::Point(10, 22),
            cv::FONT_HERSHEY_SIMPLEX,
            0.55,
            cv::Scalar(255, 255, 255),
            1,
            cv::LINE_AA
        );
        cv::imshow("TrackIR TIR5V3 Preview", image);

        const int key = cv::waitKey(1) & 0xFF;
        if (key == 'q') {
            break;
        }
    }

    status = otir_tir5v3_stop_streaming(handle.device);
    if (status != OTIR_STATUS_OK) {
        fail(status, "Failed to stop streaming");
    }
    return 0;
}
