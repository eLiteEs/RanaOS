# Makefile para RanaOS bootable ISO

# Herramientas
NASM		  := nasm
CXX		   := g++
LD			:= ld
GRUB_MKRESCUE := grub-mkrescue
QEMU		  := qemu-system-i386

# Flags
CXXFLAGS := -m32 -ffreestanding -O2 -Wall -Wextra \
	-fno-exceptions -fno-rtti -fno-pie -fno-pic \
	-std=gnu++17 -Ikernel
LDFLAGS := -m elf_i386 -nostdlib -L/usr/lib/gcc/x86_64-linux-gnu/11/32 -lgcc
CFLAGS   := -m32 -ffreestanding -O2 -Wall -Wextra \
	-fno-pie -fno-pic \
	-std=gnu17 -Ikernel

# Directorios
ISO_DIR   := isodir
BOOT_DIR  := $(ISO_DIR)/boot
GRUB_DIR  := $(BOOT_DIR)/grub

# Fuentes
ASM_SRCS := boot.asm threads.asm
ASM_OBJS := $(patsubst %.asm, %.o, $(ASM_SRCS))

CPP_SRCS := kernel/kernel.cpp \
	kernel/Console.cpp \
	kernel/io.cpp \
	kernel/disk.cpp \
	kernel/filesystem.cpp \
	kernel/Graphics.cpp \
	kernel/string.cpp \
	kernel/memory.cpp \
	kernel/math.cpp

CPP_OBJS := $(patsubst kernel/%.cpp, %.o, $(CPP_SRCS))

LDSCRIPT := kernel/linker.ld

MODULES = files/text.txt kernel/ata_detect.cpp kernel/Console.cpp kernel/Console.h kernel/disk.cpp kernel/disk.hpp kernel/fat32.cpp
MODULES_DIR  := $(ISO_DIR)/files

# Salidas
KERNEL_ELF := kernel.elf
ISO_IMG	:= RanaOS.iso

.PHONY: all clean iso run

all: iso

clear:
	@clear

# --------------------------------------------------------
# 1) Ensamblar ASM
# --------------------------------------------------------
%.o: %.asm
	$(NASM) -f elf32 $< -o $@

# --------------------------------------------------------
# 2) Compilar C++
# --------------------------------------------------------
%.o: kernel/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --------------------------------------------------------
# 3) Linkear kernel ELF
# --------------------------------------------------------
$(KERNEL_ELF): $(ASM_OBJS) $(CPP_OBJS) $(LDSCRIPT)
	$(LD) $(LDFLAGS) -T $(LDSCRIPT) -o $@ $(ASM_OBJS) $(CPP_OBJS) -lgcc

# --------------------------------------------------------
# 4) Generar ISO booteable
# --------------------------------------------------------
iso: $(KERNEL_ELF)
	@mkdir -p $(GRUB_DIR)
	@mkdir -p $(MODULES_DIR)
	@cp $(KERNEL_ELF) $(BOOT_DIR)/kernel.elf
	@cp grub/grub.cfg $(GRUB_DIR)/grub.cfg
	cp $(MODULES) $(MODULES_DIR)/
	@$(GRUB_MKRESCUE) -o $(ISO_IMG) $(ISO_DIR) --modules="multiboot part_msdos fat"
	@echo ">>> ISO generada: $(ISO_IMG)"

# --------------------------------------------------------
# 5) Ejecutar en QEMU
# --------------------------------------------------------
run: iso
	$(QEMU) -cdrom $(ISO_IMG) -m 512M -vga std -serial stdio

# --------------------------------------------------------
# 6) Limpiar
# --------------------------------------------------------
clean:
	@rm -f $(ASM_OBJS) $(CPP_OBJS) $(KERNEL_ELF) $(ISO_IMG)
	@rm -rf $(ISO_DIR)

# --------------------------------------------------------
# 7) Herramienta para crear discos FAT32
# --------------------------------------------------------
fat_tool:
	$(CXX) -std=c++17 -O2 tools/fat_tool.cpp -o fat_tool