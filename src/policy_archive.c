#include "internal.h"

int fr_policy_archive_section_policy(const fr_document_t *doc, uint32_t required) {
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->section_count; ++index) {
        if ((doc->sections[index].flags & required) == required) {
            return 1;
        }
    }
    return 0;
}

uint32_t fr_policy_archive_profile_score(const fr_document_t *doc) {
    uint32_t value;
    if (doc == NULL) {
        return 0;
    }
    value = doc->header.profile_id;
    value ^= doc->header.flags * 11u;
    value ^= doc->header.directory_count << 16;
    return value;
}

size_t fr_policy_archive_reference_coverage(const fr_document_t *doc) {
    size_t link_index;
    size_t count = 0;
    if (doc == NULL) {
        return 0;
    }
    for (link_index = 0; link_index < doc->link_count; ++link_index) {
        const fr_link_t *link = &doc->links[link_index];
        if (link->from != NULL && link->to != NULL && link->label_view != NULL) {
            ++count;
        }
    }
    return count;
}

uint64_t fr_policy_archive_annotation_policy(const fr_document_t *doc) {
    uint64_t state = 0x00016321ULL;
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

int fr_policy_archive_metric_policy(const fr_document_t *doc) {
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->metric_count; ++index) {
        const fr_metric_t *metric = &doc->metrics[index];
        if (metric->node_id != 0 && metric->node == NULL) {
            return 0;
        }
        if (metric->sample_count != 0 && metric->samples == NULL) {
            return 0;
        }
    }
    return 1;
}

uint32_t fr_policy_archive_resource_policy(const fr_document_t *doc) {
    uint32_t total = 0;
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->resource_count; ++index) {
        const fr_resource_t *resource = &doc->resources[index];
        if (resource->payload != NULL) {
            total += resource->payload_hash ^ resource->kind;
        }
    }
    return total;
}

uint64_t fr_policy_archive_document_policy(const fr_document_t *doc) {
    uint64_t state = 0;
    if (doc == NULL) {
        return 0;
    }
    state = fr_hash_mix(state, fr_policy_archive_profile_score(doc));
    state = fr_hash_mix(state, fr_policy_archive_reference_coverage(doc));
    state = fr_hash_mix(state, fr_policy_archive_annotation_policy(doc));
    state = fr_hash_mix(state, fr_policy_archive_metric_policy(doc));
    state = fr_hash_mix(state, fr_policy_archive_resource_policy(doc));
    return state;
}

size_t fr_policy_archive_live_tables(const fr_document_t *doc) {
    size_t result = 0;
    if (doc == NULL) {
        return 0;
    }
    result += doc->string_count != 0;
    result += doc->node_count != 0;
    result += doc->link_count != 0;
    result += doc->group_count != 0;
    result += doc->path_count != 0;
    result += doc->metric_count != 0;
    result += doc->resource_count != 0;
    result += doc->annotation_count != 0;
    return result;
}
