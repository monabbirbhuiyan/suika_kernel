# ============================================================================
# Suika OS — Makefile (64-bit Limine ISO)
# ============================================================================

TOPDIR   := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
BUILDDIR := $(TOPDIR)/build
OBJDIR   := $(BUILDDIR)/obj
ISODIR   := $(BUILDDIR)/iso
KERNEL   := $(BUILDDIR)/suika_kernel.elf
ISO      := $(BUILDDIR)/suika_kernel.iso

CC  := x86_64-elf-gcc
AS  := nasm
LD  := x86_64-elf-ld

INC   := -I $(TOPDIR)/src/kernel/include
CFLAGS  := -c -Wall -Wextra -std=gnu99 -nostdlib -ffreestanding \
           -fno-stack-protector -fno-stack-check -fno-PIC -fno-pic \
           -m64 -mno-red-zone -mno-sse -mno-mmx -mno-3dnow \
           -mcmodel=kernel \
           -O2 -pipe $(INC)

ASFLAGS := -f elf64

LDFLAGS := -T $(TOPDIR)/config/linker.ld -nostdlib -m elf_x86_64

LIMINE  := $(TOPDIR)/limine/limine
LM_BOOT := $(TOPDIR)/limine

C_SRCS := $(shell find $(TOPDIR)/src/kernel -name '*.c') $(shell find $(TOPDIR)/src/arc/x86_64 -name '*.c')
AS_SRCS := $(shell find $(TOPDIR)/src/arc/x86_64 -name '*.asm')

C_OBJS := $(patsubst $(TOPDIR)/src/%.c, $(OBJDIR)/src/%.c.o, $(C_SRCS))
AS_OBJS := $(patsubst $(TOPDIR)/src/%.asm, $(OBJDIR)/src/%.asm.o, $(AS_SRCS))
OBJS := $(C_OBJS) $(AS_OBJS)

.PHONY: all
all: $(ISO)

.PHONY: run
run: $(ISO)
	qemu-system-x86_64 -M q35 -cdrom $(ISO) -boot d -m 128M -serial stdio -display none

.PHONY: run-vga
run-vga: $(ISO)
	qemu-system-x86_64 -M q35 -cdrom $(ISO) -boot d -m 128M -serial stdio

.PHONY: run-bios
run-bios: $(ISO)
	qemu-system-x86_64 -M q35 -cdrom $(ISO) -boot d -m 128M -serial file:$(BUILDDIR)/serial.log -display none

# Compilation rules
$(AS_OBJS): $(OBJDIR)/src/%.asm.o: $(TOPDIR)/src/%.asm
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) -o $@ $<

$(C_OBJS): $(OBJDIR)/src/%.c.o: $(TOPDIR)/src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(KERNEL): $(OBJS) $(TOPDIR)/config/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo "Kernel linked: $@"

# ISO build
$(ISO): $(KERNEL) $(LIMINE)
	rm -rf $(ISODIR)
	mkdir -p $(ISODIR)/boot
	cp $(KERNEL) $(ISODIR)/boot/
	mkdir -p $(ISODIR)/boot/limine
	cp $(TOPDIR)/limine.conf $(ISODIR)/boot/limine/
	cp $(LM_BOOT)/limine-bios.sys $(LM_BOOT)/limine-bios-cd.bin $(LM_BOOT)/limine-uefi-cd.bin $(ISODIR)/boot/limine/
	mkdir -p $(ISODIR)/EFI/BOOT
	cp $(LM_BOOT)/BOOTX64.EFI $(ISODIR)/EFI/BOOT/
	cp $(LM_BOOT)/BOOTIA32.EFI $(ISODIR)/EFI/BOOT/
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
		-apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		$(ISODIR) -o $(ISO)
	$(LIMINE) bios-install $(ISO)
	@echo "ISO created: $(ISO)"
	rm -rf $(ISODIR)

.PHONY: clean
clean:
	rm -rf $(BUILDDIR)

.PHONY: distclean
distclean: clean
	$(MAKE) -C $(LM_BOOT) clean 2>/dev/null || true
	rm -rf $(LM_BOOT)