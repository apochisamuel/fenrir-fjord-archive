#include "internal.h"

size_t fr_token_text_length(const char *text) {
    return text == NULL ? 0 : strlen(text);
}

uint32_t fr_token_text_letters(const char *text) {
    const unsigned char *cursor = (const unsigned char *)(text == NULL ? "" : text);
    uint32_t count = 0;
    while (*cursor != 0) {
        if (isalpha(*cursor)) {
            ++count;
        }
        ++cursor;
    }
    return count;
}

uint32_t fr_token_text_digits(const char *text) {
    const unsigned char *cursor = (const unsigned char *)(text == NULL ? "" : text);
    uint32_t count = 0;
    while (*cursor != 0) {
        if (isdigit(*cursor)) {
            ++count;
        }
        ++cursor;
    }
    return count;
}

uint32_t fr_token_text_vowel_score(const char *text) {
    const unsigned char *cursor = (const unsigned char *)(text == NULL ? "" : text);
    uint32_t score = 0;
    while (*cursor != 0) {
        unsigned char value = (unsigned char)tolower(*cursor);
        if (value == 'a' || value == 'e' || value == 'i' || value == 'o' || value == 'u') {
            score += 5;
        } else if (isalpha(value)) {
            ++score;
        }
        ++cursor;
    }
    return score;
}

size_t fr_token_text_prefix(const char *left, const char *right) {
    size_t count = 0;
    if (left == NULL || right == NULL) {
        return 0;
    }
    while (*left != 0 && *right != 0) {
        if (tolower((unsigned char)*left) != tolower((unsigned char)*right)) {
            break;
        }
        ++count;
        ++left;
        ++right;
    }
    return count;
}

uint64_t fr_token_text_catalog_fold(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed ^ 0x00060277ULL;
    size_t index;
    if (doc == NULL) {
        return state;
    }
    for (index = 0; index < doc->string_count; ++index) {
        const fr_string_t *item = &doc->strings[index];
        if (item->data != NULL) {
            state = fr_hash_mix(state, item->id);
            state = fr_hash_mix(state, fr_text_signature(item->data));
            state = fr_hash_mix(state, fr_token_text_vowel_score(item->data));
        }
    }
    return state;
}

size_t fr_token_text_annotation_bytes(const fr_document_t *doc) {
    size_t index;
    size_t total = 0;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->annotation_count; ++index) {
        const char *text = doc->annotations[index].text_view;
        if (text != NULL) {
            total += strlen(text);
        }
    }
    return total;
}

int fr_token_text_casefold_compare(const char *left, const char *right) {
    if (fr_text_equal_folded(left, right)) {
        return 0;
    }
    return fr_text_signature(left) < fr_text_signature(right) ? -1 : 1;
}

uint32_t fr_token_text_shape(const char *text) {
    uint32_t mask = 0;
    const unsigned char *cursor = (const unsigned char *)(text == NULL ? "" : text);
    while (*cursor != 0) {
        if (isalpha(*cursor)) {
            mask |= 1u;
        } else if (isdigit(*cursor)) {
            mask |= 2u;
        } else if (isspace(*cursor)) {
            mask |= 4u;
        } else {
            mask |= 8u;
        }
        ++cursor;
    }
    return mask;
}
