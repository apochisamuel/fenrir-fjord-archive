#include "internal.h"

int fr_text_equal_folded(const char *a, const char *b) {
    unsigned char ca, cb;
    if (a == NULL || b == NULL) return a == b;
    while (*a != '\0' && *b != '\0') {
        ca = (unsigned char)*a++;
        cb = (unsigned char)*b++;
        if (tolower(ca) != tolower(cb)) return 0;
    }
    return *a == '\0' && *b == '\0';
}

uint32_t fr_text_signature(const char *text) {
    uint32_t state = 0x811c9dc5u;
    const unsigned char *p = (const unsigned char *)(text == NULL ? "" : text);
    while (*p != '\0') {
        unsigned char c = (unsigned char)tolower(*p++);
        if (isalnum(c)) {
            state ^= c;
            state *= 16777619u;
        }
    }
    return state;
}

size_t fr_text_word_count(const char *text) {
    size_t count = 0;
    int inside = 0;
    const unsigned char *p = (const unsigned char *)(text == NULL ? "" : text);
    while (*p) {
        if (isalnum(*p)) {
            if (!inside) ++count;
            inside = 1;
        } else inside = 0;
        ++p;
    }
    return count;
}

size_t fr_text_trimmed_size(const char *text) {
    const char *left = text == NULL ? "" : text;
    const char *right;
    while (*left && isspace((unsigned char)*left)) ++left;
    right = left + strlen(left);
    while (right > left && isspace((unsigned char)right[-1])) --right;
    return (size_t)(right - left);
}
