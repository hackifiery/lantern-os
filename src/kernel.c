#include "io.h"
#include "string_utils.h"
#include "idt.h"

void kmain(void) {
    clearScreen();
    initTimer(100);
    enableCursor(14, 15);
    
    char input[256];
    fmtWrite("Welcome to lanternOS!\n");
    fmtWrite("Type 'help' for commands.\n");

    while(1) {
        fmtWrite("\n-> ");
        fmtGet("%s", input);

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
        else {
            fmtWrite("Unknown command: %s", input);
        }
    }
}