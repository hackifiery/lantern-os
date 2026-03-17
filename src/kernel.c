#include "io.h"
#include "string_utils.h"
#include "idt.h"
#include "sys.h"
#include "gdt.h"

#define VER "0.0.1"

#ifndef __BUILD_DATE__
#define __BUILD_DATE__ "unknown"
#endif

#ifndef __BUILD_ARCH__
#define __BUILD_ARCH__ "unknown"
#endif


static int tokenize(char* str, char** tokens, int max_tokens) {
    int count = 0;
    int in_token = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] != ' ' && !in_token) {
            if (count < max_tokens) {
                tokens[count++] = &str[i];
                in_token = 1;
            }
        } else if (str[i] == ' ') {
            str[i] = '\0';
            in_token = 0;
        }
    }
    return count;
}

static void com(struct MemoryInfo* mbPtr) {
    char input[256];
    char* tokens[16]; 

    for(;;) {
        fmtWrite("lanternCOM -> ");
        fmtGet("%s", input); 

        int tokenCount = tokenize(input, tokens, 16);
        if (tokenCount == 0) continue;
        #define cmd(s) else if (strcmp(tokens[0], s) == 0)

        if (strcmp(tokens[0], "") == 0) continue;
        
        cmd("help") {
            fmtWrite("Available: help, echo, cls, ping, uptime, sysinfo, mem, panic, reboot, shutdown");
        }
        cmd("echo") {
            for(int i = 1; i < tokenCount; i++) {
                fmtWrite("%s ", tokens[i]);
            }
        }
        cmd("cls") {
            clearScreen();
            continue;
        }
        cmd("ping") fmtWrite("Pong!");
        cmd("uptime") {
            unsigned int seconds = sysTicks / 100;
            fmtWrite("%d s", seconds);
        }
        cmd("sysinfo") {
            fmtWrite("lanternOS i386 v%s (built %s on %s)", VER, __BUILD_DATE__, __BUILD_ARCH__);
        }
        cmd("mem") {
            // fmtWrite("Still workin' on it...\n"); continue; // TODO: fix mem function
            unsigned int total = getTotalMem(mbPtr);
            unsigned int used = getUsedMem();
            if (strcmp(tokens[1], "/m") == 0)      fmtWrite("total = %dm, used = %dm, free = %dm", total/1024, used/1024, (total - used)/1024);
            else if (strcmp(tokens[1], "/g") == 0) fmtWrite("total = %dg, used = %dg, free = %dg", total/1024/1024, used/1024/1024, (total - used)/1024/1024);
            else if (strcmp(tokens[1], "/b") == 0) fmtWrite("total = %db, used = %db, free = %db", total*1024, used*1024, (total - used)*1024);
            else                                   fmtWrite("total = %dk, used = %dk, free = %dk", total, used, total - used);
        }
        cmd("panic") {
            if (atoi(tokens[1]) > 21 || atoi(tokens[1]) == 34 || atoi(tokens[1]) == 9 || atoi(tokens[1]) == 15 || atoi(tokens[1]) == 18 || atoi(tokens[1]) == 20) fmtWrite("Unknown fault interrupt");
            if (tokenCount == 2) sendInterrupt(atoi(tokens[1]));
            else panic();
        }
        cmd("reboot")   reboot();
        cmd("shutdown") shutdown();
        else {
            fmtWrite("Unknown command: %s", tokens[0]);
        }

        fmtWrite("\n");
        #undef cmd
    }
}

void kmain(unsigned int entryCount, struct E820Entry* entries) {
    fmtWrite("\n");
    moveCursor(0,0);
    clearScreen();
    fmtWrite("LanternOS kernel v%s, copyright (c) 2026 hackifiery. All rights reserved.\n\n", VER);
    #define init(f, name) \
        fmtWrite("Initializing %s...", name); \
        f; \
        fmtWrite("ok\n")
    init(initIdt(), "IDT");
    init(initGdt(), "GDT");
    init(__asm__ volatile("sti"), "interrupts");
    init(initTimer(100), "timer");

    enableCursor(14, 15);
    struct MemoryInfo mem;
    mem.entry_count = entryCount;
    mem.entries = entries;

    fmtWrite("\nWelcome to lanternOS!\n");
    fmtWrite("Type 'help' for commands.\n\n");
    #undef init
    for (;;) com(&mem);
}

/* DEBUG
void kmain() {
    clearScreen();
    unsigned short* vga = (unsigned short*)0xb8000;
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