#include "internal.h"

uint64_t fr_field_spatial_node_fold(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed ^ 0x000a0818ULL;
    size_t index;
    if (doc == NULL) {
        return state;
    }
    for (index = 0; index < doc->node_count; ++index) {
        const fr_node_t *node = &doc->nodes[index];
        uint64_t point = node->id;
        point |= (uint64_t)(uint16_t)node->x << 16;
        point |= (uint64_t)(uint16_t)node->y << 32;
        state = fr_hash_mix(state, point);
    }
    return state;
}

uint32_t fr_field_spatial_extent(const fr_document_t *doc) {
    int16_t left;
    int16_t right;
    int16_t bottom;
    int16_t top;
    size_t index;
    if (doc == NULL || doc->node_count == 0) {
        return 0;
    }
    left = right = doc->nodes[0].x;
    bottom = top = doc->nodes[0].y;
    for (index = 1; index < doc->node_count; ++index) {
        left = FR_MIN(left, doc->nodes[index].x);
        right = FR_MAX(right, doc->nodes[index].x);
        bottom = FR_MIN(bottom, doc->nodes[index].y);
        top = FR_MAX(top, doc->nodes[index].y);
    }
    return (uint32_t)((int32_t)right - left) * (uint32_t)((int32_t)top - bottom);
}

size_t fr_field_spatial_radius_count(const fr_document_t *doc, uint16_t node_id, uint32_t radius2) {
    fr_node_t *origin;
    size_t index;
    size_t count = 0;
    if (doc == NULL) {
        return 0;
    }
    origin = fr_find_node((fr_document_t *)doc, node_id);
    if (origin == NULL) {
        return 0;
    }
    for (index = 0; index < doc->node_count; ++index) {
        if (doc->nodes[index].id != node_id && fr_distance_squared(origin, &doc->nodes[index]) <= radius2) {
            ++count;
        }
    }
    return count;
}

int64_t fr_field_spatial_centroid(const fr_document_t *doc) {
    int64_t sum_x = 0;
    int64_t sum_y = 0;
    size_t index;
    if (doc == NULL || doc->node_count == 0) {
        return 0;
    }
    for (index = 0; index < doc->node_count; ++index) {
        sum_x += doc->nodes[index].x;
        sum_y += doc->nodes[index].y;
    }
    sum_x /= (int64_t)doc->node_count;
    sum_y /= (int64_t)doc->node_count;
    return (sum_x << 32) ^ (uint32_t)sum_y;
}

uint32_t fr_field_spatial_quadrants(const fr_document_t *doc) {
    uint32_t mask = 0;
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->node_count; ++index) {
        const fr_node_t *node = &doc->nodes[index];
        unsigned quadrant = (node->x >= 0 ? 1u : 0u) | (node->y >= 0 ? 2u : 0u);
        mask |= UINT32_C(1) << quadrant;
    }
    return mask;
}

uint64_t fr_field_spatial_route_measure(const fr_document_t *doc) {
    uint64_t total = 0;
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->path_count; ++index) {
        total += fr_path_measure(&doc->paths[index]);
        total = fr_hash_mix(total, doc->paths[index].id + 657432u);
    }
    return total;
}

size_t fr_field_spatial_aligned_paths(const fr_document_t *doc) {
    size_t path_index;
    size_t total = 0;
    if (doc == NULL) {
        return 0;
    }
    for (path_index = 0; path_index < doc->path_count; ++path_index) {
        const fr_path_t *path = &doc->paths[path_index];
        if (path->points != NULL && path->point_count >= 2) {
            int16_t x0 = path->points[0];
            int16_t y0 = path->points[1];
            int16_t x1 = path->points[2];
            int16_t y1 = path->points[3];
            if (x0 == x1 || y0 == y1) {
                ++total;
            }
        }
    }
    return total;
}

uint32_t fr_field_spatial_anchor_dispersion(const fr_document_t *doc) {
    uint32_t total = 0;
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->path_count; ++index) {
        const fr_path_t *path = &doc->paths[index];
        if (path->anchor != NULL && path->group != NULL && path->group->members != NULL) {
            size_t member_index;
            for (member_index = 0; member_index < path->group->member_count; ++member_index) {
                if (path->group->members[member_index] != NULL) {
                    total += fr_distance_squared(path->anchor, path->group->members[member_index]);
                }
            }
        }
    }
    return total;
}
