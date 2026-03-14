#include "io.h"
#include "string_utils.h"

void kmain(void) {
    clearScreen();
    enableCursor(14, 15);
    
    char input[256];
    fmtWrite("Welcome to gahOS!\n");
    fmtWrite("Type 'help' for commands.\n");

    while(1) {
        fmtWrite("\nkernel> ");
        fmtGet("%s", input); // This uses your new IRQ1-buffered input!

        if (strcmp(input, "help") == 0) {
            fmtWrite("Available commands: help, clear, ping, ticks");
        } 
        else if (strcmp(input, "clear") == 0) {
            clearScreen();
        } 
        else if (strcmp(input, "ping") == 0) {
            fmtWrite("Pong!");
        } 
        else if (strcmp(input, "ticks") == 0) {
            // You can track a global 'volatile int timer_ticks' in idt.c
            // and extern it here to show uptime!
            fmtWrite("The system has been running for a while...");
        } 
        else {
            fmtWrite("Unknown command: %s", input);
        }
    }
}