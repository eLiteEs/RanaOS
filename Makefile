# Herramientas
NASM := nasm
CXX  := g++

# Flags de compilación
CXXFLAGS := -m32 -ffreestanding -O2 -Wall -Wextra \
            -fno-exceptions -fno-rtti -fno-pie -fno-pic \
            -std=gnu++17

# Flags de link
LDFLAGS := -static -nostdlib -nostartfiles

.PHONY: all clean iso run

# Objetivos
ASM_SRCS := boot.asm
CPP_SRCS := kernel.cpp Keyboard.cpp Console.cpp

ASM_OBJS := $(ASM_SRCS:.asm=.o)
CPP_OBJS := $(CPP_SRCS:.cpp=.o)

all: iso

# 1) Ensamblar boot.S
%.o: %.asm
	$(NASM) -f elf32 $< -o $@

# 2) Compilar C++
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 3) Linkear todo en un ELF
kernel.elf: $(ASM_OBJS) $(CPP_OBJS) linker.ld
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -T linker.ld \
		-o $@ $(ASM_OBJS) $(CPP_OBJS)	

# 4) ISO de arranque (usamos el ELF directamente)
iso: kernel.elf
	mkdir -p isodir/boot/grub
	cp kernel.elf isodir/boot/kernel.elf
	echo 'set timeout=0'                         > isodir/boot/grub/grub.cfg
	echo 'set default=0'                        >> isodir/boot/grub/grub.cfg
	echo ''                                     >> isodir/boot/grub/grub.cfg
	echo 'menuentry "MiOS 32-bit" {'            >> isodir/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.elf'         >> isodir/boot/grub/grub.cfg
	echo '  boot'                               >> isodir/boot/grub/grub.cfg
	echo '}'                                    >> isodir/boot/grub/grub.cfg
	grub-mkrescue -o miOS.iso isodir
	rm -rf isodir

run: iso
	qemu-system-i386 -cdrom miOS.iso -m 512M -curses

clean:
	rm -f *.o *.elf *.iso
	rm -rf isodir

