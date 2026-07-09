/* =============================================================================
 * Suika OS — Kernel Panic (32-bit)
 * ============================================================================= */

#include <kernel.h>

void panic(const char *message) {
    __asm__ volatile("cli");
    serial_puts("\n=== KERNEL PANIC ===\n");
    serial_puts(message);
    serial_puts("\nSystem halted.\n");
    for (;;) { __asm__ volatile("cli; hlt"); }
}

void panic_assert(const char *file, uint64_t line, const char *desc) {
    __asm__ volatile("cli");
    serial_puts("\n=== ASSERTION FAILED ===\nFile: ");
    serial_puts(file);
    serial_puts("\nLine: "); serial_put_dec(line);
    serial_puts("\nCondition: "); serial_puts(desc);
    serial_puts("\nSystem halted.\n");
    for (;;) { __asm__ volatile("cli; hlt"); }
}