#include <kernel.h>

#define PIT_BASE_FREQ   1193182

static volatile uint64_t timer_ticks = 0;
static uint32_t timer_freq = 0;
static uint16_t pit_divisor = 0;

static inline uint16_t pit_read_count(void) {
    outb(0x43, 0x00);
    uint8_t low = inb(0x40);
    uint8_t high = inb(0x40);
    return (uint16_t)low | ((uint16_t)high << 8);
}

void timer_handler(registers_t *regs) {
    timer_ticks++;
    UNUSED(regs);
}

uint64_t timer_get_ticks(void) { return timer_ticks; }

void timer_sleep(uint32_t ms) {
    uint64_t target = timer_ticks + (timer_freq * ms / 1000);
    __asm__ volatile("sti");
    while (timer_ticks < target) {
        __asm__ volatile("hlt");
    }
    __asm__ volatile("cli");
}

void timer_init(uint32_t frequency) {
    timer_freq = frequency;
    timer_ticks = 0;
    pit_divisor = PIT_BASE_FREQ / frequency;
    outb(0x43, 0x34);
    outb(0x40, (uint8_t)(pit_divisor & 0xFF));
    outb(0x40, (uint8_t)((pit_divisor >> 8) & 0xFF));
}
