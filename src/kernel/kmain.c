#include <kernel.h>
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
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end[4] = LIMINE_REQUESTS_END_MARKER;

extern void keyboard_handler(registers_t *regs);
extern void timer_handler(registers_t *regs);

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

void irq_handler(registers_t *regs) {
    uint64_t int_no = regs->int_no;
    switch (int_no) {
        case 32: timer_handler(regs); break;
        case 33: keyboard_handler(regs); break;
        default: break;
    }
    if (int_no >= 40) outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

static void hcf(void) {
    for (;;) { __asm__ volatile("hlt"); }
}

void kmain(void) {
    serial_init();
    serial_puts("\n=== Suika OS (64-bit) booting via Limine ===\n");

    serial_puts("[BOOT] Initializing GDT... ");
    gdt_init();
    serial_puts("OK\n");

    serial_puts("[BOOT] Initializing IDT & PIC... ");
    idt_init();
    serial_puts("OK\n");

    serial_puts("[BOOT] Initializing PIT (100 Hz)... ");
    timer_init(100);
    serial_puts("OK\n");

    serial_puts("[BOOT] Initializing PS/2 keyboard... ");
    keyboard_init();
    serial_puts("OK\n");

    serial_puts("[BOOT] All subsystems initialized\n");

    serial_puts("[TEST] Waiting 1 second... ");
    timer_sleep(1000);
    serial_puts("done\n");

    serial_puts("[TEST] Timer works\n");
    serial_puts("[KERNEL] Boot sequence complete\n");

    for (;;) { __asm__ volatile("hlt"); }
}
