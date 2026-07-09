# Suika OS

A 32-bit hobby OS kernel for x86, booting into protected mode with drivers for VGA text mode, serial, PIT timer, and PS/2 keyboard.

## Prerequisites

- `x86_64-elf-gcc` (cross-compiler)
- `nasm`
- `qemu-system-x86_64` (v7+)
- `GNU make`

Install via Homebrew:
```
brew install x86_64-elf-gcc nasm qemu make
```

## Build

```
make
```

Output: `build/suika_kernel.elf`

## Run

With graphical window (keyboard works when window is focused):
```
qemu-system-x86_64 -kernel build/suika_kernel.elf -m 128M -serial stdio
```

Headless (serial output only, no PS/2 input):
```
qemu-system-x86_64 -kernel build/suika_kernel.elf -m 128M -serial stdio -display none
```

## Debug

```
qemu-system-x86_64 -kernel build/suika_kernel.elf -m 128M -serial stdio -d cpu_reset,int,guest_errors -D build/qemu.log
```

## Clean

```
make clean
```

## Project Structure

```
src/
  arc/x86_64/
    boot/boot.asm     # Entry point, PVH ELF Note, Multiboot1 header
    gdt.c             # Global Descriptor Table
    idt.c             # Interrupt Descriptor Table & PIC
    interrupts.asm    # ISR/IRQ stubs
    drivers/
      keyboard.c      # PS/2 keyboard driver
      timer.c         # PIT timer driver
  kernel/
    kmain.c           # Kernel main
    vga.c             # VGA text mode driver
    serial.c          # COM1 serial driver
    panic.c           # Kernel panic/assert
    include/kernel.h  # Common types & declarations
    memory/
      pmm.c           # Physical memory manager (stub)
      vmm.c           # Virtual memory manager (stub)
config/linker.ld      # Linker script (elf32-i386, base 0x100000)
Makefile              # Build system
```
