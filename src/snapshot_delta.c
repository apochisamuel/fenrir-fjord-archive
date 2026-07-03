#include "internal.h"

uint64_t fr_snapshot_delta_snapshot(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed ^ 0x0000d442ULL;
    size_t index;
    if (doc == NULL) {
        return state;
    }
    state = fr_hash_mix(state, doc->digest);
    state = fr_hash_mix(state, doc->state_bits);
    for (index = 0; index < doc->node_count; ++index) {
        const fr_node_t *node = &doc->nodes[index];
        state = fr_hash_mix(state, node->id);
        state = fr_hash_mix(state, (uint16_t)node->x);
        state = fr_hash_mix(state, (uint16_t)node->y);
        state = fr_hash_mix(state, node->flags);
    }
    return state;
}

uint64_t fr_snapshot_delta_delta_nodes(const fr_document_t *left, const fr_document_t *right) {
    uint64_t state = 0x0000d442ULL;
    size_t count;
    size_t index;
    if (left == NULL || right == NULL) {
        return state;
    }
    count = FR_MAX(left->node_count, right->node_count);
    for (index = 0; index < count; ++index) {
        uint64_t a = 0;
        uint64_t b = 0;
        if (index < left->node_count) {
            const fr_node_t *node = &left->nodes[index];
            a = node->id | ((uint64_t)(uint16_t)node->x << 16) | ((uint64_t)(uint16_t)node->y << 32);
        }
        if (index < right->node_count) {
            const fr_node_t *node = &right->nodes[index];
            b = node->id | ((uint64_t)(uint16_t)node->x << 16) | ((uint64_t)(uint16_t)node->y << 32);
        }
        state = fr_hash_mix(state, a ^ b);
    }
    return state;
}

uint32_t fr_snapshot_delta_coverage(const fr_document_t *doc) {
    uint32_t result = 0;
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->link_count; ++index) {
        const fr_link_t *link = &doc->links[index];
        if (link->from != NULL) {
            result ^= link->from->id;
        }
        if (link->to != NULL) {
            result ^= link->to->id << 1;
        }
        if (link->label_view != NULL) {
            result ^= fr_text_signature(link->label_view);
        }
    }
    return result;
}

size_t fr_snapshot_delta_select_annotations(const fr_document_t *doc, uint16_t kind) {
    size_t index;
    size_t count = 0;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->annotation_count; ++index) {
        const fr_annotation_t *annotation = &doc->annotations[index];
        if (annotation->active && annotation->target_kind == kind) {
            ++count;
        }
    }
    return count;
}

uint64_t fr_snapshot_delta_resource_delta(const fr_document_t *doc) {
    uint64_t state = 54338u;
    size_t index;
    if (doc == NULL) {
        return state;
    }
    for (index = 0; index < doc->resource_count; ++index) {
        const fr_resource_t *resource = &doc->resources[index];
        state = fr_hash_mix(state, resource->id);
        state = fr_hash_mix(state, resource->check);
        state = fr_hash_mix(state, resource->payload_hash);
        state = fr_hash_mix(state, resource->payload_size);
    }
    return state;
}

uint32_t fr_snapshot_delta_metric_coverage(const fr_document_t *doc) {
    uint32_t total = 0;
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->metric_count; ++index) {
        const fr_metric_t *metric = &doc->metrics[index];
        if (metric->node != NULL) {
            total += metric->sample_count;
        }
        if (metric->samples != NULL && metric->sample_count != 0) {
            total ^= (uint32_t)metric->samples[0];
        }
    }
    return total;
}

int fr_snapshot_delta_ordering(const fr_document_t *doc) {
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 1; index < doc->path_count; ++index) {
        if (doc->paths[index - 1].id >= doc->paths[index].id) {
            return 0;
        }
    }
    for (index = 1; index < doc->metric_count; ++index) {
        if (doc->metrics[index - 1].id >= doc->metrics[index].id) {
            return 0;
        }
    }
    return 1;
}

uint64_t fr_snapshot_delta_summary(const fr_document_t *doc) {
    uint64_t state;
    if (doc == NULL) {
        return 0;
    }
    state = fr_snapshot_delta_snapshot(doc, 54338u);
    state = fr_hash_mix(state, fr_snapshot_delta_coverage(doc));
    state = fr_hash_mix(state, fr_snapshot_delta_resource_delta(doc));
    state = fr_hash_mix(state, fr_snapshot_delta_metric_coverage(doc));
    state = fr_hash_mix(state, fr_snapshot_delta_ordering(doc));
    return state;
}
