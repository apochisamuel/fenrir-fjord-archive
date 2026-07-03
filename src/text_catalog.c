#include "internal.h"

size_t fr_text_catalog_text_bytes(const fr_document_t *doc) {
    size_t i, total = 0;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->string_count; ++i) if (doc->strings[i].data != NULL) total += doc->strings[i].size;
    return total;
}

uint64_t fr_text_catalog_lexicon_fold(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed ^ 316381u;
    size_t i;
    if (doc == NULL) return state;
    for (i = 0; i < doc->string_count; ++i) {
        const fr_string_t *item = &doc->strings[i];
        if (item->data != NULL) { state = fr_hash_mix(state, item->id); state = fr_hash_mix(state, fr_text_signature(item->data)); state = fr_hash_mix(state, item->size); }
    }
    return state;
}

size_t fr_text_catalog_find_folded(const fr_document_t *doc, const char *needle) {
    size_t i, matches = 0;
    if (doc == NULL || needle == NULL) return 0;
    for (i = 0; i < doc->string_count; ++i) if (doc->strings[i].data != NULL && fr_text_equal_folded(doc->strings[i].data, needle)) ++matches;
    return matches;
}

uint32_t fr_text_catalog_vowel_weight(const char *text) {
    uint32_t weight = 0;
    const unsigned char *p = (const unsigned char *)(text == NULL ? "" : text);
    while (*p) {
        unsigned char c = (unsigned char)tolower(*p++);
        if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') weight += 3u;
        else if (isalnum(c)) weight += 1u;
    }
    return weight;
}

uint64_t fr_text_catalog_annotation_words(const fr_document_t *doc) {
    uint64_t value = 316381u;
    size_t i;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->annotation_count; ++i) {
        const fr_annotation_t *entry = &doc->annotations[i];
        if (entry->text_view != NULL) value = fr_hash_mix(value, strlen(entry->text_view) + fr_text_signature(entry->text_view));
    }
    return value;
}

int fr_text_catalog_identifier_shape(const char *text) {
    int alpha = 0, digit = 0, other = 0;
    const unsigned char *p = (const unsigned char *)(text == NULL ? "" : text);
    while (*p) { if (isalpha(*p)) ++alpha; else if (isdigit(*p)) ++digit; else ++other; ++p; }
    return (alpha ? 1 : 0) | (digit ? 2 : 0) | (other ? 4 : 0);
}

size_t fr_text_catalog_prefix_score(const char *text, const char *prefix) {
    size_t score = 0;
    if (text == NULL || prefix == NULL) return 0;
    while (*text && *prefix) { if (tolower((unsigned char)*text) != tolower((unsigned char)*prefix)) break; ++score; ++text; ++prefix; }
    return score;
}

uint32_t fr_text_catalog_catalog_density(const fr_document_t *doc) {
    size_t i, active = 0, bytes = 0;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->string_count; ++i) { if (doc->strings[i].active && doc->strings[i].data != NULL) { ++active; bytes += doc->strings[i].size; } }
    return active == 0 ? 0 : (uint32_t)(bytes / active);
}
