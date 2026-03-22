#ifndef OPENTRACKIR_TIR5_TOOLING_H
#define OPENTRACKIR_TIR5_TOOLING_H

#include <stdbool.h>

#include "tir5.h"

#ifdef __cplusplus
extern "C" {
#endif

otir_status otir_cli_read_maximum_frames_per_second(
    int argc,
    const char *const *argv,
    double *out_maximum_frames_per_second
);
double otir_monotonic_time_seconds(void);

#ifdef __cplusplus
}
#endif

#endif
