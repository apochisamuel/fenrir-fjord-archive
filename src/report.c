#include "internal.h"

static size_t fr_report_header(const fr_document_t *doc, char *out, size_t cap, size_t at) {
    at = fr_append_format(out, cap, at, "Fjord Archive v%u\n", doc->header.version);
    at = fr_append_format(out, cap, at, "Profile: %u\n", doc->header.profile_id);
    at = fr_append_format(out, cap, at, "Sections: %zu\n", doc->section_count);
    at = fr_append_format(out, cap, at, "Digest: %016llx\n", (unsigned long long)doc->digest);
    return at;
}

static size_t fr_report_nodes(const fr_document_t *doc, char *out, size_t cap, size_t at) {
    size_t i;
    at = fr_append_format(out, cap, at, "Nodes (%zu)\n", doc->node_count);
    for (i = 0; i < doc->node_count; ++i) {
        const fr_node_t *node = &doc->nodes[i];
        at = fr_append_format(out, cap, at, "  %u %d %d %s\n", node->id, node->x, node->y, node->name_view == NULL ? "" : node->name_view);
    }
    return at;
}

static size_t fr_report_links(const fr_document_t *doc, char *out, size_t cap, size_t at) {
    size_t i;
    at = fr_append_format(out, cap, at, "Links (%zu)\n", doc->link_count);
    for (i = 0; i < doc->link_count; ++i) {
        const fr_link_t *link = &doc->links[i];
        at = fr_append_format(out, cap, at, "  %u %u->%u kind=%u weight=%d %s\n", link->id, link->from_id, link->to_id, link->kind, link->weight, link->label_view == NULL ? "" : link->label_view);
    }
    return at;
}

static size_t fr_report_groups(const fr_document_t *doc, char *out, size_t cap, size_t at) {
    size_t i;
    at = fr_append_format(out, cap, at, "Groups (%zu)\n", doc->group_count);
    for (i = 0; i < doc->group_count; ++i) {
        const fr_group_t *group = &doc->groups[i];
        at = fr_append_format(out, cap, at, "  %u members=%u score=%u %s\n", group->id, group->member_count, group->score, group->name_view == NULL ? "" : group->name_view);
    }
    return at;
}

static size_t fr_report_metrics(const fr_document_t *doc, char *out, size_t cap, size_t at) {
    size_t i;
    at = fr_append_format(out, cap, at, "Metrics (%zu)\n", doc->metric_count);
    for (i = 0; i < doc->metric_count; ++i) {
        const fr_metric_t *metric = &doc->metrics[i];
        at = fr_append_format(out, cap, at, "  %u node=%u samples=%u total=%lld\n", metric->id, metric->node_id, metric->sample_count, (long long)metric->total);
    }
    return at;
}

fr_status_t fr_document_render_report(const fr_document_t *doc, char *out, size_t cap, size_t *needed) {
    size_t at;
    if (doc == NULL || needed == NULL) return FR_E_ARGUMENT;
    if (out != NULL && cap != 0) out[0] = '\0';
    at = 0;
    at = fr_report_header(doc, out, cap, at);
    at = fr_report_nodes(doc, out, cap, at);
    at = fr_report_links(doc, out, cap, at);
    at = fr_report_groups(doc, out, cap, at);
    at = fr_report_metrics(doc, out, cap, at);
    *needed = at + 1;
    return out == NULL || cap >= at + 1 ? FR_OK : FR_E_RANGE;
}
