/* =============================================================================
 * Suika OS — Kernel Header (64-bit)
 * ============================================================================= */

#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>

#define NULL ((void *)0)

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void io_wait(void) { outb(0x80, 0); }

#define UNUSED(x) ((void)(x))

enum vga_color {
    VGA_BLACK        = 0x0, VGA_BLUE         = 0x1,
    VGA_GREEN        = 0x2, VGA_CYAN         = 0x3,
    VGA_RED          = 0x4, VGA_MAGENTA      = 0x5,
    VGA_BROWN        = 0x6, VGA_LIGHT_GREY   = 0x7,
    VGA_DARK_GREY    = 0x8, VGA_LIGHT_BLUE   = 0x9,
    VGA_LIGHT_GREEN  = 0xA, VGA_LIGHT_CYAN   = 0xB,
    VGA_LIGHT_RED    = 0xC, VGA_LIGHT_MAGENTA= 0xD,
    VGA_YELLOW       = 0xE, VGA_WHITE        = 0xF,
};

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no;
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed)) registers_t;

void kmain(void);
void panic(const char *message);
void panic_assert(const char *file, uint64_t line, const char *desc);

void serial_init(void);
void serial_putchar(char c);
void serial_puts(const char *str);
void serial_put_hex(uint64_t value);
void serial_put_dec(uint64_t value);

void vga_clear(void);
void vga_putchar(char c);
void vga_puts(const char *str);
void vga_puts_color(const char *str, uint8_t fg, uint8_t bg);
void vga_put_hex(uint64_t value);
void vga_put_dec(uint64_t value);
void vga_set_color(uint8_t fg, uint8_t bg);

void timer_init(uint32_t frequency);
uint64_t timer_get_ticks(void);
void timer_sleep(uint32_t ms);

void keyboard_init(void);
int  keyboard_getchar(void);
char keyboard_read(void);

void gdt_init(void);
void idt_init(void);

size_t strlen(const char *str);
void *memset(void *dst, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
int strcmp(const char *a, const char *b);

#define ASSERT(c) do { if (!(c)) panic_assert(__FILE__, __LINE__, #c); } while(0)

#endif