#ifndef FENRIR_INTERNAL_H
#define FENRIR_INTERNAL_H

#include "fenrir.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define FR_TAG(a,b,c,d) ((uint32_t)(uint8_t)(a) | ((uint32_t)(uint8_t)(b) << 8) | ((uint32_t)(uint8_t)(c) << 16) | ((uint32_t)(uint8_t)(d) << 24))
#define FR_MAX_SECTIONS 128u
#define FR_MAX_ITEMS 8192u
#define FR_MAX_TEXT 65535u
#define FR_MIN(a,b) ((a) < (b) ? (a) : (b))
#define FR_MAX(a,b) ((a) > (b) ? (a) : (b))

typedef struct fr_reader {
    const uint8_t *data;
    size_t size;
    size_t pos;
    int failed;
} fr_reader_t;

typedef struct fr_slice {
    const uint8_t *data;
    size_t size;
} fr_slice_t;

typedef struct fr_header {
    uint16_t version;
    uint16_t flags;
    uint32_t directory_offset;
    uint16_t directory_count;
    uint16_t header_check;
    uint32_t declared_size;
    uint32_t profile_id;
} fr_header_t;

typedef struct fr_section {
    uint32_t tag;
    uint32_t offset;
    uint32_t length;
    uint32_t flags;
    uint32_t check;
} fr_section_t;

typedef struct fr_string {
    uint16_t id;
    char *data;
    size_t size;
    uint32_t hash;
    uint16_t flags;
    int active;
} fr_string_t;

typedef struct fr_node {
    uint16_t id;
    int16_t x;
    int16_t y;
    uint16_t name_id;
    uint16_t resource_id;
    uint16_t flags;
    char *name_view;
    size_t name_size;
    uint8_t *resource_view;
    size_t resource_size;
    int active;
} fr_node_t;

typedef struct fr_link {
    uint16_t id;
    uint16_t from_id;
    uint16_t to_id;
    uint16_t label_id;
    uint16_t kind;
    int16_t weight;
    fr_node_t *from;
    fr_node_t *to;
    char *label_view;
    int active;
} fr_link_t;

typedef struct fr_group {
    uint16_t id;
    uint16_t name_id;
    uint16_t member_count;
    uint16_t *member_ids;
    fr_node_t **members;
    char *name_view;
    uint32_t score;
    int active;
} fr_group_t;

typedef struct fr_path {
    uint16_t id;
    uint16_t group_id;
    uint16_t anchor_id;
    uint16_t point_count;
    int16_t *points;
    fr_group_t *group;
    fr_node_t *anchor;
    fr_node_t **point_nodes;
    uint32_t length_hint;
    int active;
} fr_path_t;

typedef struct fr_metric {
    uint16_t id;
    uint16_t node_id;
    uint16_t sample_count;
    int32_t *samples;
    fr_node_t *node;
    int64_t total;
    int active;
} fr_metric_t;

typedef struct fr_resource {
    uint16_t id;
    uint16_t kind;
    uint32_t check;
    uint32_t payload_size;
    uint8_t *payload;
    uint32_t payload_hash;
    int active;
} fr_resource_t;

typedef struct fr_annotation {
    uint16_t id;
    uint16_t target_kind;
    uint16_t target_id;
    uint16_t text_id;
    uint16_t metric_id;
    char *text_view;
    fr_metric_t *metric;
    void *target;
    int active;
} fr_annotation_t;

typedef struct fr_name_cache {
    uint16_t *ids;
    char **values;
    fr_string_t **records;
    size_t count;
} fr_name_cache_t;

typedef struct fr_route_cache {
    uint16_t *path_ids;
    fr_node_t **anchors;
    int16_t **points;
    size_t *point_counts;
    size_t count;
} fr_route_cache_t;

typedef struct fr_metric_cache {
    uint16_t *metric_ids;
    int32_t **samples;
    size_t *sample_counts;
    size_t count;
} fr_metric_cache_t;

typedef struct fr_group_cache {
    uint16_t *group_ids;
    uint16_t **member_ids;
    fr_node_t ***member_views;
    size_t *member_counts;
    size_t count;
} fr_group_cache_t;

typedef struct fr_annotation_cache {
    uint16_t *annotation_ids;
    char **texts;
    fr_metric_t **metrics;
    size_t count;
} fr_annotation_cache_t;

typedef struct fr_resource_cache {
    uint16_t *resource_ids;
    uint8_t **payloads;
    size_t *sizes;
    size_t count;
} fr_resource_cache_t;

typedef struct fr_node_index {
    uint16_t *ids;
    fr_node_t **values;
    size_t count;
} fr_node_index_t;

