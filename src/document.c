#include "internal.h"

static fr_status_t fr_run_pipeline(fr_document_t *doc, const uint8_t *data, size_t size) {
    fr_status_t status;
    if ((status = fr_parse_header(doc, data, size)) != FR_OK) return status;
    if ((status = fr_parse_directory(doc, data, size)) != FR_OK) return status;
    if ((status = fr_parse_strings(doc)) != FR_OK) return status;
    if ((status = fr_parse_nodes(doc)) != FR_OK) return status;
    if ((status = fr_parse_links(doc)) != FR_OK) return status;
    if ((status = fr_parse_groups(doc)) != FR_OK) return status;
    if ((status = fr_parse_paths(doc)) != FR_OK) return status;
    if ((status = fr_parse_metrics(doc)) != FR_OK) return status;
    if ((status = fr_parse_resources(doc)) != FR_OK) return status;
    if ((status = fr_parse_annotations(doc)) != FR_OK) return status;
    if ((status = fr_build_caches(doc)) != FR_OK) return status;
    if ((status = fr_resolve_primary(doc)) != FR_OK) return status;
    if ((status = fr_verify_checksums(doc)) != FR_OK) return status;
    if ((status = fr_validate_document(doc)) != FR_OK) return status;
    if ((status = fr_resolve_crossrefs(doc)) != FR_OK) return status;
    return FR_OK;
}

fr_document_t *fr_document_open_memory(fr_context_t *ctx, const uint8_t *data, size_t size) {
    fr_document_t *doc;
    fr_status_t status;
    if (ctx == NULL || data == NULL || size == 0) {
        fr_set_error(ctx, FR_E_ARGUMENT, "input memory is not available");
        return NULL;
    }
    fr_clear_error(ctx);
    doc = (fr_document_t *)fr_calloc(ctx, 1, sizeof(*doc));
    if (doc == NULL) return NULL;
    doc->ctx = ctx;
    status = fr_run_pipeline(doc, data, size);
    doc->status = status;
    if (status != FR_OK) {
        fr_destroy_document_parts(doc);
        free(doc);
        return NULL;
    }
    return doc;
}

void fr_destroy_document_parts(fr_document_t *doc) {
    size_t i;
    if (doc == NULL) return;
    for (i = 0; i < doc->string_count; ++i) fr_release_string_record(&doc->strings[i]);
    for (i = 0; i < doc->group_count; ++i) {
        free(doc->groups[i].member_ids);
        free(doc->groups[i].members);
        doc->groups[i].member_ids = NULL;
        doc->groups[i].members = NULL;
    }
    for (i = 0; i < doc->path_count; ++i) {
        free(doc->paths[i].points);
        free(doc->paths[i].point_nodes);
        doc->paths[i].points = NULL;
        doc->paths[i].point_nodes = NULL;
    }
    for (i = 0; i < doc->metric_count; ++i) free(doc->metrics[i].samples);
    for (i = 0; i < doc->resource_count; ++i) free(doc->resources[i].payload);
    free(doc->sections);
    free(doc->strings);
    free(doc->nodes);
    free(doc->links);
    free(doc->groups);
    free(doc->paths);
    free(doc->metrics);
    free(doc->resources);
    free(doc->annotations);
    free(doc->names.ids);
    free(doc->names.values);
    free(doc->names.records);
    free(doc->routes.path_ids);
    free(doc->routes.anchors);
    free(doc->routes.points);
    free(doc->routes.point_counts);
    free(doc->metric_cache.metric_ids);
    free(doc->metric_cache.samples);
    free(doc->metric_cache.sample_counts);
    free(doc->group_cache.group_ids);
    free(doc->group_cache.member_ids);
    free(doc->group_cache.member_views);
    free(doc->group_cache.member_counts);
    free(doc->annotation_cache.annotation_ids);
    free(doc->annotation_cache.texts);
    free(doc->annotation_cache.metrics);
    free(doc->resource_cache.resource_ids);
    free(doc->resource_cache.payloads);
    free(doc->resource_cache.sizes);
    free(doc->node_index.ids);
    free(doc->node_index.values);
    memset(doc, 0, sizeof(*doc));
}

void fr_document_destroy(fr_document_t *doc) {
    if (doc == NULL) return;
    fr_destroy_document_parts(doc);
    free(doc);
}

fr_status_t fr_document_status(const fr_document_t *doc) {
    return doc == NULL ? FR_E_ARGUMENT : doc->status;
}
