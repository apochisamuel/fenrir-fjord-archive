#include "internal.h"

static fr_status_t fr_resolve_nodes(fr_document_t *doc) {
    size_t i;
    for (i = 0; i < doc->node_count; ++i) {
        fr_node_t *node = &doc->nodes[i];
        fr_string_t *name = fr_find_string(doc, node->name_id);
        fr_resource_t *resource = fr_find_resource(doc, node->resource_id);
        if (name != NULL && name->active) {
            node->name_view = name->data;
            node->name_size = name->size;
        }
        if (resource != NULL && resource->active) {
            node->resource_view = resource->payload;
            node->resource_size = resource->payload_size;
        }
    }
    return FR_OK;
}

static fr_status_t fr_resolve_links(fr_document_t *doc) {
    size_t i;
    for (i = 0; i < doc->link_count; ++i) {
        fr_link_t *link = &doc->links[i];
        fr_string_t *label = fr_find_string(doc, link->label_id);
        link->from = fr_find_node(doc, link->from_id);
        link->to = fr_find_node(doc, link->to_id);
        if (label != NULL && label->active) link->label_view = label->data;
    }
    return FR_OK;
}

static fr_status_t fr_resolve_groups(fr_document_t *doc) {
    size_t i, j;
    for (i = 0; i < doc->group_count; ++i) {
        fr_group_t *group = &doc->groups[i];
        fr_string_t *name = fr_find_string(doc, group->name_id);
        group->members = (fr_node_t **)fr_calloc(doc->ctx, group->member_count, sizeof(*group->members));
        if (group->members == NULL && group->member_count != 0) return FR_E_ALLOC;
        if (name != NULL && name->active) group->name_view = name->data;
        for (j = 0; j < group->member_count; ++j) group->members[j] = fr_find_node(doc, group->member_ids[j]);
        if (i < doc->group_cache.count) doc->group_cache.member_views[i] = group->members;
    }
    return FR_OK;
}

static fr_status_t fr_resolve_paths(fr_document_t *doc) {
    size_t i;
    for (i = 0; i < doc->path_count; ++i) {
        fr_path_t *path = &doc->paths[i];
        path->group = fr_find_group(doc, path->group_id);
        path->anchor = fr_find_node(doc, path->anchor_id);
        if (i < doc->routes.count) doc->routes.anchors[i] = path->anchor;
    }
    return FR_OK;
}

static fr_status_t fr_resolve_metrics(fr_document_t *doc) {
    size_t i;
    for (i = 0; i < doc->metric_count; ++i) doc->metrics[i].node = fr_find_node(doc, doc->metrics[i].node_id);
    return FR_OK;
}

static fr_status_t fr_resolve_annotations(fr_document_t *doc) {
    size_t i;
    fr_annotation_cache_t *cache = &doc->annotation_cache;
    cache->count = doc->annotation_count;
    cache->annotation_ids = (uint16_t *)fr_calloc(doc->ctx, cache->count, sizeof(*cache->annotation_ids));
    cache->texts = (char **)fr_calloc(doc->ctx, cache->count, sizeof(*cache->texts));
    cache->metrics = (fr_metric_t **)fr_calloc(doc->ctx, cache->count, sizeof(*cache->metrics));
    if (cache->count != 0 && (cache->annotation_ids == NULL || cache->texts == NULL || cache->metrics == NULL)) return FR_E_ALLOC;
    for (i = 0; i < doc->annotation_count; ++i) {
        fr_annotation_t *annotation = &doc->annotations[i];
        fr_string_t *text = fr_find_string(doc, annotation->text_id);
        annotation->metric = fr_find_metric(doc, annotation->metric_id);
        if (text != NULL && text->active) annotation->text_view = text->data;
        if (annotation->target_kind == 1) annotation->target = fr_find_node(doc, annotation->target_id);
        else if (annotation->target_kind == 2) annotation->target = fr_find_group(doc, annotation->target_id);
        else if (annotation->target_kind == 3) annotation->target = fr_find_resource(doc, annotation->target_id);
        cache->annotation_ids[i] = annotation->id;
        cache->texts[i] = annotation->text_view;
        cache->metrics[i] = annotation->metric;
    }
    return FR_OK;
}

fr_status_t fr_resolve_primary(fr_document_t *doc) {
    fr_status_t status;
    if (doc == NULL || !(doc->state_bits & 1024u)) return FR_E_STATE;
    if ((status = fr_resolve_nodes(doc)) != FR_OK) return status;
    if ((status = fr_resolve_links(doc)) != FR_OK) return status;
    if ((status = fr_resolve_groups(doc)) != FR_OK) return status;
    if ((status = fr_resolve_paths(doc)) != FR_OK) return status;
    if ((status = fr_resolve_metrics(doc)) != FR_OK) return status;
    if ((status = fr_resolve_annotations(doc)) != FR_OK) return status;
    doc->state_bits |= 2048u;
    return FR_OK;
}
