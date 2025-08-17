[![Build RanaOS ISO](https://github.com/eLiteEs/RanaOS/actions/workflows/build.yml/badge.svg)](https://github.com/eLiteEs/RanaOS/actions/workflows/build.yml)

# RanaOS
32-bit OS made using C++, C and Assembly

## How to generate a .iso

Simply run `make` on a Ubuntu Linux machine (I'm adding support to Windows later) and that compiles all sources into a booteable `.iso`.

For running the `.iso`, you can run `make run`, it uses qemu.

## Can I collaborate?

Of course you can, you can add commands, features and when it's available, also executables for RanaOS.

## Can I use the code?

Yeah, although it isn't very commented and some parts are in Spanish, you can fork freely this repo. This repo is a project I started thinking not long ago for making OS development easier than it's now.

## What languages does it use?

The bootloader is coded in assembly. The kernel and command line are coded in a raw C++ which I made some functions for it looking like C#. Most of the functions are using C++, C and Assembly for low-level tools.

The UI it's going to be made with Rust and other programs and tools maybe are written in Go, Fortan or COBOL.

## Recommended tools to have instaled

On my Ubuntu WSL I have instaled `qemu`, `nasm`, `xorriso`, `grub_mkrescue`, `binutils`, `mtools`, rust and `g++`. (maybe some are missing)

## Credits
- Also I have used code from [hubenchang0515/font8x16](https://github.com/hubenchang0515/font8x16) (MIT License) in CGraphics for adding fonts to graphical console mode, thanks!

## License

License (GNU GPLv3) available at `LICENSE` file.
