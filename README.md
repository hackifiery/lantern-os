# LanternOS
LanternOS is a hobbyist OS and kernel made by me to explore how OS's work. It currently has an IDT, a basic GDT, IRQ and ISRs, a basic keyboard and VGA text-mode driver, and an ATA and USTAR fs driver. It also uses a custom bootloader called Lightbulb. It runs on 32-bit x86, and although it boots on x86_64, 64-bit hasn't been thoroughly tested yet and the binaries are still 32-bit. Picolibc has been ported, although very unstable.

## Building
### Prereqs
- An `i686-elf` cross toolchain with picolibc (prefferably, native toolchains can also be used, i think...)
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
Not supported :sob:
## Goals for v0.0.2
- Migrate from newlib to picolibc
- Decent picolibc support (such as basic i/o functions, math/string parsing functions, etc.)
- POSIX-like read/write interface
## TODO
- Add native apps
- GUI (maybe?)