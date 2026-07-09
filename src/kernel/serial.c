/* =============================================================================
 * Suika OS — Serial Port Driver (COM1)
 * ============================================================================= */

#include <kernel.h>

#define SERIAL_COM1    0x3F8

void serial_init(void) {
    outb(SERIAL_COM1 + 1, 0x00);   /* Disable interrupts */
    outb(SERIAL_COM1 + 3, 0x80);   /* Enable DLAB */
    outb(SERIAL_COM1 + 0, 0x03);   /* Divisor lo (38400 baud) */
    outb(SERIAL_COM1 + 1, 0x00);   /* Divisor hi */
    outb(SERIAL_COM1 + 3, 0x03);   /* 8N1 */
    outb(SERIAL_COM1 + 2, 0xC7);   /* Enable FIFO */
    outb(SERIAL_COM1 + 4, 0x0B);   /* IRQs, RTS/DSR */
}

static int serial_tx_empty(void) {
    return inb(SERIAL_COM1 + 5) & 0x20;
}

void serial_putchar(char c) {
    while (!serial_tx_empty());
    outb(SERIAL_COM1, (uint8_t)c);
}

void serial_puts(const char *str) {
    while (*str) {
        if (*str == '\n') serial_putchar('\r');
        serial_putchar(*str++);
    }
}

void serial_put_hex(uint64_t value) {
    char buf[18];
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 15; i >= 0; i--) {
        int digit = (value >> (i * 4)) & 0xF;
        buf[16 - i] = digit < 10 ? '0' + digit : 'a' + digit - 10;
    }
    buf[17] = '\0';
    serial_puts(buf);
}

void serial_put_dec(uint64_t value) {
    if (value == 0) { serial_putchar('0'); return; }
    char buf[21];
    int i = 20;
    buf[20] = '\0';
    uint32_t v = (uint32_t)value;
    while (v > 0) { buf[--i] = '0' + (v % 10); v /= 10; }
    serial_puts(&buf[i]);
}