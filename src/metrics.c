#include "internal.h"

static int fr_metric_compare_id(const void *left, const void *right) {
    const fr_metric_t *a = (const fr_metric_t *)left;
    const fr_metric_t *b = (const fr_metric_t *)right;
    return a->id < b->id ? -1 : a->id > b->id;
}

void fr_release_metric_samples(fr_metric_t *metric) {
    if (metric == NULL) return;
    free(metric->samples);
    metric->samples = NULL;
    metric->sample_count = 0;
    metric->total = 0;
    metric->active = 0;
}

fr_status_t fr_parse_metrics(fr_document_t *doc) {
    fr_reader_t r;
    uint16_t count;
    size_t i, j;
    if (fr_section_reader(doc, FR_TAG('M','T','R','C'), &r) != FR_OK) return FR_OK;
    count = fr_read_u16(&r);
    if (!fr_reader_ok(&r) || count > FR_MAX_ITEMS) {
        fr_set_error(doc->ctx, FR_E_RANGE, "metric table count is outside supported range");
        return FR_E_RANGE;
    }
    doc->metrics = (fr_metric_t *)fr_calloc(doc->ctx, count, sizeof(*doc->metrics));
    if (doc->metrics == NULL && count != 0) return FR_E_ALLOC;
    doc->metric_count = count;
    for (i = 0; i < count; ++i) {
        fr_metric_t *metric = &doc->metrics[i];
        metric->id = fr_read_u16(&r);
        metric->node_id = fr_read_u16(&r);
        metric->sample_count = fr_read_u16(&r);
        (void)fr_read_u16(&r);
        if (!fr_reader_ok(&r) || metric->sample_count > FR_MAX_ITEMS) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "metric record has invalid dimensions");
            return FR_E_FORMAT;
        }
        metric->samples = (int32_t *)fr_calloc(doc->ctx, metric->sample_count, sizeof(*metric->samples));
        if (metric->samples == NULL && metric->sample_count != 0) return FR_E_ALLOC;
        metric->total = 0;
        for (j = 0; j < metric->sample_count; ++j) {
            metric->samples[j] = fr_read_i32(&r);
            metric->total += metric->samples[j];
        }
        if (!fr_reader_ok(&r)) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "metric sample list is incomplete");
            return FR_E_FORMAT;
        }
        metric->active = 1;
    }
    qsort(doc->metrics, doc->metric_count, sizeof(*doc->metrics), fr_metric_compare_id);
    for (i = 1; i < doc->metric_count; ++i) {
        if (doc->metrics[i - 1].id == doc->metrics[i].id) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "metric table contains duplicate identifiers");
            return FR_E_FORMAT;
        }
    }
    doc->state_bits |= 128u;
    return FR_OK;
}

fr_metric_t *fr_find_metric(fr_document_t *doc, uint16_t id) {
    size_t lo = 0, hi;
    if (doc == NULL || doc->metrics == NULL) return NULL;
    hi = doc->metric_count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (doc->metrics[mid].id == id) return &doc->metrics[mid];
        if (doc->metrics[mid].id < id) lo = mid + 1;
        else hi = mid;
    }
    return NULL;
}


fr_status_t fr_prepare_metric_cache(fr_document_t *doc) {
    size_t i;
    fr_metric_cache_t *cache;
    if (doc == NULL) return FR_E_ARGUMENT;
    cache = &doc->metric_cache;
    cache->count = doc->metric_count;
    cache->metric_ids = (uint16_t *)fr_calloc(doc->ctx, cache->count, sizeof(*cache->metric_ids));
    cache->samples = (int32_t **)fr_calloc(doc->ctx, cache->count, sizeof(*cache->samples));
    cache->sample_counts = (size_t *)fr_calloc(doc->ctx, cache->count, sizeof(*cache->sample_counts));
    if (cache->count != 0 && (cache->metric_ids == NULL || cache->samples == NULL || cache->sample_counts == NULL)) return FR_E_ALLOC;
    for (i = 0; i < cache->count; ++i) {
        cache->metric_ids[i] = doc->metrics[i].id;
        cache->samples[i] = doc->metrics[i].samples;
        cache->sample_counts[i] = doc->metrics[i].sample_count;
    }
    return FR_OK;
}
