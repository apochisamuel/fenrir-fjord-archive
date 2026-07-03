#include "fenrir.h"
#include <stdio.h>
#include <stdlib.h>

static unsigned char *read_file(const char *path, size_t *size) {
    FILE *stream = fopen(path, "rb");
    unsigned char *data;
    long end;
    if (stream == NULL) return NULL;
    if (fseek(stream, 0, SEEK_END) != 0) { fclose(stream); return NULL; }
    end = ftell(stream);
    if (end < 0 || fseek(stream, 0, SEEK_SET) != 0) { fclose(stream); return NULL; }
    data = (unsigned char *)malloc((size_t)end);
    if (data == NULL && end != 0) { fclose(stream); return NULL; }
    if ((size_t)end != fread(data, 1, (size_t)end, stream)) { free(data); fclose(stream); return NULL; }
    fclose(stream);
    *size = (size_t)end;
    return data;
}

int main(int argc, char **argv) {
    fr_context_t *ctx;
    fr_document_t *doc;
    unsigned char *data;
    size_t size;
    size_t needed;
    char buffer[4096];
    if (argc != 2) return 2;
    data = read_file(argv[1], &size);
    if (data == NULL) return 1;
    ctx = fr_context_create();
    if (ctx == NULL) { free(data); return 1; }
    doc = fr_document_open_memory(ctx, data, size);
    if (doc == NULL) { fprintf(stderr, "%s\n", fr_context_last_error(ctx)); fr_context_destroy(ctx); free(data); return 1; }
    if (fr_document_render_report(doc, buffer, sizeof(buffer), &needed) == FR_OK) fputs(buffer, stdout);
    fr_document_destroy(doc);
    fr_context_destroy(ctx);
    free(data);
    return 0;
}
