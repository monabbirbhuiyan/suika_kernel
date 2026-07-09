#include <kernel.h>
#include <memory.h>

#define PML4_I(v)  (((v) >> 39) & 0x1FF)
#define PDPT_I(v)  (((v) >> 30) & 0x1FF)
#define PD_I(v)    (((v) >> 21) & 0x1FF)
#define PT_I(v)    (((v) >> 12) & 0x1FF)

static uint64_t phys_mask(uint64_t e) {
    return e & 0x000FFFFFFFFFF000;
}

static uint64_t get_cr3(void) {
    uint64_t v;
    __asm__ volatile("mov %%cr3, %0" : "=r"(v));
    return v;
}

static void do_invlpg(uint64_t v) {
    __asm__ volatile("invlpg (%0)" :: "r"(v) : "memory");
}

static uint64_t ensure_table(uint64_t hhdm, uint64_t *entry, uint64_t flags) {
    if (*entry & 1) return phys_mask(*entry);
    uint64_t p = pmm_alloc();
    if (!p) return 0;
    for (int i = 0; i < 4096; i++) ((uint8_t *)(hhdm + p))[i] = 0;
    *entry = p | flags;
    return p;
}

void vmm_init(void) {}

void vmm_map_page(uint64_t hhdm, uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t cr3 = get_cr3();
    uint64_t *pml4 = (uint64_t *)(hhdm + (cr3 & ~0xFFF));

    uint64_t *p4e = &pml4[PML4_I(virt)];
    uint64_t pdpt_pa = ensure_table(hhdm, p4e, 0x03);
    if (!pdpt_pa) return;

    uint64_t *pdpt = (uint64_t *)(hhdm + pdpt_pa);
    uint64_t *p3e = &pdpt[PDPT_I(virt)];
    if (*p3e & 0x80) return;

    uint64_t pd_pa = ensure_table(hhdm, p3e, 0x03);
    if (!pd_pa) return;

    uint64_t *pd = (uint64_t *)(hhdm + pd_pa);
    uint64_t *p2e = &pd[PD_I(virt)];
    if (*p2e & 0x80) return;

    uint64_t pt_pa = ensure_table(hhdm, p2e, 0x03);
    if (!pt_pa) return;

    uint64_t *pt = (uint64_t *)(hhdm + pt_pa);
    pt[PT_I(virt)] = (phys & ~0xFFF) | flags;
    do_invlpg(virt);
}

void vmm_unmap_page(uint64_t hhdm, uint64_t virt) {
    uint64_t cr3 = get_cr3();
    uint64_t *pml4 = (uint64_t *)(hhdm + (cr3 & ~0xFFF));
    uint64_t p4e = pml4[PML4_I(virt)];
    if (!(p4e & 1)) return;

    uint64_t *pdpt = (uint64_t *)(hhdm + phys_mask(p4e));
    uint64_t p3e = pdpt[PDPT_I(virt)];
    if (!(p3e & 1) || (p3e & 0x80)) return;

    uint64_t *pd = (uint64_t *)(hhdm + phys_mask(p3e));
    uint64_t p2e = pd[PD_I(virt)];
    if (!(p2e & 1) || (p2e & 0x80)) return;

    uint64_t *pt = (uint64_t *)(hhdm + phys_mask(p2e));
    pt[PT_I(virt)] = 0;
    do_invlpg(virt);
}

uint64_t vmm_virt_to_phys(uint64_t hhdm, uint64_t virt) {
    uint64_t cr3 = get_cr3();
    uint64_t *pml4 = (uint64_t *)(hhdm + (cr3 & ~0xFFF));
    uint64_t p4e = pml4[PML4_I(virt)];
    if (!(p4e & 1)) return 0;

    uint64_t *pdpt = (uint64_t *)(hhdm + phys_mask(p4e));
    uint64_t p3e = pdpt[PDPT_I(virt)];
    if (!(p3e & 1)) return 0;
    if (p3e & 0x80) return (p3e & 0x000FFFFFC0000000) + (virt & 0x3FFFFFFF);

    uint64_t *pd = (uint64_t *)(hhdm + phys_mask(p3e));
    uint64_t p2e = pd[PD_I(virt)];
    if (!(p2e & 1)) return 0;
    if (p2e & 0x80) return phys_mask(p2e) + (virt & 0x1FFFFF);

    uint64_t *pt = (uint64_t *)(hhdm + phys_mask(p2e));
    uint64_t p1e = pt[PT_I(virt)];
    if (!(p1e & 1)) return 0;
    return phys_mask(p1e) + (virt & 0xFFF);
}