# User Docs: 1. Running RanaOS in a Virtual Machine

RanaOS doesn't need lots of resources to work properly, it's very lightweight. So on the Virtual Machine you need simple configurations.

At the moment I tried Oracle's VirtualBox, QEMU and VMWare Workstation. VMWare didn't worked (I don't know if it was my fault) but VirtualBox and QEMU are the best options at the moment. Here's a guide of how to setup RanaOS for this Virtual Machines.

## QEMU

QEMU is an open-source virtualization software which can be runned on Windows and Linux from a command.

First you need to download the `.iso` of RanaOS and have QEMU installed on your computer.

For running in QEMU this is the command:

`qemu-system-i386 -cdrom RanaOS.iso -m 512M -vga std -serial stdio -boot d -rtc base=localtime`

This runs a 32-bit QEMU virtual machine running RanaOS with 512MB of memory, serial output to console, enables vga and sets the virtual machine's time to the computer's time.

If your PC isn't very powerfull you can use this command instead:

`qemu-system-i386 -cdrom RanaOS.iso -m 16M -vga std -serial stdio -boot d -rtc base=localtime`

This initializes the virtual machine with 16MB instead of 512MB.

## VirtualBox

Oracle's VirtualBox is another virtualization software which is simpler than QEMU. It has a GUI which lets the user make virtual machines easily.

This are the minimun requisites for the virtual machine:

- 128MB RAM
- 64MB VRAM
- No disk or 1GB disk
- No EFI, network, USB
- Optional, serial port for debugging

With all that options you can setup a virtual machine for RanaOS.

There's only one problem, for some reason, VirtualBox doesn't show resolutions like 1920x1080 using GRUB Framebuffer, so the solution is on the menu that appears when you boot the virtual machine, pick the last option which isn't reboot. That loads RanaOS in a resolution that works on VirtualBox.

## Other virtualization softwares

I haven't tried other softwares but QEMU and VirtualBox are the best options right now.

## Real machines

At the moment it isn't safe to run RanaOS in a real machine, the time I have tried the computer just started beeping for some reason and I had to turn it off, I'm working in a solution to that problem.