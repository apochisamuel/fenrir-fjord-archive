#include "internal.h"

static uint16_t fr_header_fold(const uint8_t *data, size_t n) {
    uint32_t value = 0x5a5au;
    size_t i;
    for (i = 0; i < n; ++i) {
        value = (value << 3) | (value >> 13);
        value ^= data[i];
        value += (uint32_t)(i * 17u + 11u);
    }
    return (uint16_t)(value ^ (value >> 16));
}

fr_status_t fr_parse_header(fr_document_t *doc, const uint8_t *data, size_t size) {
    fr_reader_t r;
    fr_slice_t magic;
    if (doc == NULL || data == NULL || size < 24) {
        fr_set_error(doc ? doc->ctx : NULL, FR_E_FORMAT, "archive header is incomplete");
        return FR_E_FORMAT;
    }
    doc->source = data;
    doc->source_size = size;
    fr_reader_init(&r, data, size);
    magic = fr_read_slice(&r, 4);
    if (!fr_reader_ok(&r) || memcmp(magic.data, "FJAR", 4) != 0) {
        fr_set_error(doc->ctx, FR_E_FORMAT, "archive magic is not recognized");
        return FR_E_FORMAT;
    }
    doc->header.version = fr_read_u16(&r);
    doc->header.flags = fr_read_u16(&r);
    doc->header.directory_offset = fr_read_u32(&r);
    doc->header.directory_count = fr_read_u16(&r);
    doc->header.header_check = fr_read_u16(&r);
    doc->header.declared_size = fr_read_u32(&r);
    doc->header.profile_id = fr_read_u32(&r);
    if (!fr_reader_ok(&r)) {
        fr_set_error(doc->ctx, FR_E_FORMAT, "archive header field is incomplete");
        return FR_E_FORMAT;
    }
    if (doc->header.version == 0 || doc->header.version > 4) {
        fr_set_error(doc->ctx, FR_E_FORMAT, "archive version is outside supported range");
        return FR_E_FORMAT;
    }
    if (doc->header.directory_count == 0 || doc->header.directory_count > FR_MAX_SECTIONS) {
        fr_set_error(doc->ctx, FR_E_RANGE, "directory count is outside supported range");
        return FR_E_RANGE;
    }
    if (doc->header.directory_offset > size || (size_t)doc->header.directory_count > (size - doc->header.directory_offset) / 20u) {
        fr_set_error(doc->ctx, FR_E_RANGE, "directory range exceeds archive data");
        return FR_E_RANGE;
    }
    if (doc->header.declared_size != 0 && doc->header.declared_size != size) {
        doc->ctx->warnings++;
    }
    if (doc->header.header_check != 0 && doc->header.header_check != fr_header_fold(data, 12)) {
        doc->ctx->warnings++;
    }
    doc->state_bits |= 1u;
    return FR_OK;
}
