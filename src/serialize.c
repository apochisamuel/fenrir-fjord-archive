#include "internal.h"

static size_t fr_json_key(char *out, size_t cap, size_t at, const char *key) {
    at = fr_append_text(out, cap, at, "\"");
    at += fr_escape_json(key, out == NULL || at >= cap ? NULL : out + at, out == NULL || at >= cap ? 0 : cap - at);
    return fr_append_text(out, cap, at, "\":");
}

static size_t fr_json_nodes(const fr_document_t *doc, char *out, size_t cap, size_t at) {
    size_t i;
    at = fr_append_text(out, cap, at, "[");
    for (i = 0; i < doc->node_count; ++i) {
        const fr_node_t *node = &doc->nodes[i];
        if (i) at = fr_append_text(out, cap, at, ",");
        at = fr_append_format(out, cap, at, "{\"id\":%u,\"x\":%d,\"y\":%d,\"name\":\"", node->id, node->x, node->y);
        at += fr_escape_json(node->name_view == NULL ? "" : node->name_view, out == NULL || at >= cap ? NULL : out + at, out == NULL || at >= cap ? 0 : cap - at);
        at = fr_append_text(out, cap, at, "\"}");
    }
    return fr_append_text(out, cap, at, "]");
}

static size_t fr_json_counts(const fr_document_t *doc, char *out, size_t cap, size_t at) {
    at = fr_append_text(out, cap, at, "{\"strings\":");
    at = fr_append_format(out, cap, at, "%zu", doc->string_count);
    at = fr_append_text(out, cap, at, ",\"links\":");
    at = fr_append_format(out, cap, at, "%zu", doc->link_count);
    at = fr_append_text(out, cap, at, ",\"groups\":");
    at = fr_append_format(out, cap, at, "%zu", doc->group_count);
    at = fr_append_text(out, cap, at, ",\"paths\":");
    at = fr_append_format(out, cap, at, "%zu", doc->path_count);
    at = fr_append_text(out, cap, at, ",\"metrics\":");
    at = fr_append_format(out, cap, at, "%zu", doc->metric_count);
    at = fr_append_text(out, cap, at, ",\"resources\":");
    at = fr_append_format(out, cap, at, "%zu", doc->resource_count);
    return fr_append_text(out, cap, at, "}");
}

fr_status_t fr_document_export_json(const fr_document_t *doc, char *out, size_t cap, size_t *needed) {
    size_t at = 0;
    if (doc == NULL || needed == NULL) return FR_E_ARGUMENT;
    if (out != NULL && cap != 0) out[0] = '\0';
    at = fr_append_text(out, cap, at, "{");
    at = fr_json_key(out, cap, at, "version");
    at = fr_append_format(out, cap, at, "%u,", doc->header.version);
    at = fr_json_key(out, cap, at, "digest");
    at = fr_append_format(out, cap, at, "\"%016llx\",", (unsigned long long)doc->digest);
    at = fr_json_key(out, cap, at, "counts");
    at = fr_json_counts(doc, out, cap, at);
    at = fr_append_text(out, cap, at, ",");
    at = fr_json_key(out, cap, at, "nodes");
    at = fr_json_nodes(doc, out, cap, at);
    at = fr_append_text(out, cap, at, "}");
    *needed = at + 1;
    return out == NULL || cap >= at + 1 ? FR_OK : FR_E_RANGE;
}
