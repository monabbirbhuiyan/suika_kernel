#ifndef MEMORY_H
#define MEMORY_H

#include <kernel.h>

/* --- Physical Memory Manager (stack-based free list) --- */
void pmm_init(uint64_t hhdm_offset);
uint64_t pmm_alloc(void);
void pmm_free(uint64_t phys_addr);
uint64_t pmm_get_free_count(void);
uint64_t pmm_get_hhdm(void);

/* --- Virtual Memory Manager --- */
void vmm_init(void);
void vmm_map_page(uint64_t hhdm_offset, uint64_t virt, uint64_t phys, uint64_t flags);
void vmm_unmap_page(uint64_t hhdm_offset, uint64_t virt);
uint64_t vmm_virt_to_phys(uint64_t hhdm_offset, uint64_t virt);

/* --- Kernel Heap --- */
void heap_init(uint64_t hhdm_offset);
void *kmalloc(uint64_t size);
void kfree(void *ptr);

#endif