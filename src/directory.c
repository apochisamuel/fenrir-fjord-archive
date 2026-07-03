#include "internal.h"

static int fr_section_compare(const void *a, const void *b) {
    const fr_section_t *left = (const fr_section_t *)a;
    const fr_section_t *right = (const fr_section_t *)b;
    if (left->tag < right->tag) return -1;
    if (left->tag > right->tag) return 1;
    if (left->offset < right->offset) return -1;
    if (left->offset > right->offset) return 1;
    return 0;
}

fr_status_t fr_parse_directory(fr_document_t *doc, const uint8_t *data, size_t size) {
    fr_reader_t r;
    size_t i;
    if (doc == NULL || data == NULL || !(doc->state_bits & 1u)) return FR_E_STATE;
    doc->sections = (fr_section_t *)fr_calloc(doc->ctx, doc->header.directory_count, sizeof(*doc->sections));
    if (doc->sections == NULL) return FR_E_ALLOC;
    doc->section_count = doc->header.directory_count;
    fr_reader_init(&r, data + doc->header.directory_offset, size - doc->header.directory_offset);
    for (i = 0; i < doc->section_count; ++i) {
        fr_section_t *s = &doc->sections[i];
        s->tag = fr_read_u32(&r);
        s->offset = fr_read_u32(&r);
        s->length = fr_read_u32(&r);
        s->flags = fr_read_u32(&r);
        s->check = fr_read_u32(&r);
        if (!fr_reader_ok(&r) || s->offset > size || s->length > size - s->offset) {
            fr_set_error(doc->ctx, FR_E_RANGE, "section %zu has an invalid range", i);
            return FR_E_RANGE;
        }
    }
    qsort(doc->sections, doc->section_count, sizeof(*doc->sections), fr_section_compare);
    for (i = 1; i < doc->section_count; ++i) {
        const fr_section_t *prior = &doc->sections[i - 1];
        const fr_section_t *current = &doc->sections[i];
        if (prior->tag == current->tag) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "directory includes duplicate section tags");
            return FR_E_FORMAT;
        }
    }
    doc->state_bits |= 2u;
    return FR_OK;
}

const fr_section_t *fr_find_section(const fr_document_t *doc, uint32_t tag) {
    size_t lo = 0, hi;
    if (doc == NULL || doc->sections == NULL) return NULL;
    hi = doc->section_count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (doc->sections[mid].tag == tag) return &doc->sections[mid];
        if (doc->sections[mid].tag < tag) lo = mid + 1;
        else hi = mid;
    }
    return NULL;
}

fr_status_t fr_section_reader(const fr_document_t *doc, uint32_t tag, fr_reader_t *out) {
    const fr_section_t *s;
    if (doc == NULL || out == NULL) return FR_E_ARGUMENT;
    s = fr_find_section(doc, tag);
    if (s == NULL) {
        fr_reader_init(out, NULL, 0);
        return FR_E_FORMAT;
    }
    fr_reader_init(out, doc->source + s->offset, s->length);
    return FR_OK;
}
