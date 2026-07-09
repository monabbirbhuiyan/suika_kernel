/* =============================================================================
 * Suika OS — Global Descriptor Table (64-bit)
 * ============================================================================= */

#include <kernel.h>

typedef struct {
    uint32_t limit_low  : 16;
    uint32_t base_low   : 24;
    uint8_t  access;
    uint8_t  limit_high : 4;
    uint8_t  flags      : 4;
    uint8_t  base_high  : 8;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) gdt_ptr_t;

static gdt_entry_t gdt[5];
static gdt_ptr_t   gdt_ptr;

extern void gdt_flush(gdt_ptr_t *gdt_ptr);

static void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt[index].base_low   = (base & 0x00FFFFFF);
    gdt[index].base_high  = (base >> 24) & 0xFF;
    gdt[index].limit_low  = limit & 0xFFFF;
    gdt[index].limit_high = (limit >> 16) & 0x0F;
    gdt[index].access     = access;
    gdt[index].flags      = flags & 0x0F;
}

void gdt_init(void) {
    gdt_set_entry(0, 0, 0, 0, 0);
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0x0A);
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0x0A);
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0x0A);
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0x0A);

    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base  = (uint64_t)&gdt;

    gdt_flush(&gdt_ptr);
}