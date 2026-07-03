#include "internal.h"

static int fr_group_compare_id(const void *left, const void *right) {
    const fr_group_t *a = (const fr_group_t *)left;
    const fr_group_t *b = (const fr_group_t *)right;
    return a->id < b->id ? -1 : a->id > b->id;
}

fr_status_t fr_parse_groups(fr_document_t *doc) {
    fr_reader_t r;
    uint16_t count;
    size_t i, j;
    if (fr_section_reader(doc, FR_TAG('G','R','U','P'), &r) != FR_OK) return FR_OK;
    count = fr_read_u16(&r);
    if (!fr_reader_ok(&r) || count > FR_MAX_ITEMS) {
        fr_set_error(doc->ctx, FR_E_RANGE, "group table count is outside supported range");
        return FR_E_RANGE;
    }
    doc->groups = (fr_group_t *)fr_calloc(doc->ctx, count, sizeof(*doc->groups));
    if (doc->groups == NULL && count != 0) return FR_E_ALLOC;
    doc->group_count = count;
    for (i = 0; i < count; ++i) {
        fr_group_t *group = &doc->groups[i];
        group->id = fr_read_u16(&r);
        group->name_id = fr_read_u16(&r);
        group->member_count = fr_read_u16(&r);
        group->score = fr_read_u32(&r);
        if (!fr_reader_ok(&r) || group->member_count > FR_MAX_ITEMS) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "group record has invalid dimensions");
            return FR_E_FORMAT;
        }
        group->member_ids = (uint16_t *)fr_calloc(doc->ctx, group->member_count, sizeof(*group->member_ids));
        if (group->member_ids == NULL && group->member_count != 0) return FR_E_ALLOC;
        for (j = 0; j < group->member_count; ++j) group->member_ids[j] = fr_read_u16(&r);
        if (!fr_reader_ok(&r)) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "group member list is incomplete");
            return FR_E_FORMAT;
        }
        group->active = 1;
    }
    qsort(doc->groups, doc->group_count, sizeof(*doc->groups), fr_group_compare_id);
    for (i = 1; i < doc->group_count; ++i) {
        if (doc->groups[i - 1].id == doc->groups[i].id) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "group table contains duplicate identifiers");
            return FR_E_FORMAT;
        }
    }
    doc->state_bits |= 32u;
    return FR_OK;
}

fr_group_t *fr_find_group(fr_document_t *doc, uint16_t id) {
    size_t lo = 0, hi;
    if (doc == NULL || doc->groups == NULL) return NULL;
    hi = doc->group_count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (doc->groups[mid].id == id) return &doc->groups[mid];
        if (doc->groups[mid].id < id) lo = mid + 1;
        else hi = mid;
    }
    return NULL;
}


fr_status_t fr_prepare_group_cache(fr_document_t *doc) {
    size_t i;
    fr_group_cache_t *cache;
    if (doc == NULL) return FR_E_ARGUMENT;
    cache = &doc->group_cache;
    cache->count = doc->group_count;
    cache->group_ids = (uint16_t *)fr_calloc(doc->ctx, cache->count, sizeof(*cache->group_ids));
    cache->member_ids = (uint16_t **)fr_calloc(doc->ctx, cache->count, sizeof(*cache->member_ids));
    cache->member_views = (fr_node_t ***)fr_calloc(doc->ctx, cache->count, sizeof(*cache->member_views));
    cache->member_counts = (size_t *)fr_calloc(doc->ctx, cache->count, sizeof(*cache->member_counts));
    if (cache->count != 0 && (cache->group_ids == NULL || cache->member_ids == NULL || cache->member_views == NULL || cache->member_counts == NULL)) return FR_E_ALLOC;
    for (i = 0; i < cache->count; ++i) {
        cache->group_ids[i] = doc->groups[i].id;
        cache->member_ids[i] = doc->groups[i].member_ids;
        cache->member_views[i] = NULL;
        cache->member_counts[i] = doc->groups[i].member_count;
    }
    return FR_OK;
}
