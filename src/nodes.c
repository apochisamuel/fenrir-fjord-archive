#include "internal.h"

static int fr_node_compare_id(const void *left, const void *right) {
    const fr_node_t *a = (const fr_node_t *)left;
    const fr_node_t *b = (const fr_node_t *)right;
    return a->id < b->id ? -1 : a->id > b->id;
}

fr_status_t fr_parse_nodes(fr_document_t *doc) {
    fr_reader_t r;
    uint16_t count;
    size_t i;
    if (fr_section_reader(doc, FR_TAG('N','O','D','E'), &r) != FR_OK) return FR_OK;
    count = fr_read_u16(&r);
    if (!fr_reader_ok(&r) || count > FR_MAX_ITEMS) {
        fr_set_error(doc->ctx, FR_E_RANGE, "node table count is outside supported range");
        return FR_E_RANGE;
    }
    doc->nodes = (fr_node_t *)fr_calloc(doc->ctx, count, sizeof(*doc->nodes));
    if (doc->nodes == NULL && count != 0) return FR_E_ALLOC;
    doc->node_count = count;
    for (i = 0; i < count; ++i) {
        fr_node_t *node = &doc->nodes[i];
        node->id = fr_read_u16(&r);
        node->x = fr_read_i16(&r);
        node->y = fr_read_i16(&r);
        node->name_id = fr_read_u16(&r);
        node->resource_id = fr_read_u16(&r);
        node->flags = fr_read_u16(&r);
        node->active = 1;
        if (!fr_reader_ok(&r)) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "node record is incomplete");
            return FR_E_FORMAT;
        }
    }
    qsort(doc->nodes, doc->node_count, sizeof(*doc->nodes), fr_node_compare_id);
    for (i = 1; i < doc->node_count; ++i) {
        if (doc->nodes[i - 1].id == doc->nodes[i].id) {
            fr_set_error(doc->ctx, FR_E_FORMAT, "node table contains duplicate identifiers");
            return FR_E_FORMAT;
        }
    }
    doc->state_bits |= 8u;
    return FR_OK;
}

fr_node_t *fr_find_node(fr_document_t *doc, uint16_t id) {
    size_t lo = 0, hi;
    if (doc == NULL || doc->nodes == NULL) return NULL;
    hi = doc->node_count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (doc->nodes[mid].id == id) return &doc->nodes[mid];
        if (doc->nodes[mid].id < id) lo = mid + 1;
        else hi = mid;
    }
    return NULL;
}
