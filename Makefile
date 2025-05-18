# Makefile para RanaOS bootable ISO

# Herramientas
NASM          := nasm
CXX           := g++
LD            := ld
GRUB_MKRESCUE := grub-mkrescue
QEMU          := qemu-system-i386

# Flags
CXXFLAGS := -m32 -ffreestanding -O2 -Wall -Wextra \
            -fno-exceptions -fno-rtti -fno-pie -fno-pic \
            -std=gnu++17
LDFLAGS  := -m elf_i386

# Directorios
ISO_DIR   := isodir
BOOT_DIR  := $(ISO_DIR)/boot
GRUB_DIR  := $(BOOT_DIR)/grub
FILES_SRC_DIR = files
FILES_DEST_DIR = $(ISO_DIR)/files

FILES = $(wildcard $(FILES_SRC_DIR)/*.bin)

# Fuentes ASM
ASM_SRCS := boot.asm

ASM_OBJS := boot.o

# Fuentes C++
# Obs.: los objetos van en la raíz: console.o, keyboard.o, etc.
CPP_SRCS := kernel/kernel.cpp                \
            kernel/Console.cpp

CPP_OBJS := kernel.o                         \
            console.o

# Script de linker
LDSCRIPT := kernel/linker.ld

# Salidas
KERNEL_ELF := kernel.elf
ISO_IMG    := RanaOS.iso

.PHONY: all clean iso run

all: iso

# --------------------------------------------------------
# 1) Ensamblar ASM
# --------------------------------------------------------
boot.o: boot.asm
	$(NASM) -f elf32 $< -o $@

getKey.o: kernel/getKey.asm
	$(NASM) -f elf32 $< -o $@

keyboard_poll.o: kernel/keyboard_poll.asm
	$(NASM) -f elf32 $< -o $@

# --------------------------------------------------------
# 2) Compilar C++
# --------------------------------------------------------
kernel.o: kernel/kernel.cpp kernel/Console.h kernel/Keyboard.h kernel/io.h \
          kernel/idt.h kernel/pic.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

console.o: kernel/Console.cpp kernel/Console.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --------------------------------------------------------
# 3) Linkear kernel ELF
# --------------------------------------------------------
$(KERNEL_ELF): $(ASM_OBJS) $(CPP_OBJS) $(LDSCRIPT)
	$(LD) $(LDFLAGS) -T $(LDSCRIPT) -o $@ \
	    $(ASM_OBJS) $(CPP_OBJS)

# --------------------------------------------------------
# 4) Generar ISO booteable con GRUB
# --------------------------------------------------------
copy-binaries:
	@mkdir -p $(FILES_DEST_DIR)
	cp $(FILES) $(FILES_DEST_DIR)

iso: $(KERNEL_ELF)
	@mkdir -p $(GRUB_DIR)
	@cp $(KERNEL_ELF) $(BOOT_DIR)/kernel.elf
	@cp grub/grub.cfg $(GRUB_DIR)/grub.cfg
	@$(GRUB_MKRESCUE) -o $(ISO_IMG) $(ISO_DIR) \
	    --modules="multiboot part_msdos"
	@rm -rf $(ISO_DIR)
	@echo ">>> ISO creada: $(ISO_IMG)"

# --------------------------------------------------------
# 5) Arrancar en QEMU
# --------------------------------------------------------
run: iso
	$(QEMU) -cdrom $(ISO_IMG) -m 512M -curses

# --------------------------------------------------------
# 6) Limpiar
# --------------------------------------------------------
clean:
	@rm -f *.o getKey.o keyboard_poll.o \
	         $(CPP_OBJS) $(ASM_OBJS) \
	         $(KERNEL_ELF) $(ISO_IMG)
	@rm -rf $(ISO_DIR)

