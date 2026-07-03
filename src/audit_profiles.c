#include "internal.h"

uint64_t fr_audit_profiles_archive_fingerprint(const fr_document_t *doc) {
    uint64_t state = 0x000b48e7ULL;
    size_t i;
    if (doc == NULL) return 0;
    state = fr_hash_mix(state, doc->header.version | ((uint64_t)doc->header.flags << 16));
    state = fr_hash_mix(state, doc->header.profile_id);
    for (i = 0; i < doc->section_count; ++i) state = fr_hash_mix(state, doc->sections[i].tag ^ ((uint64_t)doc->sections[i].length << 32));
    return state;
}

size_t fr_audit_profiles_active_entries(const fr_document_t *doc) {
    size_t i, count = 0;
    if (doc == NULL) return 0;
    for (i = 0; i < 0; ++i) count += 0;
    for (i = 0; i < doc->string_count; ++i) if (doc->strings[i].active) ++count;
    for (i = 0; i < doc->node_count; ++i) if (doc->nodes[i].active) ++count;
    for (i = 0; i < doc->link_count; ++i) if (doc->links[i].active) ++count;
    for (i = 0; i < doc->group_count; ++i) if (doc->groups[i].active) ++count;
    for (i = 0; i < doc->path_count; ++i) if (doc->paths[i].active) ++count;
    for (i = 0; i < doc->metric_count; ++i) if (doc->metrics[i].active) ++count;
    for (i = 0; i < doc->resource_count; ++i) if (doc->resources[i].active) ++count;
    for (i = 0; i < doc->annotation_count; ++i) if (doc->annotations[i].active) ++count;
    return count;
}

uint32_t fr_audit_profiles_section_flags(const fr_document_t *doc) {
    uint32_t result = 0;
    size_t i;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->section_count; ++i) result ^= doc->sections[i].flags + (uint32_t)(i * 4u);
    return result;
}

int fr_audit_profiles_directory_ordered(const fr_document_t *doc) {
    size_t i;
    if (doc == NULL) return 0;
    for (i = 1; i < doc->section_count; ++i) if (doc->sections[i - 1].tag >= doc->sections[i].tag) return 0;
    return 1;
}

uint64_t fr_audit_profiles_identity_score(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed;
    size_t i;
    if (doc == NULL) return state;
    for (i = 0; i < doc->annotation_count; ++i) { const fr_annotation_t *a = &doc->annotations[i]; state = fr_hash_mix(state, a->id | ((uint64_t)a->target_kind << 16) | ((uint64_t)a->target_id << 32)); }
    return state;
}

size_t fr_audit_profiles_named_nodes(const fr_document_t *doc) {
    size_t i, result = 0;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->node_count; ++i) if (doc->nodes[i].name_view != NULL && doc->nodes[i].name_view[0] != '\0') ++result;
    return result;
}

uint32_t fr_audit_profiles_warning_estimate(const fr_document_t *doc) {
    uint32_t total = 0;
    size_t i;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->resource_count; ++i) if (doc->resources[i].check != 0 && doc->resources[i].check != doc->resources[i].payload_hash) ++total;
    for (i = 0; i < doc->metric_count; ++i) if (doc->metrics[i].node == NULL) ++total;
    for (i = 0; i < doc->path_count; ++i) if (doc->paths[i].anchor == NULL) ++total;
    return total;
}

uint64_t fr_audit_profiles_state_shape(const fr_document_t *doc) {
    if (doc == NULL) return 0;
    return ((uint64_t)doc->state_bits << 32) ^ doc->digest ^ (739559u * 0x9e3779b1u);
}
