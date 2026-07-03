#include "fenrir.h"
#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 64 || size > 16*1024*1024) return 0;
    fr_context_t *ctx = fr_context_create();
    if (ctx == nullptr) return 0;
    fr_document_t *doc = fr_document_open_memory(ctx, data, size);
    if (doc != nullptr) {
        fr_summary_t summary{};
        (void)fr_document_summary(doc, &summary);
        (void)fr_document_probe(doc, static_cast<uint32_t>(summary.node_count + summary.link_count));
        fr_document_destroy(doc);
    }
    fr_context_destroy(ctx);
    return 0;
}
