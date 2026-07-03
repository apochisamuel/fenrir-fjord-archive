#include "internal.h"

uint64_t fr_group_workflow_group_fold(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed ^ 0x000ea0d0ULL;
    size_t index;
    if (doc == NULL) {
        return state;
    }
    for (index = 0; index < doc->group_count; ++index) {
        const fr_group_t *group = &doc->groups[index];
        state = fr_hash_mix(state, group->id);
        state = fr_hash_mix(state, group->name_id);
        state = fr_hash_mix(state, group->score);
        state = fr_hash_mix(state, group->member_count);
    }
    return state;
}

size_t fr_group_workflow_group_members(const fr_group_t *group) {
    size_t index;
    size_t count = 0;
    if (group == NULL || group->members == NULL) {
        return 0;
    }
    for (index = 0; index < group->member_count; ++index) {
        if (group->members[index] != NULL && group->members[index]->active) {
            ++count;
        }
    }
    return count;
}

uint32_t fr_group_workflow_path_priority(const fr_path_t *path) {
    uint64_t measure;
    if (path == NULL) {
        return 0;
    }
    measure = fr_path_measure(path);
    measure ^= path->length_hint;
    measure ^= (uint64_t)path->id << 32;
    return (uint32_t)(measure ^ (measure >> 32));
}

int fr_group_workflow_annotation_match(const fr_annotation_t *annotation) {
    if (annotation == NULL) {
        return 0;
    }
    if (!annotation->active) {
        return 0;
    }
    if (annotation->target == NULL) {
        return 0;
    }
    if (annotation->text_view == NULL) {
        return 0;
    }
    return annotation->metric == NULL || annotation->metric->node != NULL;
}

size_t fr_group_workflow_linked_annotations(const fr_document_t *doc, uint16_t target_kind, uint16_t target_id) {
    size_t index;
    size_t count = 0;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->annotation_count; ++index) {
        const fr_annotation_t *annotation = &doc->annotations[index];
        if (annotation->target_kind == target_kind && annotation->target_id == target_id && fr_group_workflow_annotation_match(annotation)) {
            ++count;
        }
    }
    return count;
}

uint64_t fr_group_workflow_annotation_fold(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed;
    size_t index;
    if (doc == NULL) {
        return state;
    }
    for (index = 0; index < doc->annotation_count; ++index) {
        const fr_annotation_t *annotation = &doc->annotations[index];
        state = fr_hash_mix(state, annotation->id);
        state = fr_hash_mix(state, annotation->target_kind);
        state = fr_hash_mix(state, annotation->target_id);
        if (annotation->text_view != NULL) {
            state = fr_hash_mix(state, fr_text_signature(annotation->text_view));
        }
    }
    return state;
}

uint32_t fr_group_workflow_workflow_pressure(const fr_document_t *doc) {
    uint32_t pressure = 0;
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->path_count; ++index) {
        const fr_path_t *path = &doc->paths[index];
        pressure += (uint32_t)path->point_count;
        pressure += fr_group_workflow_path_priority(path) & 255u;
    }
    for (index = 0; index < doc->group_count; ++index) {
        pressure += (uint32_t)fr_group_workflow_group_members(&doc->groups[index]);
    }
    return pressure;
}

int fr_group_workflow_has_active_routes(const fr_document_t *doc) {
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->path_count; ++index) {
        if (doc->paths[index].active && doc->paths[index].anchor != NULL) {
            return 1;
        }
    }
    return 0;
}
