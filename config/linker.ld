/* =============================================================================
 * Suika OS — Linker Script (64-bit Limine)
 * ============================================================================= */

OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

PHDRS
{
    limine_requests PT_LOAD;
    text PT_LOAD;
    rodata PT_LOAD;
    data PT_LOAD;
}

SECTIONS
{
    . = 0xffffffff80000000;

    .limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :limine_requests

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .text ALIGN(4096) : {
        *(.text)
    } :text

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .rodata ALIGN(4096) : {
        *(.rodata)
    } :rodata

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .data ALIGN(4096) : {
        *(.data)
    } :data

    .bss ALIGN(4096) : {
        _bss_start = .;
        *(.bss)
        *(COMMON)
        _bss_end = .;
    } :data

    _kernel_end = .;

    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }
}