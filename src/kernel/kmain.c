#include <kernel.h>
#include <memory.h>
#include <scheduler.h>
#include <limine.h>

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start[4] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[3] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request fb_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end[4] = LIMINE_REQUESTS_END_MARKER;

extern void keyboard_handler(registers_t *regs);
extern void timer_handler(registers_t *regs);

static volatile uint64_t thread_a_count = 0;
static volatile uint64_t thread_b_count = 0;

static void thread_a(void) {
    for (;;) thread_a_count++;
}

static void thread_b(void) {
    for (;;) thread_b_count++;
}

static const char *exception_names[] = {
    "Division By Zero", "Debug", "Non-Maskable Interrupt", "Breakpoint",
    "Overflow", "Bound Range Exceeded", "Invalid Opcode", "Device Not Available",
    "Double Fault", "Coprocessor Segment Overrun", "Invalid TSS",
    "Segment Not Present", "Stack-Segment Fault", "General Protection Fault",
    "Page Fault", "Reserved", "x87 Floating-Point Exception", "Alignment Check",
    "Machine Check", "SIMD Floating-Point Exception", "Virtualization Exception",
    "Control Protection Exception",
};

void isr_handler(registers_t *regs) {
    uint64_t int_no = regs->int_no;
    if (int_no < 22) {
        serial_puts("\n=== KERNEL PANIC ===\nException: ");
        serial_puts(exception_names[int_no]);
        serial_puts("\nError: ");  serial_put_hex(regs->error_code);
        serial_puts("\nRIP: ");   serial_put_hex(regs->rip);
        serial_puts("\n");
    } else {
        serial_puts("\nUnknown interrupt: "); serial_put_dec(int_no); serial_puts("\n");
    }
    for (;;) { __asm__ volatile("cli; hlt"); }
}

uint64_t irq_handler(registers_t *regs) {
    uint64_t int_no = regs->int_no;
    uint64_t new_rsp = 0;
    switch (int_no) {
        case 32: timer_handler(regs); new_rsp = scheduler_tick((uint64_t)regs); break;
        case 33: keyboard_handler(regs); break;
        default: break;
    }
    if (int_no >= 40) outb(0xA0, 0x20);
    outb(0x20, 0x20);
    return new_rsp;
}

void kmain(void) {
    serial_init();
    serial_puts("\n=== Suika OS (64-bit) booting via Limine ===\n");

    uint64_t hhdm_offset = hhdm_request.response->offset;

    serial_puts("[BOOT] Initializing GDT... ");
    gdt_init();
    serial_puts("OK\n");

    serial_puts("[BOOT] Initializing IDT & PIC... ");
    idt_init();
    serial_puts("OK\n");

    serial_puts("[BOOT] Initializing APIC (LINT0)... ");
    apic_init(hhdm_offset);
    serial_puts("OK\n");

    serial_puts("[BOOT] Initializing PIT (100 Hz)... ");
    timer_init(100);
    serial_puts("OK\n");

    serial_puts("[BOOT] Initializing PS/2 keyboard... ");
    keyboard_init();
    serial_puts("OK\n");

    serial_puts("[BOOT] Initializing PMM... ");
    pmm_init(hhdm_offset);
    serial_puts("OK\n");
    serial_puts("  Free frames: "); serial_put_dec(pmm_get_free_count()); serial_puts("\n");

    serial_puts("[BOOT] Initializing VMM... ");
    vmm_init();
    serial_puts("OK\n");

    serial_puts("[BOOT] Initializing framebuffer... ");
    if (fb_request.response && fb_request.response->framebuffer_count) {
        struct limine_framebuffer *fb = fb_request.response->framebuffers[0];
        fb_init(hhdm_offset, (uint64_t)fb->address - hhdm_offset, fb->width, fb->height, fb->pitch, fb->bpp);
        fb_clear(0x000080);
        fb_puts_color("================================================================\n", 0xFFFFFF, 0x000080);
        fb_puts_color("                          SUIKA OS (64-bit)                    \n", 0xFFFFFF, 0x000080);
        fb_puts_color("                    Developer-Friendly OS                       \n", 0xFFFFFF, 0x000080);
        fb_puts_color("================================================================\n", 0xFFFFFF, 0x000080);
        serial_puts("OK\n");
    } else {
        serial_puts("no framebuffer\n");
    }

    serial_puts("[BOOT] Initializing heap... ");
    heap_init(hhdm_offset);
    serial_puts("OK\n");

    serial_puts("[BOOT] Initializing scheduler... ");
    scheduler_init();
    serial_puts("OK\n");

    serial_puts("[BOOT] All subsystems initialized\n\n");

    serial_puts("[TEST] Waiting 1 second... ");
    timer_sleep(1000);
    serial_puts("done\n");

    serial_puts("[KERNEL] Creating threads...\n");
    task_create("thread-a", thread_a);
    task_create("thread-b", thread_b);
    serial_puts("[KERNEL] Threads created, running\n");

    for (;;) {
        timer_sleep(2000);
        serial_puts("[init] a="); serial_put_dec(thread_a_count);
        serial_puts(" b="); serial_put_dec(thread_b_count);
        serial_puts("\n");
    }
}