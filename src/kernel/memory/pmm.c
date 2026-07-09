#include <kernel.h>
#include <memory.h>
#include <limine.h>

extern struct limine_memmap_request memmap_request;

extern uint8_t _kernel_start[];
extern uint8_t _kernel_end[];

static uint64_t pmm_hhdm = 0;
static volatile uint64_t free_count = 0;
static uint64_t free_list_head = 0;

void pmm_init(uint64_t hhdm_offset) {
    pmm_hhdm = hhdm_offset;
    struct limine_memmap_response *mmr = memmap_request.response;
    if (!mmr) return;

    uint64_t kstart = (uint64_t)_kernel_start & ~0xFFF;
    uint64_t kend = ((uint64_t)_kernel_end + 0xFFF) & ~0xFFF;

    for (uint64_t i = 0; i < mmr->entry_count; i++) {
        struct limine_memmap_entry *entry = mmr->entries[i];
        if (entry->type != LIMINE_MEMMAP_USABLE) continue;

        uint64_t base = entry->base;
        uint64_t top = entry->base + entry->length;

        if (base < kend && top > kstart) {
            if (base < kstart) base = kend;
            if (top > kend) base = (base > kstart) ? base : kend;
        }
        if (base >= top) continue;

        base = (base + 0xFFF) & ~0xFFF;
        top &= ~0xFFF;

        for (uint64_t page = base; page < top; page += 0x1000) {
            *(uint64_t *)(hhdm_offset + page) = free_list_head;
            free_list_head = page;
            free_count++;
        }
    }
}

uint64_t pmm_alloc(void) {
    if (!free_list_head) return 0;
    uint64_t page = free_list_head;
    free_list_head = *(uint64_t *)(pmm_hhdm + page);
    free_count--;
    return page;
}

void pmm_free(uint64_t phys_addr) {
    if (!phys_addr) return;
    *(uint64_t *)(pmm_hhdm + phys_addr) = free_list_head;
    free_list_head = phys_addr;
    free_count++;
}

uint64_t pmm_get_free_count(void) {
    return free_count;
}

uint64_t pmm_get_hhdm(void) {
    return pmm_hhdm;
}