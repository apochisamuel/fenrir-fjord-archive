#include "fenrir.h"
#include <stdio.h>
#include <stdlib.h>

static unsigned char *load(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    long n;
    unsigned char *data;
    if (f == NULL) return NULL;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n < 0) { fclose(f); return NULL; }
    data = (unsigned char *)malloc((size_t)n);
    if (data == NULL && n != 0) { fclose(f); return NULL; }
    if (fread(data, 1, (size_t)n, f) != (size_t)n) { free(data); fclose(f); return NULL; }
    fclose(f);
    *out_size = (size_t)n;
    return data;
}

int main(int argc, char **argv) {
    fr_context_t *ctx;
    fr_document_t *doc;
    fr_summary_t summary;
    unsigned char *data;
    size_t size;
    if (argc != 2) return 2;
    data = load(argv[1], &size);
    if (data == NULL) return 1;
    ctx = fr_context_create();
    doc = fr_document_open_memory(ctx, data, size);
    if (doc == NULL) { free(data); fr_context_destroy(ctx); return 1; }
    if (fr_document_summary(doc, &summary) != FR_OK) { fr_document_destroy(doc); fr_context_destroy(ctx); free(data); return 1; }
    if (summary.string_count == 0) { fr_document_destroy(doc); fr_context_destroy(ctx); free(data); return 1; }
    fr_document_destroy(doc);
    fr_context_destroy(ctx);
    free(data);
    return 0;
}
