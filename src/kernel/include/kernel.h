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

void fb_init(uint64_t hhdm_offset, uint64_t fb_phys, uint64_t width, uint64_t height, uint64_t pitch, uint16_t bpp);
void fb_clear(uint32_t color);
void fb_putchar(char c);
void fb_puts(const char *str);
void fb_puts_color(const char *str, uint32_t fg, uint32_t bg);
void fb_put_hex(uint64_t value);
void fb_put_dec(uint64_t value);

void timer_init(uint32_t frequency);
uint64_t timer_get_ticks(void);
void timer_sleep(uint32_t ms);

void keyboard_init(void);
int  keyboard_getchar(void);
char keyboard_read(void);

void gdt_init(void);
void idt_init(void);
void apic_init(uint64_t hhdm_offset);

size_t strlen(const char *str);
void *memset(void *dst, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
int strcmp(const char *a, const char *b);

#define ASSERT(c) do { if (!(c)) panic_assert(__FILE__, __LINE__, #c); } while(0)

#endif