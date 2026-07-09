#include <kernel.h>

void *memset(void *dst, int c, size_t n) {
    uint8_t *p = (uint8_t *)dst;
    for (size_t i = 0; i < n; i++) p[i] = (uint8_t)c;
    return dst;
}

void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
    return dst;
}

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

int strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}
