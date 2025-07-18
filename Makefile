# Makefile para RanaOS bootable ISO

# Herramientas
NASM		  := nasm
CXX		   	  := g++
LD			  := ld
GRUB_MKRESCUE := grub-mkrescue
QEMU		  := qemu-system-i386
C             := gcc

# Flags
CXXFLAGS := -m32 -ffreestanding -O2 -Wall -Wextra \
	-fno-exceptions -fno-rtti -fno-pie -fno-pic \
	-std=gnu++17 -Ikernel
LDFLAGS := -m elf_i386 -nostdlib
CFLAGS := -m32 -ffreestanding -O2 -Wall -Wextra \
    -fno-pie -fno-pic \
    -std=gnu17 -Ikernel

# Directorios
ISO_DIR   := isodir
BOOT_DIR  := $(ISO_DIR)/boot
GRUB_DIR  := $(BOOT_DIR)/grub
FILES_SRC_DIR = files
FILES_DEST_DIR = $(ISO_DIR)/files

FILES = $(wildcard $(FILES_SRC_DIR)/*.bin)

# Fuentes	
ASM_SRCS := boot.asm threads.asm
ASM_OBJS := boot.o threads.o

CPP_SRCS := kernel/kernel.cpp \
	kernel/Console.cpp \
	kernel/io.cpp \
	kernel/fat32.cpp \
	kernel/disk.cpp \
	kernel/floppy.cpp \
	kernel/Graphics.cpp \
	kenrel/Font.c

CPP_OBJS := kernel.o \
	console.o \
	io.o \
	fat32.o \
	disk.o \
	floppy.o \
	Graphics.o \
	Font.o

LDSCRIPT := kernel/linker.ld

# Salidas
KERNEL_ELF := kernel.elf
ISO_IMG	:= RanaOS.iso

.PHONY: all clean iso run

all: iso

# --------------------------------------------------------
# 1) Ensamblar ASM
# --------------------------------------------------------
boot.o: boot.asm
	$(NASM) -f elf32 $< -o $@

threads.o: threads.asm
	$(NASM) -f elf32 $< -o $@

# --------------------------------------------------------
# 2) Compilar C++
# --------------------------------------------------------
kernel.o: kernel/kernel.cpp kernel/Console.h kernel/Keyboard.h kernel/io.h \
		  kernel/idt.h kernel/pic.h kernel/fat32.h kernel/Graphics.h kernel/Font.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

console.o: kernel/Console.cpp kernel/Console.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

io.o: kernel/io.cpp kernel/io.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

disk.o: kernel/disk.cpp kernel/disk.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

fat32.o: kernel/fat32.cpp kernel/fat32.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

floppy.o: kernel/floppy.cpp kernel/floppy.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Graphics.o: kernel/Graphics.cpp kernel/Graphics.h kernel/multiboot.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

Font.o: kernel/Font.c kernel/Font.h
	$(C) $(CFLAGS) -c $< -o $@

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
	@cp grub/bg-fixed.png $(GRUB_DIR)/bg.png
	@$(GRUB_MKRESCUE) -o $(ISO_IMG) $(ISO_DIR) \
		--modules="multiboot part_msdos"
	@rm -rf $(ISO_DIR)
	@echo ">>> ISO creada: $(ISO_IMG)"

# --------------------------------------------------------
# 5) Arrancar en QEMU
# --------------------------------------------------------
run: iso
	$(QEMU) -cdrom $(ISO_IMG) -m 512M -vga std

# --------------------------------------------------------
# 6) Limpiar
# --------------------------------------------------------
clean:
	@rm -f *.o $(CPP_OBJS) $(ASM_OBJS) \
			 $(KERNEL_ELF)

# --------------------------------------------------------
# 7) Generate FATnenuphar bin
# --------------------------------------------------------
fatnenuphar:
	g++ -std=c++17 -O2 fatnenuphar.cpp -o fatnenuphar