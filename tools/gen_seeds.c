#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void put16(unsigned char *out, size_t *at, uint16_t value) {
    out[(*at)++] = (unsigned char)value;
    out[(*at)++] = (unsigned char)(value >> 8);
}

static void put32(unsigned char *out, size_t *at, uint32_t value) {
    out[(*at)++] = (unsigned char)value;
    out[(*at)++] = (unsigned char)(value >> 8);
    out[(*at)++] = (unsigned char)(value >> 16);
    out[(*at)++] = (unsigned char)(value >> 24);
}

int main(int argc, char **argv) {
    unsigned char data[128] = {0};
    size_t at = 0;
    FILE *stream;
    if (argc != 2) return 2;
    memcpy(data + at, "FJAR", 4); at += 4;
    put16(data, &at, 1); put16(data, &at, 0); put32(data, &at, 24); put16(data, &at, 1); put16(data, &at, 0); put32(data, &at, 64); put32(data, &at, 7);
    put32(data, &at, 0x54584554u); put32(data, &at, 44); put32(data, &at, 8); put32(data, &at, 0); put32(data, &at, 0);
    put16(data, &at, 1); put16(data, &at, 1); put16(data, &at, 0); put16(data, &at, 2); data[at++] = 'o'; data[at++] = 'k';
    stream = fopen(argv[1], "wb");
    if (stream == NULL) return 1;
    fwrite(data, 1, 64, stream);
    fclose(stream);
    return 0;
}
