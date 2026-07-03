#include "internal.h"

fr_status_t fr_parse_links(fr_document_t *doc) {
    fr_reader_t r;
    uint16_t count;
    size_t i;
    if (fr_section_reader(doc, FR_TAG('L','I','N','K'), &r) != FR_OK) return FR_OK;
    count = fr_read_u16(&r);
    if (!fr_reader_ok(&r) || count > FR_MAX_ITEMS) {
        fr_set_error(doc->ctx, FR_E_RANGE, "link table count is outside supported range");
        return FR_E_RANGE;
    }
    doc->links = (fr_link_t *)fr_calloc(doc->ctx, count, sizeof(*doc->links));
    if (doc->links == NULL && count != 0) return FR_E_ALLOC;
    doc->link_count = count;
    for (i = 0; i < count; ++i) {
        fr_link_t *link = &doc->links[i];
        link->id = fr_read_u16(&r);
        link->from_id = fr_read_u16(&r);
        link->to_id = fr_read_u16(&r);
        link->label_id = fr_read_u16(&r);
        link->kind = fr_read_u16(&r);
        link->weight = fr_read_i16(&r);
        link->active = 1;
        if (!fr_reader_ok(&r)) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "link record is incomplete");
            return FR_E_FORMAT;
        }
    }
    doc->state_bits |= 16u;
    return FR_OK;
}
