/* =============================================================================
 * Suika OS — Global Descriptor Table (32-bit)
 * ============================================================================= */

#include <kernel.h>

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

static gdt_entry_t gdt[5];
static gdt_ptr_t   gdt_ptr;

extern void gdt_flush(uint32_t gdt_ptr);

static void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt[index].base_low     = base & 0xFFFF;
    gdt[index].base_middle  = (base >> 16) & 0xFF;
    gdt[index].base_high    = (base >> 24) & 0xFF;
    gdt[index].limit_low    = limit & 0xFFFF;
    gdt[index].granularity   = ((limit >> 16) & 0x0F) | (flags << 4);
    gdt[index].access       = access;
}

void gdt_init(void) {
    gdt_set_entry(0, 0, 0, 0, 0);                    /* Null */
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0x0C);     /* Code (4GB) */
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0x0C);     /* Data (4GB) */
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0x0C);     /* User Code */
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0x0C);     /* User Data */

    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base  = (uint32_t)&gdt;

    gdt_flush((uint32_t)&gdt_ptr);
}