#include "opentrackir/tir5.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct otir_tir5v3_blob_accumulator {
    bool is_used;
    long long total_weight;
    long long sum_x;
    long long sum_y;
    int area_points;
    int brightness_sum;
    int min_x;
    int max_x;
    int min_y;
    int max_y;
} otir_tir5v3_blob_accumulator;

typedef struct otir_tir5v3_polygon_point {
    double x;
    double y;
} otir_tir5v3_polygon_point;

typedef struct otir_tir5v3_blob_workspace {
    size_t parents[OTIR_TIR5V3_MAX_STRIPES];
    int blob_for_root[OTIR_TIR5V3_MAX_STRIPES];
    otir_tir5v3_blob_accumulator blobs[OTIR_TIR5V3_MAX_STRIPES];
    otir_tir5v3_polygon_point points[OTIR_TIR5V3_MAX_STRIPES * 4];
    otir_tir5v3_polygon_point hull[OTIR_TIR5V3_MAX_STRIPES * 4];
} otir_tir5v3_blob_workspace;

static size_t blob_root(size_t *parents, size_t index);
static void merge_blob_roots(size_t *parents, size_t left_index, size_t right_index);

static int clamp_int(int value, int low, int high) {
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

static bool stripe_is_trackable(const otir_tir5v3_stripe *stripe) {
    return stripe != NULL && stripe->points > 0;
}

static bool stripe_spans_touch_or_overlap(
    const otir_tir5v3_stripe *left,
    const otir_tir5v3_stripe *right
) {
    if (left == NULL || right == NULL) {
        return false;
    }

    return left->hstart <= right->hstop + 1 &&
        right->hstart <= left->hstop + 1;
}

static void merge_blob_rows(
    const otir_tir5v3_stripe *stripes,
    size_t stripe_count,
    int row_adjacency,
    size_t *parents,
    int *next_in_row
) {
    int row_heads[OTIR_TIR5V3_FRAME_HEIGHT];
    size_t index;
    int row;

    for (row = 0; row < OTIR_TIR5V3_FRAME_HEIGHT; ++row) {
        row_heads[row] = -1;
    }

    for (index = stripe_count; index > 0; --index) {
        const otir_tir5v3_stripe *stripe = &stripes[index - 1];

        if (!stripe_is_trackable(stripe) ||
            stripe->vline < 0 ||
            stripe->vline >= OTIR_TIR5V3_FRAME_HEIGHT) {
            continue;
        }

        next_in_row[index - 1] = row_heads[stripe->vline];
        row_heads[stripe->vline] = (int)(index - 1);
    }

    for (row = 0; row < OTIR_TIR5V3_FRAME_HEIGHT; ++row) {
        int last_compare_row = clamp_int(
            row + row_adjacency,
            row,
            OTIR_TIR5V3_FRAME_HEIGHT - 1
        );
        int left_index = row_heads[row];

        while (left_index >= 0) {
            int compare_row;

            for (compare_row = row; compare_row <= last_compare_row; ++compare_row) {
                int right_index = compare_row == row
                    ? next_in_row[left_index]
                    : row_heads[compare_row];

                while (right_index >= 0) {
                    if (stripe_spans_touch_or_overlap(
                        &stripes[left_index],
                        &stripes[right_index]
                    )) {
                        merge_blob_roots(parents, (size_t)left_index, (size_t)right_index);
                    }
                    right_index = next_in_row[right_index];
                }
            }

            left_index = next_in_row[left_index];
        }
    }
}

static size_t blob_root(size_t *parents, size_t index) {
    while (parents[index] != index) {
        parents[index] = parents[parents[index]];
        index = parents[index];
    }

    return index;
}

static size_t blob_root_read_only(const size_t *parents, size_t index) {
    while (parents[index] != index) {
        index = parents[index];
    }

    return index;
}

static void merge_blob_roots(size_t *parents, size_t left_index, size_t right_index) {
    size_t left_root = blob_root(parents, left_index);
    size_t right_root = blob_root(parents, right_index);

    if (left_root == right_root) {
        return;
    }

    if (left_root < right_root) {
        parents[right_root] = left_root;
    } else {
        parents[left_root] = right_root;
    }
}

static void initialize_blob_accumulator(
    otir_tir5v3_blob_accumulator *blob,
    const otir_tir5v3_stripe *stripe
) {
    blob->is_used = true;
    blob->min_x = stripe->hstart;
    blob->max_x = stripe->hstop;
    blob->min_y = stripe->vline;
    blob->max_y = stripe->vline;
}

static void accumulate_blob_stripe(
    otir_tir5v3_blob_accumulator *blob,
    const otir_tir5v3_stripe *stripe
) {
    if (blob == NULL || stripe == NULL) {
        return;
    }

    blob->area_points += stripe->points;
    if (stripe->sum > 0) {
        blob->brightness_sum += stripe->sum;
        blob->total_weight += stripe->sum;
        blob->sum_x += (long long)stripe->sum * stripe->hstart + stripe->sum_x;
        blob->sum_y += (long long)stripe->sum * stripe->vline;
    }
    if (stripe->hstart < blob->min_x) {
        blob->min_x = stripe->hstart;
    }
    if (stripe->hstop > blob->max_x) {
        blob->max_x = stripe->hstop;
    }
    if (stripe->vline < blob->min_y) {
        blob->min_y = stripe->vline;
    }
    if (stripe->vline > blob->max_y) {
        blob->max_y = stripe->vline;
    }
}

static bool blob_centroid(
    const otir_tir5v3_blob_accumulator *blob,
    double *out_x,
    double *out_y
) {
    if (blob == NULL || out_x == NULL || out_y == NULL || blob->total_weight <= 0) {
        return false;
    }

    *out_x = (double)blob->sum_x / (double)blob->total_weight;
    *out_y = (double)blob->sum_y / (double)blob->total_weight;
    return true;
}

static double squared_distance(
    double left_x,
    double left_y,
    double right_x,
    double right_y
) {
    double delta_x = left_x - right_x;
    double delta_y = left_y - right_y;

    return (delta_x * delta_x) + (delta_y * delta_y);
}

static bool blob_is_better_candidate(
    const otir_tir5v3_blob_accumulator *candidate,
    const otir_tir5v3_blob_accumulator *current_best,
    bool has_previous_centroid,
    double previous_centroid_x,
    double previous_centroid_y
) {
    double candidate_x = 0.0;
    double candidate_y = 0.0;
    double best_x = 0.0;
    double best_y = 0.0;
    double candidate_distance;
    double best_distance;

    if (current_best == NULL) {
        return true;
    }
    if (candidate->area_points != current_best->area_points) {
        return candidate->area_points > current_best->area_points;
    }
    if (candidate->brightness_sum != current_best->brightness_sum) {
        return candidate->brightness_sum > current_best->brightness_sum;
    }
    if (has_previous_centroid &&
        blob_centroid(candidate, &candidate_x, &candidate_y) &&
        blob_centroid(current_best, &best_x, &best_y)) {
        candidate_distance = squared_distance(
            candidate_x,
            candidate_y,
            previous_centroid_x,
            previous_centroid_y
        );
        best_distance = squared_distance(
            best_x,
            best_y,
            previous_centroid_x,
            previous_centroid_y
        );
        if (fabs(candidate_distance - best_distance) > 0.0001) {
            return candidate_distance < best_distance;
        }
    }
    if (candidate->min_x != current_best->min_x) {
        return candidate->min_x < current_best->min_x;
    }
    return candidate->min_y < current_best->min_y;
}

static int compare_polygon_points(const void *left, const void *right) {
    const otir_tir5v3_polygon_point *left_point = left;
    const otir_tir5v3_polygon_point *right_point = right;

    if (left_point->x < right_point->x) {
        return -1;
    }
    if (left_point->x > right_point->x) {
        return 1;
    }
    if (left_point->y < right_point->y) {
        return -1;
    }
    if (left_point->y > right_point->y) {
        return 1;
    }
    return 0;
}

static double polygon_cross(
    otir_tir5v3_polygon_point origin,
    otir_tir5v3_polygon_point left,
    otir_tir5v3_polygon_point right
) {
    return (left.x - origin.x) * (right.y - origin.y) -
        (left.y - origin.y) * (right.x - origin.x);
}

static size_t build_convex_hull(
    otir_tir5v3_polygon_point *points,
    size_t point_count,
    otir_tir5v3_polygon_point *out_hull
) {
    size_t unique_count = 0;
    size_t hull_count = 0;
    size_t lower_count;
    size_t index;

    if (points == NULL || out_hull == NULL || point_count < 3) {
        return 0;
    }

    qsort(points, point_count, sizeof(points[0]), compare_polygon_points);
    for (index = 0; index < point_count; ++index) {
        if (unique_count > 0 &&
            compare_polygon_points(&points[index], &points[unique_count - 1]) == 0) {
            continue;
        }
        points[unique_count] = points[index];
        unique_count += 1;
    }
    if (unique_count < 3) {
        return 0;
    }

    for (index = 0; index < unique_count; ++index) {
        while (hull_count >= 2 &&
            polygon_cross(
                out_hull[hull_count - 2],
                out_hull[hull_count - 1],
                points[index]
            ) <= 0.0) {
            hull_count -= 1;
        }
        out_hull[hull_count] = points[index];
        hull_count += 1;
    }

    lower_count = hull_count + 1;
    for (index = unique_count - 1; index > 0; --index) {
        while (hull_count >= lower_count &&
            polygon_cross(
                out_hull[hull_count - 2],
                out_hull[hull_count - 1],
                points[index - 1]
            ) <= 0.0) {
            hull_count -= 1;
        }
        out_hull[hull_count] = points[index - 1];
        hull_count += 1;
    }

    return hull_count > 1 ? hull_count - 1 : 0;
}

static bool polygon_centroid(
    const otir_tir5v3_polygon_point *polygon,
    size_t point_count,
    double *out_x,
    double *out_y
) {
    double area_sum = 0.0;
    double centroid_x = 0.0;
    double centroid_y = 0.0;
    size_t index;

    if (polygon == NULL || out_x == NULL || out_y == NULL || point_count < 3) {
        return false;
    }

    for (index = 0; index < point_count; ++index) {
        size_t next_index = (index + 1) % point_count;
        double cross = polygon[index].x * polygon[next_index].y -
            polygon[next_index].x * polygon[index].y;

        area_sum += cross;
        centroid_x += (polygon[index].x + polygon[next_index].x) * cross;
        centroid_y += (polygon[index].y + polygon[next_index].y) * cross;
    }

    if (fabs(area_sum) < 0.0001) {
        return false;
    }

    *out_x = centroid_x / (3.0 * area_sum);
    *out_y = centroid_y / (3.0 * area_sum);
    return true;
}

static bool collect_blob_hull_centroid(
    const otir_tir5v3_stripe *stripes,
    size_t stripe_count,
    const size_t *parents,
    const int *blob_for_root,
    size_t selected_blob_index,
    double hull_scale,
    otir_tir5v3_blob_workspace *workspace,
    double *out_x,
    double *out_y
) {
    otir_tir5v3_polygon_point *points;
    otir_tir5v3_polygon_point *hull;
    size_t point_count = 0;
    size_t hull_count;
    size_t index;

    if (stripes == NULL || parents == NULL || blob_for_root == NULL ||
        workspace == NULL || out_x == NULL || out_y == NULL) {
        return false;
    }

    (void)hull_scale;
    points = workspace->points;
    hull = workspace->hull;

    for (index = 0; index < stripe_count; ++index) {
        double left;
        double right;
        double top;
        double bottom;
        size_t root;

        if (!stripe_is_trackable(&stripes[index])) {
            continue;
        }

        root = blob_root_read_only(parents, index);
        if (blob_for_root[root] != (int)selected_blob_index) {
            continue;
        }

        left = stripes[index].hstart - 0.5;
        right = stripes[index].hstop + 0.5;
        top = stripes[index].vline - 0.5;
        bottom = stripes[index].vline + 0.5;
        points[point_count++] = (otir_tir5v3_polygon_point){.x = left, .y = top};
        points[point_count++] = (otir_tir5v3_polygon_point){.x = right, .y = top};
        points[point_count++] = (otir_tir5v3_polygon_point){.x = right, .y = bottom};
        points[point_count++] = (otir_tir5v3_polygon_point){.x = left, .y = bottom};
    }

    hull_count = build_convex_hull(points, point_count, hull);
    return hull_count >= 3 && polygon_centroid(hull, hull_count, out_x, out_y);
}

int otir_tir5v3_normalize_minimum_blob_area_points(int minimum_area_points) {
    return minimum_area_points > 0 ? minimum_area_points : 1;
}

otir_tir5v3_blob_tracking_config otir_tir5v3_default_blob_tracking_config(void) {
    otir_tir5v3_blob_tracking_config config = {
        .minimum_area_points = 100,
        .use_scaled_hull_centroid = true,
        .row_adjacency = 1,
        .hull_scale = 0.7,
    };

    return config;
}

size_t otir_tir5v3_blob_workspace_bytes(void) {
    return sizeof(otir_tir5v3_blob_workspace);
}

bool otir_tir5v3_compute_blob_result_with_workspace(
    const otir_tir5v3_stripe *stripes,
    size_t stripe_count,
    otir_tir5v3_blob_tracking_config config,
    bool has_previous_centroid,
    double previous_centroid_x,
    double previous_centroid_y,
    void *workspace,
    size_t workspace_size,
    otir_tir5v3_blob_result *out_result
) {
    otir_tir5v3_blob_tracking_config default_config = otir_tir5v3_default_blob_tracking_config();
    otir_tir5v3_blob_workspace *blob_workspace = workspace;
    size_t *parents;
    int *blob_for_root;
    otir_tir5v3_blob_accumulator *blobs;
    size_t blob_count = 0;
    size_t selected_blob_index = SIZE_MAX;
    size_t index;

    if (workspace == NULL || workspace_size < sizeof(*blob_workspace) || out_result == NULL) {
        return false;
    }

    parents = blob_workspace->parents;
    blob_for_root = blob_workspace->blob_for_root;
    blobs = blob_workspace->blobs;

    memset(out_result, 0, sizeof(*out_result));
    out_result->centroid_mode = OTIR_TIR5V3_CENTROID_MODE_NONE;

    if (stripes == NULL || stripe_count == 0) {
        return false;
    }

    config.minimum_area_points = config.minimum_area_points > 0
        ? config.minimum_area_points
        : default_config.minimum_area_points;
    config.row_adjacency = config.row_adjacency >= 0
        ? config.row_adjacency
        : default_config.row_adjacency;
    if (!isfinite(config.hull_scale) || config.hull_scale <= 0.0 || config.hull_scale > 1.0) {
        config.hull_scale = default_config.hull_scale;
    }

    memset(blobs, 0, sizeof(*blobs) * stripe_count);
    for (index = 0; index < stripe_count; ++index) {
        parents[index] = index;
        blob_for_root[index] = -1;
    }

    merge_blob_rows(stripes, stripe_count, config.row_adjacency, parents, blob_for_root);

    for (index = 0; index < stripe_count; ++index) {
        blob_for_root[index] = -1;
    }

    for (index = 0; index < stripe_count; ++index) {
        size_t root;
        int blob_index;

        if (!stripe_is_trackable(&stripes[index])) {
            continue;
        }

        root = blob_root(parents, index);
        blob_index = blob_for_root[root];
        if (blob_index < 0) {
            blob_index = (int)blob_count;
            blob_for_root[root] = blob_index;
            initialize_blob_accumulator(&blobs[blob_index], &stripes[index]);
            blob_count += 1;
        }
        accumulate_blob_stripe(&blobs[blob_index], &stripes[index]);
    }

    out_result->blob_count = blob_count;
    for (index = 0; index < blob_count; ++index) {
        if (blobs[index].area_points < config.minimum_area_points || blobs[index].total_weight <= 0) {
            continue;
        }
        if (selected_blob_index == SIZE_MAX ||
            blob_is_better_candidate(
                &blobs[index],
                &blobs[selected_blob_index],
                has_previous_centroid,
                previous_centroid_x,
                previous_centroid_y
            )) {
            selected_blob_index = index;
        }
    }

    if (selected_blob_index != SIZE_MAX &&
        blob_centroid(
            &blobs[selected_blob_index],
            &out_result->centroid_x,
            &out_result->centroid_y
        )) {
        out_result->has_centroid = true;
        out_result->selected_blob_area_points = blobs[selected_blob_index].area_points;
        out_result->selected_blob_brightness_sum = blobs[selected_blob_index].brightness_sum;
        out_result->centroid_mode = OTIR_TIR5V3_CENTROID_MODE_RAW_BLOB;
        if (config.use_scaled_hull_centroid &&
            collect_blob_hull_centroid(
                stripes,
                stripe_count,
                parents,
                blob_for_root,
                selected_blob_index,
                config.hull_scale,
                blob_workspace,
                &out_result->centroid_x,
                &out_result->centroid_y
            )) {
            out_result->centroid_mode = OTIR_TIR5V3_CENTROID_MODE_SCALED_HULL;
        }
    }

    return out_result->has_centroid;
}

bool otir_tir5v3_compute_blob_result(
    const otir_tir5v3_stripe *stripes,
    size_t stripe_count,
    otir_tir5v3_blob_tracking_config config,
    bool has_previous_centroid,
    double previous_centroid_x,
    double previous_centroid_y,
    otir_tir5v3_blob_result *out_result
) {
    otir_tir5v3_blob_workspace workspace;

    return otir_tir5v3_compute_blob_result_with_workspace(
        stripes,
        stripe_count,
        config,
        has_previous_centroid,
        previous_centroid_x,
        previous_centroid_y,
        &workspace,
        sizeof(workspace),
        out_result
    );
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

double otir_tir5v3_normalize_maximum_frames_per_second(double maximum_frames_per_second) {
    if (!isfinite(maximum_frames_per_second) || maximum_frames_per_second <= 0.0) {
        return 0.0;
    }

    return maximum_frames_per_second;
}

bool otir_tir5v3_should_process_frame(
    double current_time_seconds,
    double last_processed_time_seconds,
    bool has_last_processed_time,
    double maximum_frames_per_second
) {
    if (!has_last_processed_time) {
        return true;
    }
    if (otir_tir5v3_normalize_maximum_frames_per_second(maximum_frames_per_second) <= 0.0) {
        return true;
    }

    return otir_tir5v3_should_publish_frame(
        current_time_seconds - last_processed_time_seconds,
        maximum_frames_per_second
    );
}

bool otir_tir5v3_should_publish_frame(
    double elapsed_since_last_frame,
    double maximum_frames_per_second
) {
    double minimum_interval;

    maximum_frames_per_second = otir_tir5v3_normalize_maximum_frames_per_second(
        maximum_frames_per_second
    );
    if (maximum_frames_per_second <= 0.0) {
        return false;
    }

    minimum_interval = 1.0 / maximum_frames_per_second;
    return elapsed_since_last_frame >= minimum_interval;
}

void otir_tir5v3_packet_stats(
    const otir_tir5v3_packet *packet,
    uint64_t frame_index,
    otir_tir5v3_frame_stats *out_stats
) {
    otir_tir5v3_blob_result blob_result;

    if (packet == NULL || out_stats == NULL) {
        return;
    }

    memset(out_stats, 0, sizeof(*out_stats));
    out_stats->frame_index = frame_index;
    out_stats->packet_type = packet->packet_type;
    out_stats->stripe_count = packet->stripe_count;
    out_stats->packet_no = packet->packet_no;
    out_stats->has_centroid = otir_tir5v3_compute_blob_result(
        packet->stripes,
        packet->stripe_count,
        otir_tir5v3_default_blob_tracking_config(),
        false,
        0.0,
        0.0,
        &blob_result
    );
    out_stats->blob_count = blob_result.blob_count;
    out_stats->selected_blob_area_points = blob_result.selected_blob_area_points;
    out_stats->selected_blob_brightness_sum = blob_result.selected_blob_brightness_sum;
    out_stats->centroid_mode = blob_result.centroid_mode;
    out_stats->centroid_x = blob_result.centroid_x;
    out_stats->centroid_y = blob_result.centroid_y;
}
