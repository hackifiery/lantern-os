#include "io.h"
#include "string_utils.h"
#include "idt.h"
#include "sys.h"
#include "gdt.h"

extern void halt(void);

void kmain(void) {
    initIdt();
    initGdt();
    clearScreen();
    initTimer(100);
    enableCursor(14, 15);
    // __asm__ volatile("sti");
    
    char input[256];
    fmtWrite("Welcome to lanternOS!\n");
    fmtWrite("Type 'help' for commands.\n\n");

    while(1) {
        fmtWrite("-> ");
        fmtGet("%s", input);
        if (strcmp(input, "") == 0) continue;

        if (strcmp(input, "help") == 0) {
            fmtWrite("Available commands: help, clear, ping, uptime");
        } 
        else if (strcmp(input, "clear") == 0) {
            clearScreen();
        } 
        else if (strcmp(input, "ping") == 0) {
            fmtWrite("Pong!");
        } 
        else if (strcmp(input, "uptime") == 0) {
            unsigned int seconds = sysTicks / 100;
            unsigned int milliseconds = (sysTicks % 100) * 10;
            
            fmtWrite("%d.%d s", seconds, milliseconds);
        }
        else if (strcmp(input, "reboot") == 0) {
            reboot();
        }
        else {
            fmtWrite("Unknown command: %s", input);
        }
        fmtWrite("\n");
    }
}