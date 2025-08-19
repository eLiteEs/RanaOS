# Herramientas
NASM      := nasm
CXX       := g++
CC        := gcc
LD        := ld
CARGO     := cargo
GRUB_MKRESCUE := grub-mkrescue
QEMU      := qemu-system-i386

# Flags
CXXFLAGS := -m32 -ffreestanding -O2 \
    -fno-exceptions -fno-rtti -fno-pie -fno-pic \
    -std=gnu++17 -Ikernel

CFLAGS   := -m32 -ffreestanding -O2 -Wall -Wextra \
    -fno-pie -fno-pic \
    -std=gnu17 -Ikernel

RUST_TARGET := i686-unknown-linux-gnu
RUST_LIB := rust/target/$(RUST_TARGET)/release/libgraphicsm.a
RUSTFLAGS := -C opt-level=2 -C panic=abort -C link-arg=-nostartfiles
LDFLAGS := -m elf_i386 -nostdlib -L$(shell dirname $$(gcc -m32 -print-libgcc-file-name))

# Directorios
ISO_DIR   := isodir
BOOT_DIR  := $(ISO_DIR)/boot
GRUB_DIR  := $(BOOT_DIR)/grub

# Fuentes
ASM_SRCS := boot.asm
ASM_OBJS := $(patsubst %.asm, %.o, $(ASM_SRCS))

CPP_SRCS := kernel/kernel.cpp \
    	kernel/Console.cpp \
    	kernel/io.cpp \
    	kernel/Graphics.cpp \
    	kernel/string.cpp \
    	kernel/memory.cpp \
    	kernel/math.cpp \
	kernel/idt.cpp \
	kernel/vgraphics.cpp \
	kernel/Debug.cpp \
	kernel/vgraphics_wrapper.cpp \
	kernel/date_wrapper.cpp \
	kernel/date.cpp \
	kernel/wait_wrapper.cpp \
	kernel/pic.cpp \
	kernel/io_wrapper.cpp \
	kernel/pic_wrapper.cpp
CPP_OBJS := $(patsubst kernel/%.cpp, %.o, $(CPP_SRCS))

LDSCRIPT := kernel/linker.ld

MODULES = files/text.txt files/bg.pim files/tux.pim
MODULES_DIR  := $(ISO_DIR)/files

# Salidas
KERNEL_ELF := kernel.elf
ISO_IMG    := RanaOS.iso

.PHONY: all clean iso run krun

all: iso

# Ensamblar ASM
%.o: %.asm
	$(NASM) -f elf32 $< -o $@

# Compilar C++
%.o: kernel/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

graphicm.o: rust/src/lib.rs
	rustc --target=i686-unknown-linux-gnu \
	      -C opt-level=2 \
	      -C panic=abort \
	      -C link-arg=-m32 \
	      --emit=obj \
	      -o $@ \
	      $<

# Linkear kernel ELF
$(KERNEL_ELF): $(ASM_OBJS) $(CPP_OBJS) graphicm.o
	$(LD) $(LDFLAGS) -T $(LDSCRIPT) -o $@ $^ -lgcc

# Generar ISO booteable
iso: $(KERNEL_ELF)
	@mkdir -p $(GRUB_DIR)
	@mkdir -p $(MODULES_DIR)
	@cp $(KERNEL_ELF) $(BOOT_DIR)/kernel.elf
	@cp grub/grub.cfg $(GRUB_DIR)/grub.cfg
	cp $(MODULES) $(MODULES_DIR)/
	@$(GRUB_MKRESCUE) -o $(ISO_IMG) $(ISO_DIR) --modules="multiboot part_msdos fat"
	@echo ">>> ISO generada: $(ISO_IMG)"

# Ejecutar en QEMU
run:
	$(QEMU) -cdrom $(ISO_IMG) -m 512M -vga std -serial stdio -boot d

krun:
	$(QEMU) -kernel $(KERNEL_ELF) -m 512M -vga std -serial stdio -hda disk1gb.qcow2 -boot d

# Limpiar
clean:
	@rm -f $(ASM_OBJS) $(CPP_OBJS) rust.o $(KERNEL_ELF) $(ISO_IMG)
	@rm -rf $(ISO_DIR)
	cd rust && cargo clean

disk:
	qemu-img create -f qcow2 disk1gb.qcow2 1G

ext2-tool:
	gcc -O2 -std=c11 kernel/ffext2/host_blockdev.cpp kernel/ffext2/ext2.cpp -o host_blockdev

