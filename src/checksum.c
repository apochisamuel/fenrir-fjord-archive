#include "internal.h"

static uint32_t fr_section_hash(const fr_document_t *doc, const fr_section_t *section) {
    if (doc == NULL || section == NULL || section->offset > doc->source_size || section->length > doc->source_size - section->offset) return 0;
    return fr_hash_bytes(doc->source + section->offset, section->length);
}

fr_status_t fr_verify_checksums(fr_document_t *doc) {
    size_t i;
    if (doc == NULL || !(doc->state_bits & 2048u)) return FR_E_STATE;
    for (i = 0; i < doc->section_count; ++i) {
        const fr_section_t *section = &doc->sections[i];
        uint32_t observed = fr_section_hash(doc, section);
        if (section->check != 0 && observed != section->check) doc->ctx->warnings++;
    }
    for (i = 0; i < doc->resource_count; ++i) {
        fr_resource_t *resource = &doc->resources[i];
        if (resource->check != 0 && resource->check != resource->payload_hash) doc->ctx->warnings++;
    }
    doc->state_bits |= 4096u;
    return FR_OK;
}
