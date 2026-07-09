#include <kernel.h>
#include <memory.h>

#define HEAP_PAGES 256

typedef struct heap_block {
    size_t size;
    struct heap_block *next;
    int free;
} heap_block_t;

static heap_block_t *heap_head = NULL;

void heap_init(uint64_t hhdm_offset) {
    uint64_t heap_virt = 0xFFFFFFFF81000000;
    for (int i = 0; i < HEAP_PAGES; i++) {
        uint64_t phys = pmm_alloc();
        if (!phys) return;
        vmm_map_page(hhdm_offset, heap_virt + i * 0x1000, phys, 0x23);
    }

    heap_head = (heap_block_t *)heap_virt;
    heap_head->size = HEAP_PAGES * 0x1000 - sizeof(heap_block_t);
    heap_head->next = NULL;
    heap_head->free = 1;
}

void *kmalloc(uint64_t size) {
    if (!heap_head || !size) return NULL;
    size = (size + 7) & ~7;

    heap_block_t *cur = heap_head;
    while (cur && !(cur->free && cur->size >= size)) {
        cur = cur->next;
    }
    if (!cur) return NULL;

    if (cur->size > size + sizeof(heap_block_t) + 16) {
        heap_block_t *newb = (heap_block_t *)((uint8_t *)cur + sizeof(heap_block_t) + size);
        newb->size = cur->size - size - sizeof(heap_block_t);
        newb->next = cur->next;
        newb->free = 1;
        cur->size = size;
        cur->next = newb;
    }

    cur->free = 0;
    return (void *)((uint8_t *)cur + sizeof(heap_block_t));
}

void kfree(void *ptr) {
    if (!ptr) return;
    heap_block_t *block = (heap_block_t *)((uint8_t *)ptr - sizeof(heap_block_t));
    block->free = 1;
    for (heap_block_t *b = heap_head; b && b->next; b = b->next) {
        if (b->free && b->next->free) {
            b->size += sizeof(heap_block_t) + b->next->size;
            b->next = b->next->next;
        }
    }
}