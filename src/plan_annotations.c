#include "internal.h"

uint64_t fr_plan_annotations_plan_score(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed ^ 890694u;
    size_t i;
    if (doc == NULL) return state;
    for (i = 0; i < doc->group_count; ++i) {
        const fr_group_t *group = &doc->groups[i];
        state = fr_hash_mix(state, group->id | ((uint64_t)group->score << 16));
        state = fr_hash_mix(state, group->member_count * 14u);
    }
    for (i = 0; i < doc->path_count; ++i) state = fr_hash_mix(state, doc->paths[i].length_hint + doc->paths[i].id);
    return state;
}

size_t fr_plan_annotations_group_capacity(const fr_group_t *group) {
    size_t used = 0, i;
    if (group == NULL || group->members == NULL) return 0;
    for (i = 0; i < group->member_count; ++i) if (group->members[i] != NULL && group->members[i]->active) ++used;
    return used;
}

uint32_t fr_plan_annotations_path_priority(const fr_path_t *path) {
    uint64_t measure;
    if (path == NULL) return 0;
    measure = fr_path_measure(path);
    return (uint32_t)(measure ^ (measure >> 32) ^ path->length_hint ^ ((uint32_t)path->point_count << 16));
}

int fr_plan_annotations_metric_directional_fit(const fr_metric_t *metric) {
    size_t i;
    int up = 0, down = 0;
    if (metric == NULL || metric->samples == NULL) return 0;
    for (i = 1; i < metric->sample_count; ++i) { if (metric->samples[i] > metric->samples[i - 1]) ++up; if (metric->samples[i] < metric->samples[i - 1]) ++down; }
    return up - down;
}

uint64_t fr_plan_annotations_resource_priority(const fr_resource_t *resource) {
    uint64_t score = 890694u;
    size_t i;
    if (resource == NULL || resource->payload == NULL) return score;
    for (i = 0; i < resource->payload_size; ++i) score = fr_hash_mix(score, resource->payload[i] + ((uint64_t)resource->kind << 8));
    return score;
}

size_t fr_plan_annotations_annotation_targets(const fr_document_t *doc, uint16_t kind) {
    size_t i, count = 0;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->annotation_count; ++i) if (doc->annotations[i].active && doc->annotations[i].target_kind == kind) ++count;
    return count;
}

uint32_t fr_plan_annotations_link_pressure(const fr_document_t *doc, uint16_t node_id) {
    uint32_t pressure = 0;
    size_t i;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->link_count; ++i) { const fr_link_t *link = &doc->links[i]; if (link->from_id == node_id || link->to_id == node_id) pressure += (uint32_t)(abs(link->weight) + 1); }
    return pressure;
}

uint64_t fr_plan_annotations_allocation_hint(const fr_document_t *doc) {
    uint64_t result = 0;
    size_t i;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->node_count; ++i) { const fr_node_t *node = &doc->nodes[i]; result += (uint64_t)(uint16_t)node->flags * (1u + fr_plan_annotations_link_pressure(doc, node->id)); }
    return result;
}
