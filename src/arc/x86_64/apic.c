#include <kernel.h>

#define APIC_LINT0  0x350

__attribute__((aligned(4096))) static uint8_t pt_pool[3][4096];
static int pt_pool_idx = 0;

static uint64_t phys_mask(uint64_t entry) {
    return entry & 0x000FFFFFFFFFF000;
}

static uint64_t virt_to_phys(uint64_t hhdm_offset, uint64_t virt) {
    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    uint64_t *pml4 = (uint64_t *)(hhdm_offset + (cr3 & ~0xFFF));
    uint64_t pml4e = pml4[(virt >> 39) & 0x1FF];
    if (!(pml4e & 1)) return 0;
    uint64_t *pdpt = (uint64_t *)(hhdm_offset + phys_mask(pml4e));
    uint64_t pdpte = pdpt[(virt >> 30) & 0x1FF];
    if (!(pdpte & 1)) return 0;
    if (pdpte & 0x80) return (pdpte & 0x000FFFFFC0000000) + (virt & 0x3FFFFFFF);
    uint64_t *pd = (uint64_t *)(hhdm_offset + phys_mask(pdpte));
    uint64_t pde = pd[(virt >> 21) & 0x1FF];
    if (!(pde & 1)) return 0;
    if (pde & 0x80) return phys_mask(pde) + (virt & 0x1FFFFF);
    uint64_t *pt = (uint64_t *)(hhdm_offset + phys_mask(pde));
    uint64_t pte = pt[(virt >> 12) & 0x1FF];
    if (!(pte & 1)) return 0;
    return phys_mask(pte) + (virt & 0xFFF);
}

static uint64_t pool_alloc_phys(uint64_t hhdm_offset) {
    if (pt_pool_idx >= 3) return 0;
    uint64_t virt = (uint64_t)&pt_pool[pt_pool_idx][0];
    pt_pool_idx++;
    return virt_to_phys(hhdm_offset, virt);
}

static void flush_tlb(void) {
    __asm__ volatile("mov %%cr3, %%rax; mov %%rax, %%cr3" ::: "rax", "memory");
}

void apic_init(uint64_t hhdm_offset) {
    uint32_t eax, edx;
    __asm__ volatile("rdmsr" : "=a"(eax), "=d"(edx) : "c"(0x1B));
    uint64_t apic_phys = (((uint64_t)edx << 32) | eax) & 0xFFFFFF000;

    uint64_t apic_virt = 0xFFFFFFFFFEE00000;

    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    uint64_t *pml4 = (uint64_t *)(hhdm_offset + (cr3 & ~0xFFF));

    uint64_t *pml4e = &pml4[(apic_virt >> 39) & 0x1FF];
    if (!(*pml4e & 1)) {
        uint64_t paddr = pool_alloc_phys(hhdm_offset);
        if (!paddr) { serial_puts("[APIC] failed: no pool for PDPT\n"); return; }
        *pml4e = paddr | 0x03;
        for (int j = 0; j < 4096; j++) ((uint8_t *)(hhdm_offset + paddr))[j] = 0;
    }

    uint64_t *pdpt = (uint64_t *)(hhdm_offset + phys_mask(*pml4e));
    uint64_t *pdpte = &pdpt[(apic_virt >> 30) & 0x1FF];
    if (!(*pdpte & 1)) {
        uint64_t paddr = pool_alloc_phys(hhdm_offset);
        if (!paddr) { serial_puts("[APIC] failed: no pool for PD\n"); return; }
        *pdpte = paddr | 0x03;
        for (int j = 0; j < 4096; j++) ((uint8_t *)(hhdm_offset + paddr))[j] = 0;
    } else if (*pdpte & 0x80) {
        serial_puts("[APIC] failed: PDPT 1GB page conflict\n");
        return;
    }

    uint64_t *pd = (uint64_t *)(hhdm_offset + phys_mask(*pdpte));
    uint64_t *pde = &pd[(apic_virt >> 21) & 0x1FF];
    if (!(*pde & 1)) {
        uint64_t paddr = pool_alloc_phys(hhdm_offset);
        if (!paddr) { serial_puts("[APIC] failed: no pool for PT\n"); return; }
        *pde = paddr | 0x03;
        for (int j = 0; j < 4096; j++) ((uint8_t *)(hhdm_offset + paddr))[j] = 0;
    } else if (*pde & 0x80) {
        uint64_t huge_phys = *pde & ~0x1FFFFF;
        uint64_t pt_paddr = pool_alloc_phys(hhdm_offset);
        if (!pt_paddr) { serial_puts("[APIC] failed: no pool to split 2MB\n"); return; }
        uint64_t *new_pt = (uint64_t *)(hhdm_offset + pt_paddr);
        for (int i = 0; i < 512; i++) {
            new_pt[i] = (huge_phys + (i << 12)) | 0x23;
        }
        *pde = pt_paddr | 0x03;
    }

    uint64_t *pt = (uint64_t *)(hhdm_offset + phys_mask(*pde));
    uint64_t *pte = &pt[(apic_virt >> 12) & 0x1FF];
    *pte = (apic_phys & ~0xFFF) | 0x23;

    flush_tlb();

    volatile uint32_t *apic = (volatile uint32_t *)apic_virt;
    apic[APIC_LINT0 / 4] = 0x700;
}