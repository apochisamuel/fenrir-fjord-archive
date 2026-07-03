#include "internal.h"

static int fr_path_compare_id(const void *left, const void *right) {
    const fr_path_t *a = (const fr_path_t *)left;
    const fr_path_t *b = (const fr_path_t *)right;
    return a->id < b->id ? -1 : a->id > b->id;
}

void fr_release_path_points(fr_path_t *path) {
    if (path == NULL) return;
    free(path->points);
    path->points = NULL;
    path->point_count = 0;
    path->active = 0;
}

fr_status_t fr_parse_paths(fr_document_t *doc) {
    fr_reader_t r;
    uint16_t count;
    size_t i, j;
    if (fr_section_reader(doc, FR_TAG('P','A','T','H'), &r) != FR_OK) return FR_OK;
    count = fr_read_u16(&r);
    if (!fr_reader_ok(&r) || count > FR_MAX_ITEMS) {
        fr_set_error(doc->ctx, FR_E_RANGE, "path table count is outside supported range");
        return FR_E_RANGE;
    }
    doc->paths = (fr_path_t *)fr_calloc(doc->ctx, count, sizeof(*doc->paths));
    if (doc->paths == NULL && count != 0) return FR_E_ALLOC;
    doc->path_count = count;
    for (i = 0; i < count; ++i) {
        fr_path_t *path = &doc->paths[i];
        path->id = fr_read_u16(&r);
        path->group_id = fr_read_u16(&r);
        path->anchor_id = fr_read_u16(&r);
        path->point_count = fr_read_u16(&r);
        path->length_hint = fr_read_u32(&r);
        if (!fr_reader_ok(&r) || path->point_count > FR_MAX_ITEMS) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "path record has invalid dimensions");
            return FR_E_FORMAT;
        }
        path->points = (int16_t *)fr_calloc(doc->ctx, path->point_count * 2u, sizeof(*path->points));
        if (path->points == NULL && path->point_count != 0) return FR_E_ALLOC;
        for (j = 0; j < (size_t)path->point_count * 2u; ++j) path->points[j] = fr_read_i16(&r);
        if (!fr_reader_ok(&r)) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "path point list is incomplete");
            return FR_E_FORMAT;
        }
        path->active = 1;
    }
    qsort(doc->paths, doc->path_count, sizeof(*doc->paths), fr_path_compare_id);
    for (i = 1; i < doc->path_count; ++i) {
        if (doc->paths[i - 1].id == doc->paths[i].id) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "path table contains duplicate identifiers");
            return FR_E_FORMAT;
        }
    }
    doc->state_bits |= 64u;
    return FR_OK;
}


fr_status_t fr_prepare_route_cache(fr_document_t *doc) {
    size_t i;
    fr_route_cache_t *cache;
    if (doc == NULL) return FR_E_ARGUMENT;
    cache = &doc->routes;
    cache->count = doc->path_count;
    cache->path_ids = (uint16_t *)fr_calloc(doc->ctx, cache->count, sizeof(*cache->path_ids));
    cache->anchors = (fr_node_t **)fr_calloc(doc->ctx, cache->count, sizeof(*cache->anchors));
    cache->points = (int16_t **)fr_calloc(doc->ctx, cache->count, sizeof(*cache->points));
    cache->point_counts = (size_t *)fr_calloc(doc->ctx, cache->count, sizeof(*cache->point_counts));
    if (cache->count != 0 && (cache->path_ids == NULL || cache->anchors == NULL || cache->points == NULL || cache->point_counts == NULL)) return FR_E_ALLOC;
    for (i = 0; i < cache->count; ++i) {
        cache->path_ids[i] = doc->paths[i].id;
        cache->anchors[i] = NULL;
        cache->points[i] = doc->paths[i].points;
        cache->point_counts[i] = doc->paths[i].point_count;
    }
    return FR_OK;
}
