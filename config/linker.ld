/* =============================================================================
 * Suika OS — Linker Script (32-bit Multiboot2 → Long Mode)
 * ============================================================================= */

OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

KERNEL_BASE = 0x100000;

SECTIONS
{
    . = KERNEL_BASE;

    .multiboot ALIGN(8) : {
        *(.multiboot)
    }

    .text ALIGN(4096) : {
        *(.text)
    }

    .rodata ALIGN(4096) : {
        *(.rodata)
    }

    .data ALIGN(4096) : {
        *(.data)
    }

    .bss ALIGN(4096) : {
        _bss_start = .;
        *(.bss)
        *(COMMON)
        _bss_end = .;
    }

    _kernel_end = .;
}