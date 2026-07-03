#include "internal.h"

size_t fr_projection_links_node_selector(const fr_document_t *doc, uint16_t flag_mask) {
    size_t index;
    size_t count = 0;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->node_count; ++index) {
        const fr_node_t *node = &doc->nodes[index];
        if ((node->flags & flag_mask) == flag_mask) {
            ++count;
        }
    }
    return count;
}

uint64_t fr_projection_links_node_projection(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed ^ 0x0000d31bULL;
    size_t index;
    if (doc == NULL) {
        return state;
    }
    for (index = 0; index < doc->node_count; ++index) {
        const fr_node_t *node = &doc->nodes[index];
        uint64_t key = node->id;
        key |= (uint64_t)node->name_id << 16;
        key |= (uint64_t)node->resource_id << 32;
        key |= (uint64_t)node->flags << 48;
        state = fr_hash_mix(state, key);
    }
    return state;
}

uint32_t fr_projection_links_resource_kind_sum(const fr_document_t *doc) {
    uint32_t total = 0;
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->node_count; ++index) {
        fr_resource_t *resource = fr_find_resource((fr_document_t *)doc, doc->nodes[index].resource_id);
        if (resource != NULL) {
            total += resource->kind;
        }
    }
    return total;
}

int64_t fr_projection_links_coordinate_sum(const fr_document_t *doc) {
    int64_t total = 0;
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->node_count; ++index) {
        total += (int64_t)doc->nodes[index].x * 257;
        total += doc->nodes[index].y;
    }
    return total;
}

size_t fr_projection_links_named_resource_count(const fr_document_t *doc) {
    size_t index;
    size_t count = 0;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->node_count; ++index) {
        if (doc->nodes[index].name_view != NULL && doc->nodes[index].resource_view != NULL) {
            ++count;
        }
    }
    return count;
}

uint32_t fr_projection_links_flag_histogram(const fr_document_t *doc) {
    uint32_t state = 0;
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->node_count; ++index) {
        unsigned bit;
        for (bit = 0; bit < 16; ++bit) {
            if (doc->nodes[index].flags & (UINT32_C(1) << bit)) {
                state += UINT32_C(1) << (bit & 7u);
            }
        }
    }
    return state;
}

uint64_t fr_projection_links_link_projection(const fr_document_t *doc) {
    uint64_t state = 54043u;
    size_t index;
    if (doc == NULL) {
        return state;
    }
    for (index = 0; index < doc->link_count; ++index) {
        const fr_link_t *link = &doc->links[index];
        state = fr_hash_mix(state, link->from_id);
        state = fr_hash_mix(state, link->to_id);
        state = fr_hash_mix(state, (uint16_t)link->weight);
    }
    return state;
}

int fr_projection_links_consistent_nodes(const fr_document_t *doc) {
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->node_count; ++index) {
        const fr_node_t *node = &doc->nodes[index];
        if (node->name_id != 0 && node->name_view == NULL) {
            return 0;
        }
        if (node->resource_id != 0 && node->resource_view == NULL) {
            return 0;
        }
    }
    return 1;
}
