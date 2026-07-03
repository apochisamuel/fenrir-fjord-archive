#include "internal.h"

size_t fr_document_node_count(const fr_document_t *doc) { return doc == NULL ? 0 : doc->node_count; }
size_t fr_document_string_count(const fr_document_t *doc) { return doc == NULL ? 0 : doc->string_count; }
size_t fr_document_resource_count(const fr_document_t *doc) { return doc == NULL ? 0 : doc->resource_count; }
size_t fr_document_link_count(const fr_document_t *doc) { return doc == NULL ? 0 : doc->link_count; }

fr_status_t fr_document_node_at(const fr_document_t *doc, size_t index, fr_node_info_t *out) {
    const fr_node_t *node;
    if (doc == NULL || out == NULL || index >= doc->node_count) return FR_E_ARGUMENT;
    node = &doc->nodes[index];
    out->id = node->id;
    out->x = node->x;
    out->y = node->y;
    out->name_id = node->name_id;
    out->resource_id = node->resource_id;
    out->flags = node->flags;
    return FR_OK;
}

fr_status_t fr_document_summary(const fr_document_t *doc, fr_summary_t *out) {
    if (doc == NULL || out == NULL) return FR_E_ARGUMENT;
    out->string_count = (uint32_t)doc->string_count;
    out->node_count = (uint32_t)doc->node_count;
    out->link_count = (uint32_t)doc->link_count;
    out->group_count = (uint32_t)doc->group_count;
    out->path_count = (uint32_t)doc->path_count;
    out->metric_count = (uint32_t)doc->metric_count;
    out->resource_count = (uint32_t)doc->resource_count;
    out->annotation_count = (uint32_t)doc->annotation_count;
    out->digest = doc->digest;
    return FR_OK;
}

uint64_t fr_document_probe(const fr_document_t *doc, uint32_t selector) {
    uint64_t value = 0;
    size_t i;
    if (doc == NULL) return 0;
    switch (selector % 6u) {
        case 0:
            value = doc->digest;
            for (i = 0; i < doc->node_count; ++i) value = fr_hash_mix(value, (uint16_t)doc->nodes[i].x | ((uint32_t)(uint16_t)doc->nodes[i].y << 16));
            break;
        case 1:
            for (i = 0; i < doc->link_count; ++i) value = fr_hash_mix(value, doc->links[i].id ^ ((uint32_t)doc->links[i].kind << 16));
            break;
        case 2:
            for (i = 0; i < doc->group_count; ++i) value = fr_hash_mix(value, doc->groups[i].score + doc->groups[i].member_count);
            break;
        case 3:
            for (i = 0; i < doc->path_count; ++i) value = fr_hash_mix(value, fr_path_measure(&doc->paths[i]));
            break;
        case 4:
            for (i = 0; i < doc->metric_count; ++i) value = fr_hash_mix(value, (uint64_t)doc->metrics[i].total);
            break;
        default:
            for (i = 0; i < doc->resource_count; ++i) value = fr_hash_mix(value, doc->resources[i].payload_hash);
            break;
    }
    return value;
}
