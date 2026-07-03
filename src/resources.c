#include "internal.h"

static int fr_resource_compare_id(const void *left, const void *right) {
    const fr_resource_t *a = (const fr_resource_t *)left;
    const fr_resource_t *b = (const fr_resource_t *)right;
    return a->id < b->id ? -1 : a->id > b->id;
}

void fr_release_resource_payload(fr_resource_t *resource) {
    if (resource == NULL) return;
    free(resource->payload);
    resource->payload = NULL;
    resource->payload_size = 0;
    resource->payload_hash = 0;
    resource->active = 0;
}

fr_status_t fr_parse_resources(fr_document_t *doc) {
    fr_reader_t r;
    uint16_t count;
    size_t i;
    if (fr_section_reader(doc, FR_TAG('R','S','R','C'), &r) != FR_OK) return FR_OK;
    count = fr_read_u16(&r);
    if (!fr_reader_ok(&r) || count > FR_MAX_ITEMS) {
        fr_set_error(doc->ctx, FR_E_RANGE, "resource table count is outside supported range");
        return FR_E_RANGE;
    }
    doc->resources = (fr_resource_t *)fr_calloc(doc->ctx, count, sizeof(*doc->resources));
    if (doc->resources == NULL && count != 0) return FR_E_ALLOC;
    doc->resource_count = count;
    for (i = 0; i < count; ++i) {
        fr_resource_t *resource = &doc->resources[i];
        fr_slice_t payload;
        resource->id = fr_read_u16(&r);
        resource->kind = fr_read_u16(&r);
        resource->check = fr_read_u32(&r);
        resource->payload_size = fr_read_u32(&r);
        if (resource->payload_size > FR_MAX_TEXT) {
            fr_set_error(doc->ctx, FR_E_RANGE, "resource payload is too large");
            return FR_E_RANGE;
        }
        payload = fr_read_slice(&r, resource->payload_size);
        if (!fr_reader_ok(&r)) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "resource payload is incomplete");
            return FR_E_FORMAT;
        }
        resource->payload = (uint8_t *)fr_calloc(doc->ctx, resource->payload_size, 1);
        if (resource->payload == NULL && resource->payload_size != 0) return FR_E_ALLOC;
        if (resource->payload_size != 0) memcpy(resource->payload, payload.data, resource->payload_size);
        resource->payload_hash = fr_hash_bytes(payload.data, payload.size);
        resource->active = 1;
    }
    qsort(doc->resources, doc->resource_count, sizeof(*doc->resources), fr_resource_compare_id);
    for (i = 1; i < doc->resource_count; ++i) {
        if (doc->resources[i - 1].id == doc->resources[i].id) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "resource table contains duplicate identifiers");
            return FR_E_FORMAT;
        }
    }
    doc->state_bits |= 256u;
    return FR_OK;
}

fr_resource_t *fr_find_resource(fr_document_t *doc, uint16_t id) {
    size_t lo = 0, hi;
    if (doc == NULL || doc->resources == NULL) return NULL;
    hi = doc->resource_count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (doc->resources[mid].id == id) return &doc->resources[mid];
        if (doc->resources[mid].id < id) lo = mid + 1;
        else hi = mid;
    }
    return NULL;
}


fr_status_t fr_prepare_resource_cache(fr_document_t *doc) {
    size_t i;
    fr_resource_cache_t *cache;
    if (doc == NULL) return FR_E_ARGUMENT;
    cache = &doc->resource_cache;
    cache->count = doc->resource_count;
    cache->resource_ids = (uint16_t *)fr_calloc(doc->ctx, cache->count, sizeof(*cache->resource_ids));
    cache->payloads = (uint8_t **)fr_calloc(doc->ctx, cache->count, sizeof(*cache->payloads));
    cache->sizes = (size_t *)fr_calloc(doc->ctx, cache->count, sizeof(*cache->sizes));
    if (cache->count != 0 && (cache->resource_ids == NULL || cache->payloads == NULL || cache->sizes == NULL)) return FR_E_ALLOC;
    for (i = 0; i < cache->count; ++i) {
        cache->resource_ids[i] = doc->resources[i].id;
        cache->payloads[i] = doc->resources[i].payload;
        cache->sizes[i] = doc->resources[i].payload_size;
    }
    return FR_OK;
}
