# LanternOS
LanternOS is a hobbyist OS and kernel made by me to explore how OS's work. It currently has an IDT, a basic GDT, IRQ and ISRs, a basic keyboard and VGA text-mode driver, and an ATA and USTAR driver. It also uses a custom bootloader called Lightbulb. It runs on 32-bit x86, and although it boots on x86_64, 64-bit hasn't been thoroughly tested yet and the binaries are still 32-bit.
## lanternCOM
lanternCOM is the basic shell for LanternOS. It only has a couple of commands so far.

- `help`: Lists all available commands in the current build.
- `echo [text]`: Repeats the provided string back to the console.
- `clear`: Clears the video buffer and resets the cursor position.
- `ping`: Prints "Pong!".
- `uname`: Displays the current OS version, build date, and target architecture.
- `free [flag]`: Reports memory statistics (not very accurate).
    - `-m`=mb, `-k`=kb, `-b`=bytes, `-g`=gb, no flag=kb
- `calc`: a simple calculator.
- `panic [code]`: Triggers a kernel panic with an optional fault code.
- `ls` and `cat`: self-explanitory
- `reboot` and `shutdown`: self-explanitory

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
If you're on windows, you have 2 options: use WSL and build for linux, or use some minGW toolchain (only cygwin has been tested). The instructions and prerequisites are the same as linux.
