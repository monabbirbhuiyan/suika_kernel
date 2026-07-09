/* =============================================================================
 * Suika OS — PS/2 Keyboard Driver (32-bit)
 * ============================================================================= */

#include <kernel.h>

#define KB_BUFFER_SIZE 256
static char kb_buffer[KB_BUFFER_SIZE];
static int  kb_head = 0;
static int  kb_tail = 0;

static const char scancode_table[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']', '\n',
    0,  'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,  '\\','z','x','c','v','b','n','m',',','.','/',  0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

static const char scancode_shift_table[128] = {
    0,  27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}', '\n',
    0,  'A','S','D','F','G','H','J','K','L',':','"','~',
    0,  '|','Z','X','C','V','B','N','M','<','>','?',  0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

static int shift_pressed = 0;

static void kb_push(char c) {
    int next = (kb_head + 1) % KB_BUFFER_SIZE;
    if (next != kb_tail) {
        kb_buffer[kb_head] = c;
        kb_head = next;
    }
}

void keyboard_handler(registers_t *regs) {
    uint8_t scancode = inb(0x60);
    if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36) shift_pressed = 0;
        UNUSED(regs);
        return;
    }
    if (scancode == 0x2A || scancode == 0x36) { shift_pressed = 1; return; }
    if (scancode < 128) {
        char c = shift_pressed ? scancode_shift_table[scancode] : scancode_table[scancode];
        if (c != 0) kb_push(c);
    }
    UNUSED(regs);
}

int keyboard_getchar(void) {
    if (kb_head == kb_tail) return -1;
    char c = kb_buffer[kb_tail];
    kb_tail = (kb_tail + 1) % KB_BUFFER_SIZE;
    return c;
}

char keyboard_read(void) {
    int c;
    while ((c = keyboard_getchar()) == -1) __asm__ volatile("hlt");
    return (char)c;
}

void keyboard_init(void) {
    kb_head = 0; kb_tail = 0;
    while (inb(0x64) & 1) inb(0x60);
}