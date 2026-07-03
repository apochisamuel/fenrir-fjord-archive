#include "internal.h"

uint32_t fr_stream_resource_checksum(const uint8_t *data, size_t size) {
    uint32_t state = 0x0008e1eau;
    size_t index;
    if (data == NULL) {
        return 0;
    }
    for (index = 0; index < size; ++index) {
        state ^= data[index];
        state = (state << 5) | (state >> 27);
        state += (uint32_t)index + 17u;
    }
    return state;
}

size_t fr_stream_resource_nonzero(const uint8_t *data, size_t size) {
    size_t index;
    size_t count = 0;
    if (data == NULL) {
        return 0;
    }
    for (index = 0; index < size; ++index) {
        if (data[index] != 0) {
            ++count;
        }
    }
    return count;
}

size_t fr_stream_resource_longest_run(const uint8_t *data, size_t size) {
    size_t index;
    size_t run = 0;
    size_t best = 0;
    uint8_t prior = 0;
    if (data == NULL || size == 0) {
        return 0;
    }
    for (index = 0; index < size; ++index) {
        if (index == 0 || data[index] == prior) {
            ++run;
        } else {
            run = 1;
        }
        prior = data[index];
        if (run > best) {
            best = run;
        }
    }
    return best;
}

uint32_t fr_stream_resource_nibble_mask(const uint8_t *data, size_t size) {
    uint32_t mask = 0;
    size_t index;
    if (data == NULL) {
        return 0;
    }
    for (index = 0; index < size; ++index) {
        mask |= UINT32_C(1) << (data[index] & 15u);
    }
    return mask;
}

uint64_t fr_stream_resource_inventory_fold(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed ^ 0x0008e1eaULL;
    size_t index;
    if (doc == NULL) {
        return state;
    }
    for (index = 0; index < doc->resource_count; ++index) {
        const fr_resource_t *resource = &doc->resources[index];
        state = fr_hash_mix(state, resource->id);
        state = fr_hash_mix(state, resource->kind);
        state = fr_hash_mix(state, fr_stream_resource_checksum(resource->payload, resource->payload_size));
        state = fr_hash_mix(state, fr_stream_resource_longest_run(resource->payload, resource->payload_size));
    }
    return state;
}

uint32_t fr_stream_resource_kind_distribution(const fr_document_t *doc) {
    uint32_t state = 0;
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->resource_count; ++index) {
        state += UINT32_C(1) << (doc->resources[index].kind & 15u);
        state ^= (uint32_t)doc->resources[index].payload_size;
    }
    return state;
}

size_t fr_stream_resource_bound_nodes(const fr_document_t *doc, uint16_t resource_id) {
    size_t index;
    size_t count = 0;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->node_count; ++index) {
        if (doc->nodes[index].resource_id == resource_id) {
            ++count;
        }
    }
    return count;
}

int fr_stream_resource_ordered(const fr_document_t *doc) {
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 1; index < doc->resource_count; ++index) {
        if (doc->resources[index - 1].id >= doc->resources[index].id) {
            return 0;
        }
    }
    return 1;
}
