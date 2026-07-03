#include "internal.h"

uint32_t fr_partition_routes_bucket(const fr_node_t *node, uint32_t width) {
    int32_t x;
    int32_t y;
    if (node == NULL || width == 0) {
        return 0;
    }
    x = node->x / (int32_t)width;
    y = node->y / (int32_t)width;
    return ((uint32_t)x & 0xffffu) | ((uint32_t)y << 16);
}

size_t fr_partition_routes_bucket_count(const fr_document_t *doc, uint32_t bucket, uint32_t width) {
    size_t index;
    size_t count = 0;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->node_count; ++index) {
        if (fr_partition_routes_bucket(&doc->nodes[index], width) == bucket) {
            ++count;
        }
    }
    return count;
}

uint64_t fr_partition_routes_partition_fold(const fr_document_t *doc, uint32_t width) {
    uint64_t state = 0x0001331fULL;
    size_t index;
    if (doc == NULL) {
        return state;
    }
    for (index = 0; index < doc->node_count; ++index) {
        uint32_t key = fr_partition_routes_bucket(&doc->nodes[index], width);
        state = fr_hash_mix(state, key);
        state = fr_hash_mix(state, doc->nodes[index].id);
    }
    return state;
}

size_t fr_partition_routes_isolated_nodes(const fr_document_t *doc) {
    size_t node_index;
    size_t count = 0;
    if (doc == NULL) {
        return 0;
    }
    for (node_index = 0; node_index < doc->node_count; ++node_index) {
        uint16_t id = doc->nodes[node_index].id;
        size_t link_index;
        int found = 0;
        for (link_index = 0; link_index < doc->link_count; ++link_index) {
            if (doc->links[link_index].from_id == id || doc->links[link_index].to_id == id) {
                found = 1;
                break;
            }
        }
        if (!found) {
            ++count;
        }
    }
    return count;
}

uint32_t fr_partition_routes_group_coverage(const fr_document_t *doc) {
    uint32_t total = 0;
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->group_count; ++index) {
        const fr_group_t *group = &doc->groups[index];
        uint32_t available = 0;
        size_t member_index;
        for (member_index = 0; group->members != NULL && member_index < group->member_count; ++member_index) {
            if (group->members[member_index] != NULL) {
                ++available;
            }
        }
        total += available * (group->score & 15u);
    }
    return total;
}

uint64_t fr_partition_routes_path_partition(const fr_document_t *doc) {
    uint64_t state = 78623u;
    size_t index;
    if (doc == NULL) {
        return state;
    }
    for (index = 0; index < doc->path_count; ++index) {
        const fr_path_t *path = &doc->paths[index];
        state = fr_hash_mix(state, path->group_id);
        state = fr_hash_mix(state, path->anchor_id);
        state = fr_hash_mix(state, path->point_count);
    }
    return state;
}

int fr_partition_routes_group_ranked(const fr_document_t *doc) {
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 1; index < doc->group_count; ++index) {
        if (doc->groups[index - 1].id >= doc->groups[index].id) {
            return 0;
        }
    }
    return 1;
}

uint32_t fr_partition_routes_route_density(const fr_document_t *doc) {
    uint32_t points = 0;
    size_t index;
    if (doc == NULL || doc->node_count == 0) {
        return 0;
    }
    for (index = 0; index < doc->path_count; ++index) {
        points += doc->paths[index].point_count;
    }
    return points / (uint32_t)doc->node_count;
}
