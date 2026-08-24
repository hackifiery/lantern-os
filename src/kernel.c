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

#ifndef __PICOLIBC_VERSION__
#error "Kernel must be built with picolibc"
#endif
#include <picolibc.h>
/*#define HEAP_START 0x00100000
#define HEAP_SIZE  (1024 * 1024) // 1mb*/
#define STOP for(;;);

//void* sbrk(int increment);

extern void jump_usermode(void);
struct KernelAPI api;

void kmain(unsigned int entryCount, struct E820Entry* entries) {
    /*volatile unsigned short *vga = (volatile unsigned short *)0xB8000;
    vga[0] = 0x0F41;
    goto DEBUG;*/
    printf("\n");
    moveCursor(0,0);
    clearScreen();
    
    printf("LanternOS v%s, copyright (c) 2026 hackifiery. All rights reserved.\n", VER);
    printf("Built with picolibc %s\n\n", __PICOLIBC_VERSION__);
    #define init(f, name) \
        printf("Initializing %s...", name); \
        f; \
        printf("ok\n")
    init(initIdt(), "IDT");
    init(initGdt(), "GDT");
    init(__asm__ volatile("sti"), "interrupts");
    init(ataIdentify(), "ATA");
    init(tarLoad(), "tar driver");
    init(initTimer(100), "timer");

    api.fmtWrite  = fmtWrite;
    api.fmtGet    = fmtGet;
    api.sfmtWrite = sfmtWrite;
    api.writeChar = writeChar;

    /*printf("memory map:\n");
    for (unsigned int i = 0; i < entryCount; i++) {
        printf("  base=%x (%dk) len=%dk type=%d\n",
            (unsigned long int)entries[i].base,
            (unsigned long int)entries[i].base/1024,
            (unsigned int)entries[i].length/1024,
            entries[i].type);
    }*/

    enableCursor(14, 15);
    struct MemoryInfo mem;
    mem.entry_count = entryCount;
    mem.entries = entries;
    //init(memInit((void*)HEAP_START, HEAP_SIZE), "memory manager");
    //jump_usermode();

    printf("\nWelcome to the lanternOS shell\nReport bugs at https://github.com/hackifiery/lantern-os.\n");
    printf("Type 'help' for commands.\n\n");
    #undef init
    //goto DEBUG;
    /*for (int i = 0; i < entryCount; i++) {
        printf("Memory Entry %d: base=%x (%dk) len=%dk type=%d\n",
            i,
            (unsigned long int)entries[i].base,
            (unsigned long int)entries[i].base/1024,
            (unsigned int)entries[i].length/1024,
            entries[i].type);
    }*/
    for (;;) sh(&mem, &api);
    return;

    DEBUG: 
    printf("strlen test: %d\n", strlen("hello"));  // should print 5
    printf("strcmp test: %d\n", strcmp("abc", "abc"));  // should print 0
    void *p = malloc(64);
    if (p) printf("malloc ok: %x\n", (unsigned int)p);
    else   printf("malloc failed\n");printf("testing sbrk\n");
    void *sb = sbrk(256);
    printf("sbrk returned: %x\n", (unsigned int)sb);
    printf("testing sprintf\n");
    //for(;;);
    char tmp[100];
    printf("printf works: %d\n", 42);
    int c = getchar();
    printf("You typed: %c\n", c);
    STOP
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

    printf("hi");

    while(1) { __asm__ ("hlt"); }
}
*/