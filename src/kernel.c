#include "io.h"
#include "string_utils.h"
#include "idt.h"
#include "sys.h"
#include "gdt.h"

extern void halt(void);

static void com(void) {
    char input[256];

    for(;;) {
        fmtWrite("lanternCOM -> ");
        fmtGet("%s", input);
        // lower(input);
        #define cmd(s) else if (strcmp(input, s) == 0)

        if (strcmp(input, "") == 0) continue;

        cmd("help") {
            fmtWrite("Available commands: help, com, exit, cls, ping, uptime, reboot, shutdown");
        }
        cmd("com") com();
        cmd("exit") return;
        cmd("cls") {
            clearScreen();
            continue;
        }
        cmd("ping") fmtWrite("Pong!");
        cmd("uptime") {
            unsigned int seconds = sysTicks / 100;
            unsigned int milliseconds = (sysTicks % 100) * 10;
            
            fmtWrite("%d.%d s", seconds, milliseconds);
        }
        cmd("reboot")   reboot();
        cmd("shutdown") shutdown();
        else {
            fmtWrite("Unknown command: %s", input);
        }
        fmtWrite("\n");
        #undef cmd
    }
}

void kmain(void) {
    initIdt();
    initGdt();
    clearScreen();
    initTimer(100);
    enableCursor(14, 15);
    // __asm__ volatile("sti");
    fmtWrite("Welcome to lanternOS!\n");
    fmtWrite("Type 'help' for commands.\n\n");
    for (;;) com();
}