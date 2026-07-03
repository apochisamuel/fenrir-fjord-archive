#include "internal.h"

uint32_t fr_tag_from_text(const char text[4]) {
    return FR_TAG(text[0], text[1], text[2], text[3]);
}

void fr_tag_to_text(uint32_t tag, char out[5]) {
    out[0] = (char)(tag & 0xffu);
    out[1] = (char)((tag >> 8) & 0xffu);
    out[2] = (char)((tag >> 16) & 0xffu);
    out[3] = (char)((tag >> 24) & 0xffu);
    out[4] = '\0';
}

size_t fr_append_text(char *out, size_t cap, size_t at, const char *text) {
    size_t n = text == NULL ? 0 : strlen(text);
    if (out != NULL && cap > 0 && at < cap) {
        size_t space = cap - at - 1;
        size_t copy = FR_MIN(space, n);
        if (copy != 0) memcpy(out + at, text, copy);
        out[at + copy] = '\0';
    }
    return at + n;
}

size_t fr_append_format(char *out, size_t cap, size_t at, const char *fmt, ...) {
    va_list ap;
    int result;
    va_start(ap, fmt);
    if (out != NULL && cap > 0 && at < cap) {
        result = vsnprintf(out + at, cap - at, fmt, ap);
    } else {
        result = vsnprintf(NULL, 0, fmt, ap);
    }
    va_end(ap);
    if (result < 0) return at;
    return at + (size_t)result;
}

size_t fr_escape_json(const char *src, char *out, size_t cap) {
    size_t at = 0;
    const unsigned char *p = (const unsigned char *)(src == NULL ? "" : src);
    while (*p) {
        const char *piece = NULL;
        char single[7];
        if (*p == '"') piece = "\\\"";
        else if (*p == '\\') piece = "\\\\";
        else if (*p == '\n') piece = "\\n";
        else if (*p == '\r') piece = "\\r";
        else if (*p == '\t') piece = "\\t";
        else if (*p < 0x20u) {
            snprintf(single, sizeof(single), "\\u%04x", *p);
            piece = single;
        } else {
            single[0] = (char)*p;
            single[1] = '\0';
            piece = single;
        }
        at = fr_append_text(out, cap, at, piece);
        ++p;
    }
    return at;
}
