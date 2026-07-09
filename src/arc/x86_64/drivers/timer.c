/* =============================================================================
 * Suika OS — PIT Timer Driver (32-bit)
 * ============================================================================= */

#include <kernel.h>

#define PIT_BASE_FREQ   1193182

static uint64_t timer_ticks = 0;
static uint32_t timer_freq = 0;

void timer_handler(registers_t *regs) {
    timer_ticks++;
    UNUSED(regs);
}

uint64_t timer_get_ticks(void) { return timer_ticks; }

void timer_sleep(uint32_t ms) {
    uint32_t target_ticks = timer_ticks + (ms * timer_freq / 1000);
    while (timer_ticks < target_ticks) __asm__ volatile("hlt");
}

void timer_init(uint32_t frequency) {
    timer_freq = frequency;
    timer_ticks = 0;
    uint32_t divisor = PIT_BASE_FREQ / frequency;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}