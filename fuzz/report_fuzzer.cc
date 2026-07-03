#include "fenrir.h"
#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 64 || size > 16*1024*1024) return 0;
    fr_context_t *ctx = fr_context_create();
    if (ctx == nullptr) return 0;
    fr_document_t *doc = fr_document_open_memory(ctx, data, size);
    if (doc != nullptr) {
        char report[2048];
        char json[2048];
        size_t needed = 0;
        (void)fr_document_render_report(doc, report, sizeof(report), &needed);
        (void)fr_document_export_json(doc, json, sizeof(json), &needed);
        (void)fr_document_resource_count(doc);
        fr_document_destroy(doc);
    }
    fr_context_destroy(ctx);
    return 0;
}
