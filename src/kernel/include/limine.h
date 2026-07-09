/* =============================================================================
 * Suika OS — Limine Boot Protocol Headers (v8+)
 * ============================================================================= */

#ifndef LIMINE_H
#define LIMINE_H

#include "kernel.h"

#define LIMINE_BASE_REVISION(N) \
    uint64_t limine_base_revision[3] = { \
        0xf9562b2d5c95a6c8, 0x6a7b384944536bdc, (N) \
    };

#define LIMINE_COMMON_MAGIC 0xc7b1dd30df4c8b88, 0x0a82e883a194f07b

struct limine_framebuffer {
    void *address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
    uint16_t bpp;
    uint8_t memory_model;
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;
    uint8_t unused[7];
    uint64_t edid_size;
    void *edid;
};

struct limine_framebuffer_response {
    uint64_t revision;
    uint64_t framebuffer_count;
    struct limine_framebuffer **framebuffers;
};

struct limine_framebuffer_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_framebuffer_response *response;
    uint64_t width;
    uint64_t height;
    uint64_t bpp;
};

#define LIMINE_FRAMEBUFFER_REQUEST \
    { LIMINE_COMMON_MAGIC, 0x9d5827dcd881dd75, 0xa3148604f6fab11b }

#endif /* LIMINE_H */
