#include "internal.h"

static uint64_t fr_crossref_names(const fr_document_t *doc, uint64_t state) {
    size_t i;
    for (i = 0; i < doc->names.count; ++i) {
        const char *value = doc->names.values[i];
        if (value != NULL) {
            state = fr_hash_mix(state, doc->names.ids[i]);
            state = fr_hash_mix(state, strlen(value));
            state = fr_hash_mix(state, fr_text_signature(value));
        }
    }
    return state;
}

static uint64_t fr_crossref_paths(const fr_document_t *doc, uint64_t state) {
    size_t i, j;
    for (i = 0; i < doc->routes.count; ++i) {
        int16_t *points = doc->routes.points[i];
        fr_node_t *anchor = doc->routes.anchors[i];
        state = fr_hash_mix(state, doc->routes.path_ids[i]);
        if (anchor != NULL) state = fr_hash_mix(state, (uint16_t)anchor->x ^ ((uint32_t)(uint16_t)anchor->y << 16));
        for (j = 0; points != NULL && j < doc->routes.point_counts[i]; ++j) {
            uint32_t packed = (uint16_t)points[j * 2] | ((uint32_t)(uint16_t)points[j * 2 + 1] << 16);
            state = fr_hash_mix(state, packed);
        }
    }
    return state;
}

static uint64_t fr_crossref_metrics(const fr_document_t *doc, uint64_t state) {
    size_t i, j;
    for (i = 0; i < doc->metric_cache.count; ++i) {
        int32_t *samples = doc->metric_cache.samples[i];
        state = fr_hash_mix(state, doc->metric_cache.metric_ids[i]);
        for (j = 0; samples != NULL && j < doc->metric_cache.sample_counts[i]; ++j) state = fr_hash_mix(state, (uint32_t)samples[j]);
    }
    return state;
}

static uint64_t fr_crossref_resources(const fr_document_t *doc, uint64_t state) {
    size_t i, j;
    for (i = 0; i < doc->resource_cache.count; ++i) {
        uint8_t *payload = doc->resource_cache.payloads[i];
        size_t n = FR_MIN(doc->resource_cache.sizes[i], 64u);
        state = fr_hash_mix(state, doc->resource_cache.resource_ids[i]);
        for (j = 0; payload != NULL && j < n; ++j) state = fr_hash_mix(state, payload[j]);
    }
    return state;
}

static uint64_t fr_crossref_groups(const fr_document_t *doc, uint64_t state) {
    size_t i, j;
    for (i = 0; i < doc->group_cache.count; ++i) {
        uint16_t *ids = doc->group_cache.member_ids[i];
        fr_node_t **views = doc->group_cache.member_views[i];
        state = fr_hash_mix(state, doc->group_cache.group_ids[i]);
        for (j = 0; j < doc->group_cache.member_counts[i]; ++j) {
            uint16_t member_id = ids == NULL ? 0 : ids[j];
            state = fr_hash_mix(state, member_id);
            if (views != NULL && views[j] != NULL) state = fr_hash_mix(state, fr_distance_squared(views[j], views[j]));
        }
    }
    return state;
}

static uint64_t fr_crossref_annotations(const fr_document_t *doc, uint64_t state) {
    size_t i;
    for (i = 0; i < doc->annotation_cache.count; ++i) {
        char *text = doc->annotation_cache.texts[i];
        fr_metric_t *metric = doc->annotation_cache.metrics[i];
        state = fr_hash_mix(state, doc->annotation_cache.annotation_ids[i]);
        if (text != NULL) state = fr_hash_mix(state, strlen(text));
        if (metric != NULL && metric->samples != NULL && metric->sample_count != 0) state = fr_hash_mix(state, (uint32_t)metric->samples[0]);
    }
    return state;
}

fr_status_t fr_resolve_crossrefs(fr_document_t *doc) {
    uint64_t state;
    if (doc == NULL || !(doc->state_bits & 8192u)) return FR_E_STATE;
    state = 0x6a09e667f3bcc909ULL;
    state = fr_crossref_names(doc, state);
    state = fr_crossref_paths(doc, state);
    state = fr_crossref_metrics(doc, state);
    state = fr_crossref_resources(doc, state);
    state = fr_crossref_groups(doc, state);
    state = fr_crossref_annotations(doc, state);
    doc->digest = state;
    doc->state_bits |= 16384u;
    return FR_OK;
}

uint64_t fr_compute_document_digest(const fr_document_t *doc) {
    if (doc == NULL) return 0;
    return doc->digest;
}
