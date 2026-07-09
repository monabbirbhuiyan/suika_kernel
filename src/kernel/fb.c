#include <kernel.h>
#include <memory.h>

static uint32_t *fb_ptr = NULL;
static uint64_t fb_width = 0;
static uint64_t fb_height = 0;
static uint64_t fb_pitch = 0;
static uint16_t fb_bpp = 0;

static int cursor_x = 0;
static int cursor_y = 0;
static uint32_t fg_color = 0xFFFFFF;
static uint32_t bg_color = 0x000000;
static int fb_char_width = 8;
static int fb_char_height = 16;

static uint8_t font8x16[256][16];

void fb_init(uint64_t hhdm_offset, uint64_t fb_phys, uint64_t width, uint64_t height, uint64_t pitch, uint16_t bpp) {
    uint64_t fb_virt = 0xFFFFFFFF82000000;
    for (uint64_t off = 0; off < height * pitch; off += 0x1000) {
        vmm_map_page(hhdm_offset, fb_virt + off, fb_phys + off, 0x23);
    }
    fb_ptr = (uint32_t *)fb_virt;
    fb_width = width;
    fb_height = height;
    fb_pitch = pitch;
    fb_bpp = bpp;
}

static void draw_pixel(int x, int y, uint32_t color) {
    if (x < 0 || (uint64_t)x >= fb_width || y < 0 || (uint64_t)y >= fb_height) return;
    fb_ptr[y * (fb_pitch / 4) + x] = color;
}

void fb_clear(uint32_t color) {
    for (uint64_t y = 0; y < fb_height; y++) {
        for (uint64_t x = 0; x < fb_width; x++) {
            fb_ptr[y * (fb_pitch / 4) + x] = color;
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

static void draw_char(int x, int y, char c, uint32_t fg, uint32_t bg) {
    uint8_t *glyph = font8x16[(uint8_t)c];
    for (int row = 0; row < fb_char_height; row++) {
        for (int col = 0; col < fb_char_width; col++) {
            draw_pixel(x + col, y + row, (glyph[row] & (0x80 >> col)) ? fg : bg);
        }
    }
}

void fb_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y += fb_char_height;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x / (fb_char_width * 4) + 1) * (fb_char_width * 4);
    } else {
        draw_char(cursor_x, cursor_y, c, fg_color, bg_color);
        cursor_x += fb_char_width;
    }
    if ((uint64_t)cursor_x + fb_char_width > fb_width) {
        cursor_x = 0;
        cursor_y += fb_char_height;
    }
    if ((uint64_t)cursor_y + fb_char_height > fb_height) {
        fb_clear(bg_color);
    }
}

void fb_puts(const char *str) {
    while (*str) fb_putchar(*str++);
}

void fb_puts_color(const char *str, uint32_t fg, uint32_t bg) {
    uint32_t old_fg = fg_color;
    uint32_t old_bg = bg_color;
    fg_color = fg;
    bg_color = bg;
    fb_puts(str);
    fg_color = old_fg;
    bg_color = old_bg;
}

void fb_put_hex(uint64_t value) {
    char buf[19];
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 15; i >= 0; i--) {
        int digit = (value >> (i * 4)) & 0xF;
        buf[17 - i] = digit < 10 ? '0' + digit : 'a' + digit - 10;
    }
    buf[18] = '\0';
    fb_puts(buf);
}

void fb_put_dec(uint64_t value) {
    if (value == 0) { fb_putchar('0'); return; }
    char buf[21];
    int i = 20;
    buf[20] = '\0';
    uint64_t v = value;
    while (v > 0) { buf[--i] = '0' + (v % 10); v /= 10; }
    fb_puts(&buf[i]);
}