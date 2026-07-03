#include "internal.h"

fr_status_t fr_parse_annotations(fr_document_t *doc) {
    fr_reader_t r;
    uint16_t count;
    size_t i;
    if (fr_section_reader(doc, FR_TAG('A','N','N','O'), &r) != FR_OK) return FR_OK;
    count = fr_read_u16(&r);
    if (!fr_reader_ok(&r) || count > FR_MAX_ITEMS) {
        fr_set_error(doc->ctx, FR_E_RANGE, "annotation table count is outside supported range");
        return FR_E_RANGE;
    }
    doc->annotations = (fr_annotation_t *)fr_calloc(doc->ctx, count, sizeof(*doc->annotations));
    if (doc->annotations == NULL && count != 0) return FR_E_ALLOC;
    doc->annotation_count = count;
    for (i = 0; i < count; ++i) {
        fr_annotation_t *annotation = &doc->annotations[i];
        annotation->id = fr_read_u16(&r);
        annotation->target_kind = fr_read_u16(&r);
        annotation->target_id = fr_read_u16(&r);
        annotation->text_id = fr_read_u16(&r);
        annotation->metric_id = fr_read_u16(&r);
        (void)fr_read_u16(&r);
        annotation->active = 1;
        if (!fr_reader_ok(&r)) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "annotation record is incomplete");
            return FR_E_FORMAT;
        }
    }
    doc->state_bits |= 512u;
    return FR_OK;
}
