; =============================================================================
; Suika OS — Bootloader Entry Point (32-bit Protected Mode)
; =============================================================================

[bits 32]
[global _start]
[extern kmain]

; ---------------------------------------------------------------------------
; PVH ELF Note (for QEMU -kernel direct boot)
; ---------------------------------------------------------------------------
section .text
align 4

pvh_note:
    dd 4                          ; namesz (len of "Xen\0")
    dd 4                          ; descsz (32-bit pointer)
    dd 3                          ; type = XEN_ELFNOTE_PHYS32_ENTRY
    db 'Xen', 0                   ; name
    dd _start                     ; entry point

; ---------------------------------------------------------------------------
; Multiboot1 Header (for GRUB)
; ---------------------------------------------------------------------------
section .multiboot
align 4

MB1_MAGIC      equ 0x1BADB002
MB1_FLAGS      equ 0x00000003  ; page align + memory info
MB1_CHECKSUM   equ -(MB1_MAGIC + MB1_FLAGS)

mb1_header:
    dd MB1_MAGIC
    dd MB1_FLAGS
    dd MB1_CHECKSUM
    dd 0
    dd 0
    dd 0
    dd 0
    dd 0

; ---------------------------------------------------------------------------
; 32-bit Entry Point
; ---------------------------------------------------------------------------
section .text

_start:
    cli

    ; Set up stack
    mov esp, 0x200000

    ; Write "SOK!" to COM1
    mov dx, 0x3F8
    mov al, 'S'
    out dx, al
    mov al, 'O'
    out dx, al
    mov al, 'K'
    out dx, al
    mov al, 10
    out dx, al

    ; Call C kernel
    call kmain

.halt:
    cli
    hlt
    jmp .halt