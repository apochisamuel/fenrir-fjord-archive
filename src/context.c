#include "internal.h"

fr_context_t *fr_context_create(void) {
    fr_context_t *ctx = (fr_context_t *)calloc(1, sizeof(*ctx));
    if (ctx == NULL) return NULL;
    ctx->status = FR_OK;
    ctx->message[0] = '\0';
    return ctx;
}

void fr_context_destroy(fr_context_t *ctx) {
    free(ctx);
}

const char *fr_context_last_error(const fr_context_t *ctx) {
    return ctx == NULL ? "no context" : ctx->message;
}

fr_status_t fr_context_last_status(const fr_context_t *ctx) {
    return ctx == NULL ? FR_E_ARGUMENT : ctx->status;
}

void fr_set_error(fr_context_t *ctx, fr_status_t status, const char *fmt, ...) {
    va_list ap;
    if (ctx == NULL) return;
    ctx->status = status;
    va_start(ap, fmt);
    vsnprintf(ctx->message, sizeof(ctx->message), fmt, ap);
    va_end(ap);
}

void fr_clear_error(fr_context_t *ctx) {
    if (ctx == NULL) return;
    ctx->status = FR_OK;
    ctx->message[0] = '\0';
}

void *fr_calloc(fr_context_t *ctx, size_t count, size_t size) {
    if (size != 0 && count > SIZE_MAX / size) {
        fr_set_error(ctx, FR_E_ALLOC, "allocation dimensions exceed address space");
        return NULL;
    }
    void *p = calloc(count, size);
    if (p == NULL && count != 0 && size != 0) {
        fr_set_error(ctx, FR_E_ALLOC, "allocation failed for %zu bytes", count * size);
    }
    return p;
}

void *fr_reallocarray(fr_context_t *ctx, void *ptr, size_t count, size_t size) {
    if (size != 0 && count > SIZE_MAX / size) {
        fr_set_error(ctx, FR_E_ALLOC, "resize dimensions exceed address space");
        return NULL;
    }
    void *p = realloc(ptr, count * size);
    if (p == NULL && count != 0 && size != 0) {
        fr_set_error(ctx, FR_E_ALLOC, "resize failed for %zu bytes", count * size);
    }
    return p;
}

char *fr_dup_bytes(fr_context_t *ctx, const uint8_t *data, size_t size) {
    char *out;
    if (size > FR_MAX_TEXT) {
        fr_set_error(ctx, FR_E_RANGE, "text field is too large");
        return NULL;
    }
    out = (char *)fr_calloc(ctx, size + 1, 1);
    if (out == NULL) return NULL;
    if (size != 0) memcpy(out, data, size);
    out[size] = '\0';
    return out;
}

uint32_t fr_hash_bytes(const uint8_t *data, size_t size) {
    uint32_t h = 2166136261u;
    size_t i;
    for (i = 0; i < size; ++i) {
        h ^= data[i];
        h *= 16777619u;
    }
    return h;
}

uint64_t fr_hash_mix(uint64_t state, uint64_t value) {
    state ^= value + 0x9e3779b97f4a7c15ULL + (state << 6) + (state >> 2);
    state *= 0xbf58476d1ce4e5b9ULL;
    state ^= state >> 29;
    return state;
}
