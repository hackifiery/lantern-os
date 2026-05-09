#define KERN
#include "io.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <stdio.h>
#include "idt.h"
#include "sys.h"
#include "gdt.h"
#include "ata.h"
#include "ustar.h"
#include "api.h"
#include "version.h"
#include "mem.h"
#include "sh.h"

#define HEAP_START 0x00100000
#define HEAP_SIZE  (1024 * 1024) // 1mb

void* sbrk(int increment);

struct KernelAPI api;

void kmain(unsigned int entryCount, struct E820Entry* entries) {
    /*volatile unsigned short *vga = (volatile unsigned short *)0xB8000;
    vga[0] = 0x0F41;
    goto DEBUG;*/
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

    /*fmtWrite("memory map:\n");
    for (unsigned int i = 0; i < entryCount; i++) {
        fmtWrite("  base=%x (%dk) len=%dk type=%d\n",
            (unsigned long int)entries[i].base,
            (unsigned long int)entries[i].base/1024,
            (unsigned int)entries[i].length/1024,
            entries[i].type);
    }*/

    enableCursor(14, 15);
    struct MemoryInfo mem;
    mem.entry_count = entryCount;
    mem.entries = entries;
    init(memInit((void*)HEAP_START, HEAP_SIZE), "memory manager");

    fmtWrite("\nWelcome to the lanternOS shell\nReport bugs at https://github.com/hackifiery/lantern-os.\n");
    fmtWrite("Type 'help' for commands.\n\n");
    #undef init
    goto DEBUG;
    for (;;) sh(&mem, &api);
    return;

    DEBUG: for (int i = 0; i < entryCount; i++) {
        fmtWrite("Memory Entry %d: base=%x (%dk) len=%dk type=%d\n",
            i,
            (unsigned long int)entries[i].base,
            (unsigned long int)entries[i].base/1024,
            (unsigned int)entries[i].length/1024,
            entries[i].type);
    }
    fmtWrite("strlen test: %d\n", strlen("hello"));  // should print 5
    fmtWrite("strcmp test: %d\n", strcmp("abc", "abc"));  // should print 0
    void *p = malloc(64);
    if (p) fmtWrite("malloc ok: %x\n", (unsigned int)p);
    else   fmtWrite("malloc failed\n");fmtWrite("testing sbrk\n");
    void *sb = sbrk(256);
    fmtWrite("sbrk returned: %x\n", (unsigned int)sb);
    fmtWrite("testing printf\n");
    //for(;;);
    printf("printf works: %d\n", 42);
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