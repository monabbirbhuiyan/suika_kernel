/* =============================================================================
 * Suika OS — Kernel Main (32-bit compatible)
 * ============================================================================= */

#include <kernel.h>

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

void *memset(void *dst, int c, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    for (size_t i = 0; i < n; i++) d[i] = (uint8_t)c;
    return dst;
}

void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
    return dst;
}

int strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return *(const uint8_t *)a - *(const uint8_t *)b;
}

/* ISR / IRQ dispatcher */
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
    uint32_t int_no = regs->int_no;
    if (int_no < 22) {
        serial_puts("\n=== KERNEL PANIC ===\nException: ");
        serial_puts(exception_names[int_no]);
        serial_puts("\nError: ");  serial_put_hex(regs->error_code);
        serial_puts("\nEIP: ");   serial_put_hex(regs->eip);
        serial_puts("\n");
    } else {
        serial_puts("\nUnknown interrupt: "); serial_put_dec(int_no); serial_puts("\n");
    }
    for (;;) { __asm__ volatile("cli; hlt"); }
}

void irq_handler(registers_t *regs) {
    uint32_t int_no = regs->int_no;
    switch (int_no) {
        case 32: timer_handler(regs); break;
        case 33: keyboard_handler(regs); break;
        default: break;
    }
    if (int_no >= 40) outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

/* Kernel main entry point */
void kmain(void) {
    serial_init();
    serial_puts("\n=== Suika OS Booting ===\n");

    vga_clear();
    vga_set_color(VGA_WHITE, VGA_BLUE);
    vga_puts("================================================================\n");
    vga_puts("                          SUIKA OS                              \n");
    vga_puts("                    Developer-Friendly OS                       \n");
    vga_puts("================================================================\n");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("\n");

    serial_puts("[BOOT] Initializing GDT...\n");
    vga_puts_color("[BOOT] ", VGA_GREEN, VGA_BLACK); vga_puts("Initializing GDT...");
    gdt_init();
    vga_puts_color(" OK\n", VGA_GREEN, VGA_BLACK);

    serial_puts("[BOOT] Initializing IDT & PIC...\n");
    vga_puts_color("[BOOT] ", VGA_GREEN, VGA_BLACK); vga_puts("Initializing IDT & PIC...");
    idt_init();
    vga_puts_color(" OK\n", VGA_GREEN, VGA_BLACK);

    serial_puts("[BOOT] Initializing PIT timer (100 Hz)...\n");
    vga_puts_color("[BOOT] ", VGA_GREEN, VGA_BLACK); vga_puts("Initializing PIT timer (100 Hz)...");
    timer_init(100);
    vga_puts_color(" OK\n", VGA_GREEN, VGA_BLACK);

    serial_puts("[BOOT] Initializing PS/2 keyboard...\n");
    vga_puts_color("[BOOT] ", VGA_GREEN, VGA_BLACK); vga_puts("Initializing PS/2 keyboard...");
    keyboard_init();
    vga_puts_color(" OK\n", VGA_GREEN, VGA_BLACK);

    vga_puts("\n");
    vga_puts_color("All Phase 1 subsystems initialized successfully!\n\n", VGA_LIGHT_GREEN, VGA_BLACK);
    serial_puts("[BOOT] All Phase 1 subsystems initialized!\n");

    serial_puts("[TEST] Waiting 1 second...\n");
    vga_puts_color("[TEST] ", VGA_CYAN, VGA_BLACK); vga_puts("Waiting 1 second...\n");
    timer_sleep(1000);
    vga_puts_color("[TEST] ", VGA_CYAN, VGA_BLACK); vga_puts("Timer working!\n\n");
    serial_puts("[TEST] Timer working!\n");

    vga_puts_color("[SHELL] ", VGA_LIGHT_MAGENTA, VGA_BLACK);
    vga_puts("Type something (keyboard test): ");
    serial_puts("[SHELL] Type something (keyboard test):\n[SHELL] > ");
    vga_set_color(VGA_WHITE, VGA_BLACK);

    for (;;) {
        char c = keyboard_read();
        if (c == '\n') {
            vga_putchar('\n');
            vga_puts_color("[SHELL] ", VGA_LIGHT_MAGENTA, VGA_BLACK); vga_puts("> ");
            serial_puts("\n[SHELL] > ");
        } else if (c == '\b') {
            vga_putchar('\b');
        } else {
            vga_putchar(c);
            serial_putchar(c);
        }
    }
}