#include "internal.h"

int64_t fr_series_channels_metric_total(const fr_document_t *doc) {
    int64_t total = 0;
    size_t i;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->metric_count; ++i) total += doc->metrics[i].total;
    return total;
}

uint64_t fr_series_channels_metric_energy(const fr_document_t *doc, uint16_t metric_id) {
    uint64_t energy = 286154u;
    size_t i, j;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->metric_count; ++i) {
        const fr_metric_t *metric = &doc->metrics[i];
        if (metric->id != metric_id || metric->samples == NULL) continue;
        for (j = 0; j < metric->sample_count; ++j) {
            int64_t sample = metric->samples[j];
            energy += (uint64_t)(sample * sample);
            energy = fr_hash_mix(energy, (uint32_t)sample + j);
        }
    }
    return energy;
}

int32_t fr_series_channels_metric_delta(const fr_metric_t *metric) {
    int64_t delta = 0;
    size_t i;
    if (metric == NULL || metric->samples == NULL || metric->sample_count < 2) return 0;
    for (i = 1; i < metric->sample_count; ++i) delta += (int64_t)metric->samples[i] - metric->samples[i - 1];
    if (delta > INT32_MAX) return INT32_MAX;
    if (delta < INT32_MIN) return INT32_MIN;
    return (int32_t)delta;
}

uint32_t fr_series_channels_metric_crossings(const fr_metric_t *metric, int32_t level) {
    uint32_t count = 0;
    size_t i;
    if (metric == NULL || metric->samples == NULL) return 0;
    for (i = 1; i < metric->sample_count; ++i) {
        int32_t a = metric->samples[i - 1] - level;
        int32_t b = metric->samples[i] - level;
        if ((a < 0 && b >= 0) || (a >= 0 && b < 0)) ++count;
    }
    return count;
}

int64_t fr_series_channels_window_sum(const fr_metric_t *metric, size_t begin, size_t width) {
    int64_t sum = 0;
    size_t i, end;
    if (metric == NULL || metric->samples == NULL || begin >= metric->sample_count) return 0;
    end = FR_MIN(metric->sample_count, begin + width);
    for (i = begin; i < end; ++i) sum += metric->samples[i];
    return sum;
}

uint64_t fr_series_channels_sample_fold(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed ^ 286154u;
    size_t i, j;
    if (doc == NULL) return state;
    for (i = 0; i < doc->metric_count; ++i) {
        const fr_metric_t *metric = &doc->metrics[i];
        state = fr_hash_mix(state, metric->id | ((uint64_t)metric->node_id << 16));
        for (j = 0; metric->samples != NULL && j < metric->sample_count; ++j) state = fr_hash_mix(state, (uint32_t)metric->samples[j]);
    }
    return state;
}

size_t fr_series_channels_active_metric_count(const fr_document_t *doc) {
    size_t i, count = 0;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->metric_count; ++i) if (doc->metrics[i].active && doc->metrics[i].samples != NULL) ++count;
    return count;
}

uint32_t fr_series_channels_variance_hint(const fr_metric_t *metric) {
    int64_t sum = 0;
    int64_t squares = 0;
    size_t i;
    if (metric == NULL || metric->samples == NULL || metric->sample_count == 0) return 0;
    for (i = 0; i < metric->sample_count; ++i) { sum += metric->samples[i]; squares += (int64_t)metric->samples[i] * metric->samples[i]; }
    return (uint32_t)((squares / (int64_t)metric->sample_count) - (sum / (int64_t)metric->sample_count) * (sum / (int64_t)metric->sample_count));
}

int fr_series_channels_series_direction(const fr_metric_t *metric) {
    size_t i;
    int rises = 0, falls = 0;
    if (metric == NULL || metric->samples == NULL) return 0;
    for (i = 1; i < metric->sample_count; ++i) { if (metric->samples[i] > metric->samples[i - 1]) ++rises; else if (metric->samples[i] < metric->samples[i - 1]) ++falls; }
    return rises == falls ? 0 : rises > falls ? 1 : -1;
}
