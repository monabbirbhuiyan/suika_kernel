# Suika OS

A 64-bit hobby OS kernel for x86-64, booting via Limine into long mode with drivers for serial, PIT timer, and PS/2 keyboard.

## Prerequisites

- `x86_64-elf-gcc` (cross-compiler)
- `nasm` (assembler)
- `qemu-system-x86_64` (v7+)
- `xorriso` (ISO creation)
- `GNU make`
- Limine bootloader (included under `limine/`)

Install via Homebrew:
```
brew install x86_64-elf-gcc nasm qemu xorriso make
```

## Build

```
make
```

Output: `build/suika_kernel.iso` (bootable ISO with Limine)

## Run

Headless (serial output only):
```
make run
```

With QEMU window (VGA visible):
```
make run-vga
```

With serial log captured to `build/serial.log`:
```
make run-bios
```

## Debug

```
qemu-system-x86_64 -cdrom build/suika_kernel.iso -m 128M -serial stdio -d cpu_reset,int,guest_errors -D build/qemu.log
```

## Clean

```
make clean
```

## Project Structure

```
src/
  arc/x86_64/
    boot/boot.asm     # 64-bit entry point (Limine protocol)
    gdt.c             # Global Descriptor Table
    idt.c             # Interrupt Descriptor Table & PIC
    interrupts.asm    # ISR/IRQ stubs (64-bit)
    drivers/
      keyboard.c      # PS/2 keyboard driver
      timer.c         # PIT timer driver (counter polling)
  kernel/
    kmain.c           # Kernel main
    vga.c             # VGA text mode driver (unused — needs HHDM)
    serial.c          # COM1 serial driver
    panic.c           # Kernel panic/assert
    include/kernel.h  # Common types & declarations
    include/limine.h  # Limine protocol header
    memory/
      pmm.c           # Physical memory manager (stub)
      vmm.c           # Virtual memory manager (stub)
config/linker.ld      # Linker script (elf64-x86-64, higher-half 0xffffffff80000000)
limine.conf           # Limine boot config
Makefile              # Build system
```

## Known Issues

- Timer interrupts not delivered (LINT0 masked by local APIC or I/O APIC routing) — PIT used via counter polling only
- VGA output broken — buffer `0xB8000` not mapped in page tables (needs HHDM offset remapping)
