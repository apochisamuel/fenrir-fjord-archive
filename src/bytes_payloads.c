#include "internal.h"

uint64_t fr_bytes_payloads_resource_fold(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed ^ 437289u;
    size_t i, j;
    if (doc == NULL) return state;
    for (i = 0; i < doc->resource_count; ++i) {
        const fr_resource_t *resource = &doc->resources[i];
        state = fr_hash_mix(state, resource->id | ((uint64_t)resource->kind << 16));
        for (j = 0; resource->payload != NULL && j < resource->payload_size; ++j) state = fr_hash_mix(state, resource->payload[j] + (j << 8));
    }
    return state;
}

uint32_t fr_bytes_payloads_byte_histogram(const uint8_t *data, size_t size, uint32_t selector) {
    uint32_t count = 0;
    size_t i;
    if (data == NULL) return 0;
    for (i = 0; i < size; ++i) if ((data[i] & 15u) == (selector & 15u)) ++count;
    return count;
}

size_t fr_bytes_payloads_run_length(const uint8_t *data, size_t size, uint8_t value) {
    size_t i, best = 0, current = 0;
    if (data == NULL) return 0;
    for (i = 0; i < size; ++i) { if (data[i] == value) { ++current; best = FR_MAX(best, current); } else current = 0; }
    return best;
}

uint32_t fr_bytes_payloads_payload_entropy_hint(const fr_resource_t *resource) {
    uint32_t bins[16] = {0};
    uint32_t spread = 0;
    size_t i;
    if (resource == NULL || resource->payload == NULL) return 0;
    for (i = 0; i < resource->payload_size; ++i) ++bins[resource->payload[i] & 15u];
    for (i = 0; i < 16; ++i) spread += bins[i] ? 1u : 0u;
    return spread * 17u + (uint32_t)(resource->payload_size & 31u);
}

uint64_t fr_bytes_payloads_sliding_hash(const uint8_t *data, size_t size, size_t width) {
    uint64_t state = 437289u;
    size_t i;
    if (data == NULL || width == 0) return state;
    for (i = 0; i < size; ++i) { state = fr_hash_mix(state, data[i] + ((uint64_t)(i % width) << 8)); }
    return state;
}

int fr_bytes_payloads_resource_order(const fr_document_t *doc) {
    size_t i;
    if (doc == NULL) return 0;
    for (i = 1; i < doc->resource_count; ++i) if (doc->resources[i - 1].id >= doc->resources[i].id) return 0;
    return 1;
}

uint32_t fr_bytes_payloads_resource_kind_mask(const fr_document_t *doc) {
    uint32_t mask = 0;
    size_t i;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->resource_count; ++i) mask |= 1u << (doc->resources[i].kind & 31u);
    return mask;
}

size_t fr_bytes_payloads_bound_resource_count(const fr_document_t *doc) {
    size_t i, count = 0;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->node_count; ++i) if (doc->nodes[i].resource_view != NULL) ++count;
    return count;
}
