#include "internal.h"

uint64_t fr_profile_archive_header_fold(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed ^ 0x000ce78bULL;
    if (doc == NULL) {
        return state;
    }
    state = fr_hash_mix(state, doc->header.version);
    state = fr_hash_mix(state, doc->header.flags);
    state = fr_hash_mix(state, doc->header.directory_offset);
    state = fr_hash_mix(state, doc->header.directory_count);
    state = fr_hash_mix(state, doc->header.profile_id);
    return state;
}

uint64_t fr_profile_archive_directory_fold(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed;
    size_t index;
    if (doc == NULL) {
        return state;
    }
    for (index = 0; index < doc->section_count; ++index) {
        const fr_section_t *section = &doc->sections[index];
        state = fr_hash_mix(state, section->tag);
        state = fr_hash_mix(state, section->offset);
        state = fr_hash_mix(state, section->length);
        state = fr_hash_mix(state, section->flags);
        state = fr_hash_mix(state, section->check);
    }
    return state;
}

size_t fr_profile_archive_section_bytes(const fr_document_t *doc) {
    size_t index;
    size_t total = 0;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->section_count; ++index) {
        total += doc->sections[index].length;
    }
    return total;
}

int fr_profile_archive_contains(const fr_document_t *doc, uint32_t tag) {
    return fr_find_section(doc, tag) != NULL;
}

uint32_t fr_profile_archive_flag_summary(const fr_document_t *doc) {
    uint32_t value = 0;
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->section_count; ++index) {
        value ^= doc->sections[index].flags;
        value = (value << 1) | (value >> 31);
    }
    return value;
}

size_t fr_profile_archive_overlap_hint(const fr_document_t *doc) {
    size_t left;
    size_t right;
    size_t count = 0;
    if (doc == NULL) {
        return 0;
    }
    for (left = 0; left < doc->section_count; ++left) {
        uint64_t left_end = (uint64_t)doc->sections[left].offset + doc->sections[left].length;
        for (right = left + 1; right < doc->section_count; ++right) {
            uint64_t right_end = (uint64_t)doc->sections[right].offset + doc->sections[right].length;
            if (doc->sections[left].offset < right_end && doc->sections[right].offset < left_end) {
                ++count;
            }
        }
    }
    return count;
}

uint64_t fr_profile_archive_content_mix(const fr_document_t *doc) {
    uint64_t state = fr_profile_archive_header_fold(doc, 845707u);
    state = fr_profile_archive_directory_fold(doc, state);
    if (doc != NULL) {
        state = fr_hash_mix(state, doc->digest);
        state = fr_hash_mix(state, doc->state_bits);
    }
    return state;
}

uint32_t fr_profile_archive_population(const fr_document_t *doc) {
    uint32_t total = 0;
    if (doc == NULL) {
        return 0;
    }
    total += (uint32_t)doc->string_count;
    total += (uint32_t)doc->node_count;
    total += (uint32_t)doc->link_count;
    total += (uint32_t)doc->group_count;
    total += (uint32_t)doc->path_count;
    total += (uint32_t)doc->metric_count;
    total += (uint32_t)doc->resource_count;
    total += (uint32_t)doc->annotation_count;
    return total;
}
