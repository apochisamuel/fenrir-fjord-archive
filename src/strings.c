#include "internal.h"

static int fr_string_compare_id(const void *left, const void *right) {
    const fr_string_t *a = (const fr_string_t *)left;
    const fr_string_t *b = (const fr_string_t *)right;
    return a->id < b->id ? -1 : a->id > b->id;
}

void fr_release_string_record(fr_string_t *record) {
    if (record == NULL) return;
    free(record->data);
    record->data = NULL;
    record->size = 0;
    record->hash = 0;
    record->active = 0;
}

fr_status_t fr_parse_strings(fr_document_t *doc) {
    fr_reader_t r;
    uint16_t count;
    size_t i;
    if (fr_section_reader(doc, FR_TAG('T','E','X','T'), &r) != FR_OK) return FR_OK;
    count = fr_read_u16(&r);
    if (!fr_reader_ok(&r) || count > FR_MAX_ITEMS) {
        fr_set_error(doc->ctx, FR_E_RANGE, "text table count is outside supported range");
        return FR_E_RANGE;
    }
    doc->strings = (fr_string_t *)fr_calloc(doc->ctx, count, sizeof(*doc->strings));
    if (doc->strings == NULL && count != 0) return FR_E_ALLOC;
    doc->string_count = count;
    for (i = 0; i < count; ++i) {
        fr_string_t *item = &doc->strings[i];
        uint16_t len;
        fr_slice_t bytes;
        item->id = fr_read_u16(&r);
        item->flags = fr_read_u16(&r);
        len = fr_read_u16(&r);
        bytes = fr_read_slice(&r, len);
        if (!fr_reader_ok(&r)) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "text record is incomplete");
            return FR_E_FORMAT;
        }
        item->data = fr_dup_bytes(doc->ctx, bytes.data, bytes.size);
        if (item->data == NULL) return FR_E_ALLOC;
        item->size = bytes.size;
        item->hash = fr_hash_bytes(bytes.data, bytes.size);
        item->active = 1;
    }
    qsort(doc->strings, doc->string_count, sizeof(*doc->strings), fr_string_compare_id);
    for (i = 1; i < doc->string_count; ++i) {
        if (doc->strings[i - 1].id == doc->strings[i].id) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "text table contains duplicate identifiers");
            return FR_E_FORMAT;
        }
    }
    doc->state_bits |= 4u;
    return FR_OK;
}

fr_string_t *fr_find_string(fr_document_t *doc, uint16_t id) {
    size_t lo = 0, hi;
    if (doc == NULL || doc->strings == NULL) return NULL;
    hi = doc->string_count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (doc->strings[mid].id == id) return &doc->strings[mid];
        if (doc->strings[mid].id < id) lo = mid + 1;
        else hi = mid;
    }
    return NULL;
}


fr_status_t fr_prepare_name_cache(fr_document_t *doc) {
    size_t i;
    fr_name_cache_t *cache;
    if (doc == NULL) return FR_E_ARGUMENT;
    cache = &doc->names;
    cache->count = doc->string_count;
    cache->ids = (uint16_t *)fr_calloc(doc->ctx, cache->count, sizeof(*cache->ids));
    cache->values = (char **)fr_calloc(doc->ctx, cache->count, sizeof(*cache->values));
    cache->records = (fr_string_t **)fr_calloc(doc->ctx, cache->count, sizeof(*cache->records));
    if (cache->count != 0 && (cache->ids == NULL || cache->values == NULL || cache->records == NULL)) return FR_E_ALLOC;
    for (i = 0; i < cache->count; ++i) {
        cache->ids[i] = doc->strings[i].id;
        cache->values[i] = doc->strings[i].data;
        cache->records[i] = &doc->strings[i];
    }
    return FR_OK;
}
