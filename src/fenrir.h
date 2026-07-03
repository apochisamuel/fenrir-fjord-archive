#ifndef FENRIR_H
#define FENRIR_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fr_context fr_context_t;
typedef struct fr_document fr_document_t;

typedef enum fr_status {
    FR_OK = 0,
    FR_E_ARGUMENT = 1,
    FR_E_ALLOC = 2,
    FR_E_FORMAT = 3,
    FR_E_RANGE = 4,
    FR_E_STATE = 5,
    FR_E_CHECKSUM = 6,
    FR_E_INTERNAL = 7
} fr_status_t;

typedef struct fr_node_info {
    uint16_t id;
    int16_t x;
    int16_t y;
    uint16_t name_id;
    uint16_t resource_id;
    uint16_t flags;
} fr_node_info_t;

typedef struct fr_summary {
    uint32_t string_count;
    uint32_t node_count;
    uint32_t link_count;
    uint32_t group_count;
    uint32_t path_count;
    uint32_t metric_count;
    uint32_t resource_count;
    uint32_t annotation_count;
    uint64_t digest;
} fr_summary_t;

fr_context_t *fr_context_create(void);
void fr_context_destroy(fr_context_t *ctx);
const char *fr_context_last_error(const fr_context_t *ctx);
fr_status_t fr_context_last_status(const fr_context_t *ctx);

fr_document_t *fr_document_open_memory(fr_context_t *ctx, const uint8_t *data, size_t size);
void fr_document_destroy(fr_document_t *doc);
fr_status_t fr_document_status(const fr_document_t *doc);

size_t fr_document_node_count(const fr_document_t *doc);
size_t fr_document_string_count(const fr_document_t *doc);
size_t fr_document_resource_count(const fr_document_t *doc);
size_t fr_document_link_count(const fr_document_t *doc);
fr_status_t fr_document_node_at(const fr_document_t *doc, size_t index, fr_node_info_t *out);
fr_status_t fr_document_summary(const fr_document_t *doc, fr_summary_t *out);
uint64_t fr_document_probe(const fr_document_t *doc, uint32_t selector);
fr_status_t fr_document_render_report(const fr_document_t *doc, char *out, size_t cap, size_t *needed);
fr_status_t fr_document_export_json(const fr_document_t *doc, char *out, size_t cap, size_t *needed);

#ifdef __cplusplus
}
#endif

#endif
