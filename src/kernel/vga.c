/* =============================================================================
 * Suika OS — VGA Text Mode Driver
 * ============================================================================= */

#include <kernel.h>

#define VGA_BUFFER  0xB8000
#define VGA_WIDTH   80
#define VGA_HEIGHT  25

static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t current_attr = 0x0F;

static inline uint16_t vga_entry(char c, uint8_t attr) {
    return (uint16_t)c | ((uint16_t)attr << 8);
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    current_attr = fg | (bg << 4);
}

static void update_cursor(void) {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static void scroll(void) {
    volatile uint16_t *buffer = (volatile uint16_t *)VGA_BUFFER;
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            buffer[(y - 1) * VGA_WIDTH + x] = buffer[y * VGA_WIDTH + x];
        }
    }
    for (int x = 0; x < VGA_WIDTH; x++) {
        buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', current_attr);
    }
    cursor_y = VGA_HEIGHT - 1;
}

void vga_clear(void) {
    volatile uint16_t *buffer = (volatile uint16_t *)VGA_BUFFER;
    uint16_t entry = vga_entry(' ', current_attr);
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) buffer[i] = entry;
    cursor_x = 0; cursor_y = 0;
    update_cursor();
}

void vga_putchar(char c) {
    volatile uint16_t *buffer = (volatile uint16_t *)VGA_BUFFER;
    if (c == '\n') { cursor_x = 0; cursor_y++; }
    else if (c == '\r') { cursor_x = 0; }
    else if (c == '\t') { cursor_x = (cursor_x + 4) & ~3; }
    else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            buffer[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(' ', current_attr);
        }
    } else {
        buffer[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(c, current_attr);
        cursor_x++;
    }
    if (cursor_x >= VGA_WIDTH) { cursor_x = 0; cursor_y++; }
    if (cursor_y >= VGA_HEIGHT) scroll();
    update_cursor();
}

void vga_puts(const char *str) {
    while (*str) vga_putchar(*str++);
}

void vga_puts_color(const char *str, uint8_t fg, uint8_t bg) {
    uint8_t old = current_attr;
    vga_set_color(fg, bg);
    vga_puts(str);
    current_attr = old;
}

void vga_put_hex(uint64_t value) {
    char buf[18];
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 15; i >= 0; i--) {
        int digit = (value >> (i * 4)) & 0xF;
        buf[16 - i] = digit < 10 ? '0' + digit : 'a' + digit - 10;
    }
    buf[17] = '\0';
    vga_puts(buf);
}

void vga_put_dec(uint64_t value) {
    if (value == 0) { vga_putchar('0'); return; }
    char buf[21];
    int i = 20;
    buf[20] = '\0';
    uint32_t v = (uint32_t)value;
    while (v > 0) { buf[--i] = '0' + (v % 10); v /= 10; }
    vga_puts(&buf[i]);
}