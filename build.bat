@echo off
setlocal

rem === Ruta directa al toolchain cruzado ===
set TOOLCHAIN=C:\Users\blasf\Downloads\i686-elf-tools-windows\bin
set CXX="%TOOLCHAIN%\i686-elf-g++"
set LD="%TOOLCHAIN%\i686-elf-ld"
set NASM=nasm

set CXXFLAGS=-m32 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -fno-pie -fno-pic -std=gnu++17
set LDFLAGS=-m elf_i386

echo =======================
echo = Compilando ASM...  =
echo =======================
%NASM% -f elf32 boot.asm -o boot.o
%NASM% -f elf32 kernel/io.asm -o io.o

echo =======================
echo = Compilando C++...  =
echo =======================
%CXX% %CXXFLAGS% -c kernel/kernel.cpp -o kernel.o
%CXX% %CXXFLAGS% -c kernel/Console.cpp -o console.o
%CXX% %CXXFLAGS% -c kernel/floppy.cpp -o floppy.o
%CXX% %CXXFLAGS% -c kernel/fatnenuphar.cpp -o fatnenuphar.o
%CXX% %CXXFLAGS% -c kernel/disk.cpp -o disk.o

echo ===========================
echo = Linkeando kernel.elf... =
echo ===========================
%LD% %LDFLAGS% -T kernel/linker.ld -o kernel.elf ^
  boot.o io.o ^
  kernel.o console.o floppy.o fatnenuphar.o disk.o

echo =======================
echo = Preparando ISO...  =
echo =======================
mkdir isodir\boot\grub 2>nul
copy kernel.elf isodir\boot\
copy grub\grub.cfg isodir\boot\grub\
copy grub\bg.png isodir\boot\grub\

echo ===========================
echo = Generando RanaOS.iso... =
echo ===========================
grub-mkrescue -o RanaOS.iso isodir --modules="multiboot part_msdos"

if %errorlevel% neq 0 (
    echo ERROR: No se pudo generar la ISO. ¿Está grub-mkrescue en el PATH?
    goto fin
)

echo ======================
echo = Limpiando campo... =
echo ======================
rd /s /q isodir

echo.
echo ✅ ¡RanaOS.iso creado con éxito!
:fin
endlocal
pause

