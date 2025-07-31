# Makefile para RanaOS bootable ISO

# Herramientas
NASM		  := nasm
CXX		   := g++
CC			:= gcc
LD			:= ld
GRUB_MKRESCUE := grub-mkrescue
QEMU		  := qemu-system-i386

# Flags
CXXFLAGS := -m32 -ffreestanding -O2 -Wall -Wextra \
	-fno-exceptions -fno-rtti -fno-pie -fno-pic \
	-std=gnu++17 -Ikernel
LDFLAGS := -m elf_i386 -nostdlib -L/usr/lib/gcc/x86_64-linux-gnu/11/32
CFLAGS   := -m32 -ffreestanding -O2 -Wall -Wextra \
	-fno-pie -fno-pic \
	-std=gnu17 -Ikernel

# Directorios
ISO_DIR   := isodir
BOOT_DIR  := $(ISO_DIR)/boot
GRUB_DIR  := $(BOOT_DIR)/grub

# Fuentes
ASM_SRCS := boot.asm threads.asm kernel/exceptions.asm
ASM_OBJS := $(patsubst %.asm, %.o, $(ASM_SRCS))

CPP_SRCS := kernel/kernel.cpp \
	kernel/Console.cpp \
	kernel/io.cpp \
	kernel/disk.cpp \
	kernel/filesystem.cpp \
	kernel/Graphics.cpp \
	kernel/string.cpp \
	kernel/memory.cpp \
	kernel/math.cpp \
	kernel/idt.cpp \
	kernel/vesa.cpp \
	kernel/vgraphics.cpp
CPP_OBJS := $(patsubst kernel/%.cpp, %.o, $(CPP_SRCS))

#C_SRCS := kernel/vesa.c
#C_OBJS := $(patsubst kernel/%.c, %.o, $(C_SRCS))

LDSCRIPT := kernel/linker.ld

MODULES = files/text.txt files/bg.pim files/tux.pim
MODULES_DIR  := $(ISO_DIR)/files

# Salidas
KERNEL_ELF := kernel.elf
ISO_IMG	:= RanaOS.iso

.PHONY: all clear clean iso run

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
# 2.1) Compilar C (usando gcc en lugar de g++)
# --------------------------------------------------------
#%.o: kernel/%.c
#	$(CC) $(CFLAGS) -c $< -o $@

# --------------------------------------------------------
# 3) Linkear kernel ELF
# --------------------------------------------------------
$(KERNEL_ELF): $(ASM_OBJS) $(CPP_OBJS) $(C_OBJS) $(LDSCRIPT)
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
run: 
	$(QEMU) -cdrom $(ISO_IMG) -m 512M -vga std -serial stdio

krun: 
	$(QEMU) -kernel $(KERNEL_ELF) -m 512M -vga std -serial stdio


# --------------------------------------------------------
# 6) Limpiar
# --------------------------------------------------------
clean:
	@rm -f $(ASM_OBJS) $(CPP_OBJS) $(C_OBJS) $(KERNEL_ELF) $(ISO_IMG)
	@rm -rf $(ISO_DIR)