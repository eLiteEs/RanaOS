# RanaOS
32-bit OS made using C++, C and Assembly

## How to generate a .iso

Simply run `make` on a Linux machine (I'm adding support to windows later) and that compiles all sources into a booteable `.iso`.

For running it, I recommend using VirtualBox, although there's and option in `make run` for using qemu which is easier.
The VM should have recommended 64 MB of RAM, 9 MB of video and 1 CPU.

I'm working on FATnenuphar, the custom file system of RanaOS. Soon I'll make a binary for generating `.vdi`s of FATnenuphar.

## Can I collaborate?

Of course you can, you can add commands, features and when it's available, also executables for RanaOS.

## Can I use the code?

Yeah, although it isn't very commented and some parts are in Spanish, you can fork freely this repo. This repo is a project I started thinking not long ago for making OS development easier than it's now.

## What languages does it use?

The bootloader (i'm not sure if it's being used) is coded in assembly. The kernel and command line are coded in a raw C++ which I made somw functions for it looking like C#. Most of the functions are using C++, C and Assembly for low-level tools.

## Recommended tools to have instaled

On my Ubuntu WSL I have instaled `qemu`, `nasm`, `xorriso`, `grub_mkrescue`, `binutils`, `mtools` and `g++`. (maybe some are missing)

## License

License (GNU GPLv3) available at `LICENSE` file.
