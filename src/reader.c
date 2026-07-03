#include "internal.h"

static int fr_reader_need(fr_reader_t *r, size_t n) {
    if (r == NULL || r->failed || n > r->size - r->pos) {
        if (r != NULL) r->failed = 1;
        return 0;
    }
    return 1;
}

void fr_reader_init(fr_reader_t *r, const uint8_t *data, size_t size) {
    r->data = data;
    r->size = size;
    r->pos = 0;
    r->failed = 0;
}

uint8_t fr_read_u8(fr_reader_t *r) {
    if (!fr_reader_need(r, 1)) return 0;
    return r->data[r->pos++];
}

uint16_t fr_read_u16(fr_reader_t *r) {
    uint16_t a = fr_read_u8(r);
    uint16_t b = fr_read_u8(r);
    return (uint16_t)(a | (uint16_t)(b << 8));
}

int16_t fr_read_i16(fr_reader_t *r) {
    return (int16_t)fr_read_u16(r);
}

uint32_t fr_read_u32(fr_reader_t *r) {
    uint32_t a = fr_read_u8(r);
    uint32_t b = fr_read_u8(r);
    uint32_t c = fr_read_u8(r);
    uint32_t d = fr_read_u8(r);
    return a | (b << 8) | (c << 16) | (d << 24);
}

int32_t fr_read_i32(fr_reader_t *r) {
    return (int32_t)fr_read_u32(r);
}

fr_slice_t fr_read_slice(fr_reader_t *r, size_t n) {
    fr_slice_t s;
    s.data = NULL;
    s.size = 0;
    if (!fr_reader_need(r, n)) return s;
    s.data = r->data + r->pos;
    s.size = n;
    r->pos += n;
    return s;
}

void fr_reader_skip(fr_reader_t *r, size_t n) {
    if (!fr_reader_need(r, n)) return;
    r->pos += n;
}

int fr_reader_ok(const fr_reader_t *r) {
    return r != NULL && !r->failed;
}
