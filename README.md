# LanternOS
LanternOS is a hobbyist OS and kernel made by me to explore how OS's work. It currently has an IDT, a basic GDT, IRQ and ISRs, and a basic keyboard and VGA text-mode driver. It also uses a custom bootloader called Lightbulb. It runs on 32-bit x86, and although it boots on x86_64, 64-bit hasn't been thoroughly tested yet and the binaries are still 32-bit.
## lanternCOM
lanternCOM is the basic shell for LanternOS. It only has a couple of commands so far.

- `help`: Lists all available commands in the current build.
- `echo [text]`: Repeats the provided string back to the console.
- `cls`: Clears the video buffer and resets the cursor position.
- `ping`: Prints "Pong!".
- `sysinfo`: Displays the current OS version, build date, and target architecture.
- `mem [flag]`: Reports memory statistics (not very accurate).
    - `/m`=mb, `/k`=kb, `/b`=bytes, `/g`=gb, no flag=kb
- `calc`: a simple calculator.
- `panic [code]`: Triggers a kernel panic with an optional fault code.
- `reboot` and `shutdown`: self-explanitory

## Screenshots
![start](samples/lanternStart.png)
![stuff](samples/lanternStuff.png)
![panic](samples/lanternPanic.png)
