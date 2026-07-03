#include "fenrir.h"
#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 64 || size > 16*1024*1024) return 0;
    fr_context_t *ctx = fr_context_create();
    if (ctx == nullptr) return 0;
    fr_document_t *doc = fr_document_open_memory(ctx, data, size);
    if (doc != nullptr) {
        size_t count = fr_document_node_count(doc);
        for (size_t index = 0; index < count && index < 32; ++index) {
            fr_node_info_t node{};
            (void)fr_document_node_at(doc, index, &node);
            (void)fr_document_probe(doc, node.id ^ node.flags);
        }
        (void)fr_document_link_count(doc);
        fr_document_destroy(doc);
    }
    fr_context_destroy(ctx);
    return 0;
}
