#include "internal.h"

uint32_t fr_distance_squared(const fr_node_t *a, const fr_node_t *b) {
    int32_t dx, dy;
    if (a == NULL || b == NULL) return 0;
    dx = (int32_t)a->x - b->x;
    dy = (int32_t)a->y - b->y;
    return (uint32_t)(dx * dx + dy * dy);
}

uint64_t fr_path_measure(const fr_path_t *path) {
    uint64_t total = 0;
    size_t i;
    if (path == NULL || path->points == NULL || path->point_count < 2) return 0;
    for (i = 1; i < path->point_count; ++i) {
        int32_t dx = (int32_t)path->points[i * 2] - path->points[(i - 1) * 2];
        int32_t dy = (int32_t)path->points[i * 2 + 1] - path->points[(i - 1) * 2 + 1];
        total += (uint64_t)(dx * dx + dy * dy);
    }
    return total;
}

static int32_t fr_orient(int16_t ax, int16_t ay, int16_t bx, int16_t by, int16_t cx, int16_t cy) {
    return ((int32_t)bx - ax) * ((int32_t)cy - ay) - ((int32_t)by - ay) * ((int32_t)cx - ax);
}

int fr_geometry_turn_score(const fr_path_t *path) {
    int score = 0;
    size_t i;
    if (path == NULL || path->points == NULL || path->point_count < 3) return 0;
    for (i = 2; i < path->point_count; ++i) {
        int32_t turn = fr_orient(path->points[(i - 2) * 2], path->points[(i - 2) * 2 + 1], path->points[(i - 1) * 2], path->points[(i - 1) * 2 + 1], path->points[i * 2], path->points[i * 2 + 1]);
        if (turn > 0) ++score;
        else if (turn < 0) --score;
    }
    return score;
}

uint32_t fr_geometry_box_area(const fr_path_t *path) {
    int16_t min_x, max_x, min_y, max_y;
    size_t i;
    if (path == NULL || path->points == NULL || path->point_count == 0) return 0;
    min_x = max_x = path->points[0];
    min_y = max_y = path->points[1];
    for (i = 1; i < path->point_count; ++i) {
        int16_t x = path->points[i * 2], y = path->points[i * 2 + 1];
        min_x = FR_MIN(min_x, x); max_x = FR_MAX(max_x, x);
        min_y = FR_MIN(min_y, y); max_y = FR_MAX(max_y, y);
    }
    return (uint32_t)((int32_t)max_x - min_x) * (uint32_t)((int32_t)max_y - min_y);
}
