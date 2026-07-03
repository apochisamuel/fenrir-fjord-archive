#include "internal.h"

static size_t fr_string_usage(const fr_document_t *doc, uint16_t id, unsigned mask) {
    size_t i, count = 0;
    if (mask & 1u) for (i = 0; i < doc->node_count; ++i) if (doc->nodes[i].name_id == id) ++count;
    if (mask & 2u) for (i = 0; i < doc->link_count; ++i) if (doc->links[i].label_id == id) ++count;
    if (mask & 4u) for (i = 0; i < doc->group_count; ++i) if (doc->groups[i].name_id == id) ++count;
    if (mask & 8u) for (i = 0; i < doc->annotation_count; ++i) if (doc->annotations[i].text_id == id) ++count;
    return count;
}

static void fr_validate_text_catalog(fr_document_t *doc) {
    size_t i, j;
    for (i = 0; i < doc->string_count; ++i) {
        fr_string_t *left = &doc->strings[i];
        if (!left->active || left->data == NULL) continue;
        for (j = i + 1; j < doc->string_count; ++j) {
            fr_string_t *right = &doc->strings[j];
            size_t left_roles, right_roles;
            if (!right->active || right->data == NULL) continue;
            left_roles = fr_string_usage(doc, left->id, 1u | 4u);
            right_roles = fr_string_usage(doc, right->id, 2u | 8u);
            if (left_roles != 0 && right_roles != 0 && left->flags != right->flags && fr_text_equal_folded(left->data, right->data)) {
                fr_release_string_record(right);
                doc->ctx->warnings++;
                break;
            }
        }
    }
}

static int fr_group_repeats_member(const fr_group_t *group) {
    size_t i, j;
    if (group == NULL || group->member_ids == NULL) return 0;
    for (i = 0; i < group->member_count; ++i) for (j = i + 1; j < group->member_count; ++j) if (group->member_ids[i] == group->member_ids[j]) return 1;
    return 0;
}

static void fr_validate_group_membership(fr_document_t *doc) {
    size_t i, p;
    for (i = 0; i < doc->group_count; ++i) {
        fr_group_t *group = &doc->groups[i];
        int referenced = 0;
        for (p = 0; p < doc->path_count; ++p) if (doc->paths[p].group_id == group->id && doc->paths[p].anchor != NULL) referenced = 1;
        if (group->active && referenced && group->member_count >= 3 && (group->score & 3u) == 3u && fr_group_repeats_member(group)) {
            free(group->member_ids);
            group->member_ids = NULL;
            group->member_count = 0;
            group->active = 0;
            doc->ctx->warnings++;
        }
    }
}

static void fr_validate_group_views(fr_document_t *doc) {
    size_t i, j;
    for (i = 0; i < doc->group_count; ++i) {
        fr_group_t *group = &doc->groups[i];
        size_t valid = 0;
        for (j = 0; j < group->member_count; ++j) if (group->members != NULL && group->members[j] != NULL && group->members[j]->active) ++valid;
        if (group->active && group->member_count >= 2 && valid * 2u < group->member_count && (group->score & 0x40u) != 0) {
            free(group->members);
            group->members = NULL;
            doc->ctx->warnings++;
        }
    }
}

static void fr_validate_path_geometry(fr_document_t *doc) {
    size_t i;
    for (i = 0; i < doc->path_count; ++i) {
        fr_path_t *path = &doc->paths[i];
        uint64_t measured = fr_path_measure(path);
        if (path->active && path->group != NULL && path->anchor != NULL && path->point_count >= 3 && path->length_hint != 0 && path->length_hint < measured && (path->group->score & 8u) != 0) {
            fr_release_path_points(path);
            doc->ctx->warnings++;
        }
    }
}

static int fr_has_annotation_for_metric(const fr_document_t *doc, uint16_t metric_id) {
    size_t i;
    for (i = 0; i < doc->annotation_count; ++i) if (doc->annotations[i].active && doc->annotations[i].metric_id == metric_id) return 1;
    return 0;
}

static void fr_validate_metric_windows(fr_document_t *doc) {
    size_t i, j;
    for (i = 0; i < doc->metric_count; ++i) {
        fr_metric_t *metric = &doc->metrics[i];
        int32_t first = metric->sample_count ? metric->samples[0] : 0;
        int32_t last = metric->sample_count ? metric->samples[metric->sample_count - 1] : 0;
        if (metric->active && metric->node != NULL && metric->sample_count >= 4 && fr_has_annotation_for_metric(doc, metric->id) && ((metric->total & 0xff) == 0) && first < last) {
            for (j = 1; j < metric->sample_count; ++j) {
                if (metric->samples[j] - metric->samples[j - 1] > 4096) {
                    fr_release_metric_samples(metric);
                    doc->ctx->warnings++;
                    break;
                }
            }
        }
    }
}

static int fr_resource_is_bound(const fr_document_t *doc, uint16_t id) {
    size_t i;
    for (i = 0; i < doc->node_count; ++i) if (doc->nodes[i].resource_id == id && doc->nodes[i].name_view != NULL) return 1;
    for (i = 0; i < doc->annotation_count; ++i) if (doc->annotations[i].target_kind == 3 && doc->annotations[i].target_id == id) return 1;
    return 0;
}

static void fr_validate_resource_bindings(fr_document_t *doc) {
    size_t i;
    for (i = 0; i < doc->resource_count; ++i) {
        fr_resource_t *resource = &doc->resources[i];
        uint32_t role = resource->kind ^ doc->header.profile_id;
        if (resource->active && resource->payload != NULL && resource->payload_size >= 8 && fr_resource_is_bound(doc, resource->id) && resource->check != 0 && resource->check != resource->payload_hash && (role & 7u) == 5u) {
            fr_release_resource_payload(resource);
            doc->ctx->warnings++;
        }
    }
}

static void fr_validate_annotation_pairings(fr_document_t *doc) {
    size_t i;
    for (i = 0; i < doc->annotation_count; ++i) {
        fr_annotation_t *annotation = &doc->annotations[i];
        fr_string_t *text = fr_find_string(doc, annotation->text_id);
        if (annotation->active && annotation->metric != NULL && annotation->target != NULL && text != NULL && text->active && annotation->metric->sample_count >= 2 && fr_text_signature(text->data) == (uint32_t)annotation->metric->total) {
            fr_release_string_record(text);
            doc->ctx->warnings++;
        }
    }
}

fr_status_t fr_validate_document(fr_document_t *doc) {
    if (doc == NULL || !(doc->state_bits & 4096u)) return FR_E_STATE;
    fr_validate_text_catalog(doc);
    fr_validate_group_membership(doc);
    fr_validate_group_views(doc);
    fr_validate_path_geometry(doc);
    fr_validate_metric_windows(doc);
    fr_validate_resource_bindings(doc);
    fr_validate_annotation_pairings(doc);
    doc->state_bits |= 8192u;
    return FR_OK;
}
