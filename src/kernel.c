#define KERN
#include "io.h"
#include "kstdint.h"
#include "string_utils.h"
#include "idt.h"
#include "sys.h"
#include "gdt.h"
#include "ata.h"
#include "ustar.h"
#include "api.h"
#include "version.h"
#include "sh.h"

struct KernelAPI api;

void kmain(unsigned int entryCount, struct E820Entry* entries) {
    fmtWrite("\n");
    moveCursor(0,0);
    clearScreen();
    fmtWrite("LanternOS v%s, copyright (c) 2026 hackifiery. All rights reserved.\n\n", VER);
    #define init(f, name) \
        fmtWrite("Initializing %s...", name); \
        f; \
        fmtWrite("ok\n")
    init(initIdt(), "IDT");
    init(initGdt(), "GDT");
    init(__asm__ volatile("sti"), "interrupts");
    init(ataIdentify(), "ATA");
    init(tarLoad(), "tar driver");
    init(initTimer(100), "timer");

    api.fmtWrite  = fmtWrite;
    api.fmtGet    = fmtGet;
    api.sfmtWrite = sfmtWrite;
    api.strcmp    = strcmp;
    api.atoi      = atoi;

    enableCursor(14, 15);
    struct MemoryInfo mem;
    mem.entry_count = entryCount;
    mem.entries = entries;

    fmtWrite("\nWelcome to the lanternCOM shell!\nReport bugs at https://github.com/hackifiery/lantern-os.\n");
    fmtWrite("Type 'help' for commands.\n\n");
    #undef init
    for (;;) sh(&mem, &api);
}

/* DEBUG
void kmain() {
    clearScreen();
    uint16_t* vga = (uint16_t*)0xb8000;
    vga[0] = 0x1f41; // Blue background, White 'A'

    initSerial();
    vga[1] = 0x1f42; // White 'B'

    initGdt();
    vga[2] = 0x1f43; // White 'C'

    initIdt();
    vga[3] = 0x1f44; // White 'D'

    fmtWrite("hi");

    while(1) { __asm__ ("hlt"); }
}
*/