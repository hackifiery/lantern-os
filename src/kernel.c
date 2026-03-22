#include "io.h"
#include "string_utils.h"
#include "idt.h"
#include "sys.h"
#include "gdt.h"
#include "ata.h"
#include "ustar.h"
#include "api.h"

#define VER "0.0.1"

#ifndef __BUILD_DATE__
#define __BUILD_DATE__ "unknown"
#endif

#ifndef __BUILD_ARCH__
#define __BUILD_ARCH__ "unknown"
#endif

struct KernelAPI api;

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
    unsigned short dskBuf[256];

    for(;;) {
        fmtWrite("lanternCOM -> ");
        fmtGet("%s", input);

        int tokenCount = tokenize(input, tokens, 16);
        if (tokenCount == 0) continue;
        #define cmd(s) else if (strcmp(tokens[0], s) == 0)

        if (strcmp(tokens[0], "") == 0) continue;

        cmd("help") {
            fmtWrite("Available: help, echo, clear, calc, ping, uptime, uname, free, cat, ls, panic, reboot, shutdown\n");
        }
        cmd("echo") {
            for(int i = 1; i < tokenCount; i++) {
                fmtWrite("%s ", tokens[i]);
            }
        }
        cmd("clear") {
            clearScreen();
            continue;
        }
        cmd("ping") fmtWrite("Pong!\n");
        cmd("uptime") {
            unsigned int seconds = sysTicks / 100;
            fmtWrite("%d s\n", seconds);
        }
        cmd("uname") {
            fmtWrite("lanternOS i386 v%s (built %s on %s)\n", VER, __BUILD_DATE__, __BUILD_ARCH__);
        }
        cmd("free") {
            unsigned int total = getTotalMem(mbPtr);
            unsigned int used = getUsedMem();
            if (strcmp(tokens[1], "-m") == 0)      fmtWrite("total = %dm, used = %dm, free = %dm\n", total/1024, used/1024, (total - used)/1024);
            else if (strcmp(tokens[1], "-g") == 0) fmtWrite("total = %dg, used = %dg, free = %dg\n", total/1024/1024, used/1024/1024, (total - used)/1024/1024);
            else if (strcmp(tokens[1], "-b") == 0) fmtWrite("total = %db, used = %db, free = %db\n", total*1024, used*1024, (total - used)*1024);
            else                                   fmtWrite("total = %dk, used = %dk, free = %dk\n", total, used, total - used);
        }
        cmd("panic") {
            if (atoi(tokens[1]) > 21 || atoi(tokens[1]) == 34 || atoi(tokens[1]) == 9 || atoi(tokens[1]) == 15 || atoi(tokens[1]) == 18 || atoi(tokens[1]) == 20) fmtWrite("Unknown fault interrupt\n");
            if (tokenCount == 2) sendInterrupt(atoi(tokens[1]));
            else userPanic();
        }
        cmd("ls") {
            //unsigned short buf[256];
            //ataRead(0, buf);
            tarLoad();
            tarList(tokens[1]);
            fmtWrite("\n");
        }
        cmd("cat") {
            tarLoad();
            tarPrintFile(tokens[1]);
            fmtWrite("\n");
        }
        cmd("reboot")   reboot();
        cmd("shutdown") shutdown();
        else {
            char *data = 0;
            int size = tarReadFile(tokens[0], &data);
            if (size == 0) { fmtWrite("not found: %s\n", tokens[0]); continue; }
            // copy to 0x200000
            unsigned char *dest = (unsigned char *)0x200000;
            for (int i = 0; i < size; i++) dest[i] = ((unsigned char *)data)[i];
            // jump to it
            /*DEBUG:fmtWrite("api addr: %x\n", (unsigned int)&api);
            fmtWrite("api.fmtWrite: %x\n", (unsigned int)api.fmtWrite);
            fmtWrite("jumping to: %x\n", (unsigned int)0x200000);
            fmtWrite("calc bytes: %02x %02x %02x %02x\n",
                    dest[0], dest[1], dest[2], dest[3]);*/
            typedef void (*Program)(struct KernelAPI *);
            Program prog = (Program)0x200000;
            prog(&api);
            //fmtWrite("Unknown command: %s", tokens[0]);
        }

        //fmtWrite("\n");
        #undef cmd
    }
}

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