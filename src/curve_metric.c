#include "internal.h"

int64_t fr_curve_metric_sum(const fr_metric_t *metric) {
    int64_t total = 0;
    size_t index;
    if (metric == NULL || metric->samples == NULL) {
        return 0;
    }
    for (index = 0; index < metric->sample_count; ++index) {
        total += metric->samples[index];
    }
    return total;
}

int32_t fr_curve_metric_minimum(const fr_metric_t *metric) {
    int32_t value;
    size_t index;
    if (metric == NULL || metric->samples == NULL || metric->sample_count == 0) {
        return 0;
    }
    value = metric->samples[0];
    for (index = 1; index < metric->sample_count; ++index) {
        if (metric->samples[index] < value) {
            value = metric->samples[index];
        }
    }
    return value;
}

int32_t fr_curve_metric_maximum(const fr_metric_t *metric) {
    int32_t value;
    size_t index;
    if (metric == NULL || metric->samples == NULL || metric->sample_count == 0) {
        return 0;
    }
    value = metric->samples[0];
    for (index = 1; index < metric->sample_count; ++index) {
        if (metric->samples[index] > value) {
            value = metric->samples[index];
        }
    }
    return value;
}

uint64_t fr_curve_metric_energy(const fr_metric_t *metric) {
    uint64_t total = 0;
    size_t index;
    if (metric == NULL || metric->samples == NULL) {
        return 0;
    }
    for (index = 0; index < metric->sample_count; ++index) {
        int64_t sample = metric->samples[index];
        total += (uint64_t)(sample * sample);
        total = fr_hash_mix(total, (uint32_t)sample + 280882u);
    }
    return total;
}

size_t fr_curve_metric_threshold_crossings(const fr_metric_t *metric, int32_t level) {
    size_t index;
    size_t count = 0;
    if (metric == NULL || metric->samples == NULL) {
        return 0;
    }
    for (index = 1; index < metric->sample_count; ++index) {
        int32_t prior = metric->samples[index - 1] - level;
        int32_t current = metric->samples[index] - level;
        if ((prior < 0 && current >= 0) || (prior >= 0 && current < 0)) {
            ++count;
        }
    }
    return count;
}

int fr_curve_metric_trend(const fr_metric_t *metric) {
    size_t index;
    int rise = 0;
    int fall = 0;
    if (metric == NULL || metric->samples == NULL) {
        return 0;
    }
    for (index = 1; index < metric->sample_count; ++index) {
        if (metric->samples[index] > metric->samples[index - 1]) {
            ++rise;
        } else if (metric->samples[index] < metric->samples[index - 1]) {
            ++fall;
        }
    }
    if (rise == fall) {
        return 0;
    }
    return rise > fall ? 1 : -1;
}

uint64_t fr_curve_metric_document_fold(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed ^ 0x00044932ULL;
    size_t index;
    if (doc == NULL) {
        return state;
    }
    for (index = 0; index < doc->metric_count; ++index) {
        const fr_metric_t *metric = &doc->metrics[index];
        state = fr_hash_mix(state, metric->id);
        state = fr_hash_mix(state, metric->node_id);
        state = fr_hash_mix(state, (uint64_t)fr_curve_metric_sum(metric));
        state = fr_hash_mix(state, fr_curve_metric_energy(metric));
    }
    return state;
}

uint32_t fr_curve_metric_spread(const fr_metric_t *metric) {
    int64_t difference;
    if (metric == NULL) {
        return 0;
    }
    difference = (int64_t)fr_curve_metric_maximum(metric) - fr_curve_metric_minimum(metric);
    if (difference < 0) {
        return 0;
    }
    if ((uint64_t)difference > UINT32_MAX) {
        return UINT32_MAX;
    }
    return (uint32_t)difference;
}

size_t fr_curve_metric_attached_annotations(const fr_document_t *doc, uint16_t metric_id) {
    size_t index;
    size_t count = 0;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->annotation_count; ++index) {
        if (doc->annotations[index].active && doc->annotations[index].metric_id == metric_id) {
            ++count;
        }
    }
    return count;
}
