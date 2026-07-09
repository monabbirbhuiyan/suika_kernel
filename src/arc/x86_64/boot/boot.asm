; =============================================================================
; Suika OS — Bootstrap Entry Point (64-bit Long Mode via Limine)
; =============================================================================

[bits 64]
[global _start]
[extern kmain]
[extern _bss_start]
[extern _bss_end]

section .text

_start:
    cli

    ; Zero BSS
    mov rdi, _bss_start
    mov rcx, _bss_end
    sub rcx, rdi
    xor al, al
    cld
    rep stosb

    mov dx, 0x3F8
    mov al, 'S'
    out dx, al
    mov al, 'O'
    out dx, al
    mov al, 'K'
    out dx, al
    mov al, 10
    out dx, al

    call kmain

.halt:
    cli
    hlt
    jmp .halt

; ---------------------------------------------------------------------------
; GDT Flush (64-bit)
; ---------------------------------------------------------------------------
[global gdt_flush]
gdt_flush:
    lgdt [rdi]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ; far return to reload CS
    push 0x08
    lea rax, [rel .reload]
    push rax
    retfq
.reload:
    ret