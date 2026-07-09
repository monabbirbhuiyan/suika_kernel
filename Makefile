# =============================================================================
# Suika OS — Makefile (32-bit kernel, switches to 64-bit at runtime)
# =============================================================================

CC      = x86_64-elf-gcc
AS      = nasm
LD      = x86_64-elf-ld

CFLAGS  = -ffreestanding -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
          -fno-stack-protector -nostdlib -nostdinc -Wall -Wextra \
          -Isrc/kernel/include -c -m32
ASFLAGS = -f elf32
LDFLAGS = -T config/linker.ld -nostdlib -m elf_i386

ASM_SOURCES = \
    src/arc/x86_64/boot/boot.asm \
    src/arc/x86_64/interrupts.asm

C_SOURCES = \
    src/kernel/kmain.c \
    src/kernel/panic.c \
    src/kernel/vga.c \
    src/kernel/serial.c \
    src/arc/x86_64/gdt.c \
    src/arc/x86_64/idt.c \
    src/arc/x86_64/drivers/keyboard.c \
    src/arc/x86_64/drivers/timer.c \
    src/kernel/memory/pmm.c \
    src/kernel/memory/vmm.c

BUILD_DIR   = build
OBJ_DIR     = $(BUILD_DIR)/obj

ASM_OBJECTS = $(patsubst %.asm,$(OBJ_DIR)/%.asm.o,$(ASM_SOURCES))
C_OBJECTS   = $(patsubst %.c,$(OBJ_DIR)/%.c.o,$(C_SOURCES))
ALL_OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)

KERNEL_ELF  = $(BUILD_DIR)/suika_kernel.elf

.PHONY: all clean run

all: $(KERNEL_ELF)

$(KERNEL_ELF): $(ALL_OBJECTS)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $^
	@echo "Kernel linked: $@"

$(OBJ_DIR)/%.asm.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@
	@echo "  AS    $<"

$(OBJ_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP $< -o $@
	@echo "  CC    $<"

clean:
	rm -rf $(BUILD_DIR)

run: $(KERNEL_ELF)
	qemu-system-i386 \
		-serial stdio \
		-display none \
		-m 128M \
		-kernel $(KERNEL_ELF)