# LanternOS
LanternOS is a hobbyist OS and kernel made by me to explore how OS's work. It currently has an IDT, a basic GDT, IRQ and ISRs, a basic keyboard and VGA text-mode driver, and an ATA and USTAR fs driver. It also uses a custom bootloader called Lightbulb. It runs on 32-bit x86, and although it boots on x86_64, 64-bit hasn't been thoroughly tested yet and the binaries are still 32-bit.

## Building
### Prereqs
- An `i686-elf` cross toolchain (prefferably, native toolchains can also be used, i think...)
    - `gcc` w/c99+ support
    - `ld`
    - `objcopy`
- `nasm`
- `make`
- `qemu-system-i386` (if you want to test out the built image)
### Linux
To build, simply run
```sh
make
```
To run the image in qemu, run
```
make run
```
### Windows
~~If you're on windows, you have 2 options: use WSL and build for linux, or use some minGW toolchain (only cygwin has been tested). The instructions and prerequisites are the same as linux.~~
Windows doesn't work at the moment, your only choice is to use WSL or a VM. will try to fix...