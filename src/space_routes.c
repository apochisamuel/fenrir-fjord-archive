#include "internal.h"

uint32_t fr_space_routes_node_extent(const fr_document_t *doc) {
    int16_t min_x, max_x, min_y, max_y;
    size_t i;
    if (doc == NULL || doc->node_count == 0) return 0;
    min_x = max_x = doc->nodes[0].x; min_y = max_y = doc->nodes[0].y;
    for (i = 1; i < doc->node_count; ++i) { const fr_node_t *node = &doc->nodes[i]; min_x = FR_MIN(min_x, node->x); max_x = FR_MAX(max_x, node->x); min_y = FR_MIN(min_y, node->y); max_y = FR_MAX(max_y, node->y); }
    return (uint32_t)((int32_t)max_x - min_x) * (uint32_t)((int32_t)max_y - min_y);
}

uint64_t fr_space_routes_coordinate_fold(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed ^ 618651u;
    size_t i;
    if (doc == NULL) return state;
    for (i = 0; i < doc->node_count; ++i) { const fr_node_t *node = &doc->nodes[i]; state = fr_hash_mix(state, node->id | ((uint64_t)(uint16_t)node->x << 16) | ((uint64_t)(uint16_t)node->y << 32)); }
    return state;
}

size_t fr_space_routes_nearby_nodes(const fr_document_t *doc, uint16_t node_id, uint32_t radius2) {
    fr_node_t *origin;
    size_t i, count = 0;
    if (doc == NULL) return 0;
    origin = fr_find_node((fr_document_t *)doc, node_id);
    if (origin == NULL) return 0;
    for (i = 0; i < doc->node_count; ++i) if (doc->nodes[i].id != node_id && fr_distance_squared(origin, &doc->nodes[i]) <= radius2) ++count;
    return count;
}

uint32_t fr_space_routes_path_box_total(const fr_document_t *doc) {
    uint32_t total = 0;
    size_t i;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->path_count; ++i) {
        const fr_path_t *path = &doc->paths[i];
        if (path->points != NULL && path->point_count != 0) {
            int16_t lo_x = path->points[0], hi_x = path->points[0], lo_y = path->points[1], hi_y = path->points[1];
            size_t j;
            for (j = 1; j < path->point_count; ++j) { lo_x = FR_MIN(lo_x, path->points[j*2]); hi_x = FR_MAX(hi_x, path->points[j*2]); lo_y = FR_MIN(lo_y, path->points[j*2+1]); hi_y = FR_MAX(hi_y, path->points[j*2+1]); }
            total += (uint32_t)((int32_t)hi_x - lo_x) * (uint32_t)((int32_t)hi_y - lo_y);
        }
    }
    return total;
}

int64_t fr_space_routes_centroid_bias(const fr_document_t *doc) {
    int64_t sum_x = 0, sum_y = 0;
    size_t i;
    if (doc == NULL || doc->node_count == 0) return 0;
    for (i = 0; i < doc->node_count; ++i) { sum_x += doc->nodes[i].x; sum_y += doc->nodes[i].y; }
    return (sum_x / (int64_t)doc->node_count) * 65537 + sum_y / (int64_t)doc->node_count;
}

uint32_t fr_space_routes_quadrant_mask(const fr_document_t *doc) {
    uint32_t mask = 0;
    size_t i;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->node_count; ++i) { const fr_node_t *node = &doc->nodes[i]; unsigned q = (node->x >= 0 ? 1u : 0u) | (node->y >= 0 ? 2u : 0u); mask |= 1u << q; }
    return mask;
}

size_t fr_space_routes_axis_aligned_paths(const fr_document_t *doc) {
    size_t i, count = 0;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->path_count; ++i) { const fr_path_t *path = &doc->paths[i]; if (path->points != NULL && path->point_count >= 2 && (path->points[0] == path->points[2] || path->points[1] == path->points[3])) ++count; }
    return count;
}
