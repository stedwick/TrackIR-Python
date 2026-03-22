#include "opentrackir/tir5_tooling.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#endif

static bool otir_cli_argument_has_prefix(const char *argument, const char *prefix);
static otir_status otir_cli_parse_frames_per_second_value(
    const char *value,
    double *out_maximum_frames_per_second
);

otir_status otir_cli_read_maximum_frames_per_second(
    int argc,
    const char *const *argv,
    double *out_maximum_frames_per_second
) {
    int index;

    if (out_maximum_frames_per_second == NULL) {
        return OTIR_STATUS_INVALID_ARGUMENT;
    }

    *out_maximum_frames_per_second = 0.0;
    for (index = 1; index < argc; ++index) {
        const char *argument = argv[index];

        if (strcmp(argument, "--fps") == 0) {
            if (index + 1 >= argc) {
                return OTIR_STATUS_INVALID_ARGUMENT;
            }

            index += 1;
            return otir_cli_parse_frames_per_second_value(
                argv[index],
                out_maximum_frames_per_second
            );
        }
        if (otir_cli_argument_has_prefix(argument, "--fps=")) {
            return otir_cli_parse_frames_per_second_value(
                argument + strlen("--fps="),
                out_maximum_frames_per_second
            );
        }

        return OTIR_STATUS_INVALID_ARGUMENT;
    }

    return OTIR_STATUS_OK;
}

double otir_monotonic_time_seconds(void) {
#if defined(_WIN32)
    LARGE_INTEGER frequency;
    LARGE_INTEGER counter;

    if (!QueryPerformanceFrequency(&frequency) || !QueryPerformanceCounter(&counter)) {
        return 0.0;
    }

    return (double)counter.QuadPart / (double)frequency.QuadPart;
#else
    struct timespec time_spec;

    if (clock_gettime(CLOCK_MONOTONIC, &time_spec) != 0) {
        return 0.0;
    }

    return (double)time_spec.tv_sec + ((double)time_spec.tv_nsec / 1000000000.0);
#endif
}

static bool otir_cli_argument_has_prefix(const char *argument, const char *prefix) {
    if (argument == NULL || prefix == NULL) {
        return false;
    }

    return strncmp(argument, prefix, strlen(prefix)) == 0;
}

static otir_status otir_cli_parse_frames_per_second_value(
    const char *value,
    double *out_maximum_frames_per_second
) {
    char *end = NULL;
    double parsed_value;

    if (value == NULL || out_maximum_frames_per_second == NULL || value[0] == '\0') {
        return OTIR_STATUS_INVALID_ARGUMENT;
    }

    errno = 0;
    parsed_value = strtod(value, &end);
    if (errno != 0 || end == value || (end != NULL && end[0] != '\0') || parsed_value < 0.0) {
        return OTIR_STATUS_INVALID_ARGUMENT;
    }

    *out_maximum_frames_per_second =
        otir_tir5v3_normalize_maximum_frames_per_second(parsed_value);
    return OTIR_STATUS_OK;
}