typedef struct fr_document {
    fr_context_t *ctx;
    const uint8_t *source;
    size_t source_size;
    fr_status_t status;
    fr_header_t header;
    fr_section_t *sections;
    size_t section_count;
    fr_string_t *strings;
    size_t string_count;
    fr_node_t *nodes;
    size_t node_count;
    fr_link_t *links;
    size_t link_count;
    fr_group_t *groups;
    size_t group_count;
    fr_path_t *paths;
    size_t path_count;
    fr_metric_t *metrics;
    size_t metric_count;
    fr_resource_t *resources;
    size_t resource_count;
    fr_annotation_t *annotations;
    size_t annotation_count;
    fr_name_cache_t names;
    fr_route_cache_t routes;
    fr_metric_cache_t metric_cache;
    fr_group_cache_t group_cache;
    fr_annotation_cache_t annotation_cache;
    fr_resource_cache_t resource_cache;
    fr_node_index_t node_index;
    uint64_t digest;
    uint32_t state_bits;
} fr_document_t;

struct fr_context {
    fr_status_t status;
    char message[256];
    uint32_t warnings;
    uint32_t phase;
};

void fr_set_error(fr_context_t *ctx, fr_status_t status, const char *fmt, ...);
void fr_clear_error(fr_context_t *ctx);
void *fr_calloc(fr_context_t *ctx, size_t count, size_t size);
void *fr_reallocarray(fr_context_t *ctx, void *ptr, size_t count, size_t size);
char *fr_dup_bytes(fr_context_t *ctx, const uint8_t *data, size_t size);
uint32_t fr_hash_bytes(const uint8_t *data, size_t size);
uint64_t fr_hash_mix(uint64_t state, uint64_t value);

void fr_reader_init(fr_reader_t *r, const uint8_t *data, size_t size);
uint8_t fr_read_u8(fr_reader_t *r);
uint16_t fr_read_u16(fr_reader_t *r);
int16_t fr_read_i16(fr_reader_t *r);
uint32_t fr_read_u32(fr_reader_t *r);
int32_t fr_read_i32(fr_reader_t *r);
fr_slice_t fr_read_slice(fr_reader_t *r, size_t n);
void fr_reader_skip(fr_reader_t *r, size_t n);
int fr_reader_ok(const fr_reader_t *r);

fr_status_t fr_parse_header(fr_document_t *doc, const uint8_t *data, size_t size);
fr_status_t fr_parse_directory(fr_document_t *doc, const uint8_t *data, size_t size);
const fr_section_t *fr_find_section(const fr_document_t *doc, uint32_t tag);
fr_status_t fr_section_reader(const fr_document_t *doc, uint32_t tag, fr_reader_t *out);
fr_status_t fr_parse_strings(fr_document_t *doc);
fr_status_t fr_parse_nodes(fr_document_t *doc);
fr_status_t fr_parse_links(fr_document_t *doc);
fr_status_t fr_parse_groups(fr_document_t *doc);
fr_status_t fr_parse_paths(fr_document_t *doc);
fr_status_t fr_parse_metrics(fr_document_t *doc);
fr_status_t fr_parse_resources(fr_document_t *doc);
fr_status_t fr_parse_annotations(fr_document_t *doc);
fr_status_t fr_prepare_name_cache(fr_document_t *doc);
fr_status_t fr_prepare_route_cache(fr_document_t *doc);
fr_status_t fr_prepare_metric_cache(fr_document_t *doc);
fr_status_t fr_prepare_group_cache(fr_document_t *doc);
fr_status_t fr_prepare_resource_cache(fr_document_t *doc);
fr_status_t fr_build_caches(fr_document_t *doc);
fr_status_t fr_resolve_primary(fr_document_t *doc);
fr_status_t fr_verify_checksums(fr_document_t *doc);
fr_status_t fr_validate_document(fr_document_t *doc);
fr_status_t fr_resolve_crossrefs(fr_document_t *doc);
void fr_destroy_document_parts(fr_document_t *doc);

fr_string_t *fr_find_string(fr_document_t *doc, uint16_t id);
fr_node_t *fr_find_node(fr_document_t *doc, uint16_t id);
fr_group_t *fr_find_group(fr_document_t *doc, uint16_t id);
fr_metric_t *fr_find_metric(fr_document_t *doc, uint16_t id);
fr_resource_t *fr_find_resource(fr_document_t *doc, uint16_t id);

uint32_t fr_tag_from_text(const char text[4]);
void fr_tag_to_text(uint32_t tag, char out[5]);
uint64_t fr_compute_document_digest(const fr_document_t *doc);
size_t fr_escape_json(const char *src, char *out, size_t cap);
size_t fr_append_text(char *out, size_t cap, size_t at, const char *text);
size_t fr_append_format(char *out, size_t cap, size_t at, const char *fmt, ...);

int fr_compare_u16(const void *a, const void *b);
int fr_compare_node_ptr(const void *a, const void *b);
uint32_t fr_distance_squared(const fr_node_t *a, const fr_node_t *b);
uint64_t fr_path_measure(const fr_path_t *path);
int fr_text_equal_folded(const char *a, const char *b);
uint32_t fr_text_signature(const char *text);

void fr_release_string_record(fr_string_t *record);
void fr_release_path_points(fr_path_t *path);
void fr_release_metric_samples(fr_metric_t *metric);
void fr_release_resource_payload(fr_resource_t *resource);

#endif
