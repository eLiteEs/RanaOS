# User Docs: 1. Running RanaOS in a Virtual Machine

RanaOS doesn't need lots of resources to work properly, it's very lightweight. So on the Virtual Machine you need simple configurations.

At the moment I tried Oracle's VirtualBox, QEMU and VMWare Workstation. VMWare didn't worked (I don't know if it was my fault) but VirtualBox and QEMU are the best options at the moment. Here's a guide of how to setup RanaOS for this Virtual Machines.

## QEMU

QEMU is an open-source virtualization software which can be runned on Windows and Linux from a command.

First you need to download the `.iso` of RanaOS and have QEMU installed on your computer.

For running in QEMU this is the command:

`qemu-system-i386 -cdrom RanaOS.iso -m 512M -vga std -serial stdio -boot d -rtc base=localtime`

This runs a 32-bit QEMU virtual machine running RanaOS with 512MB of memory, serial output to console 