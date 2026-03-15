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

static void com(struct multibootInfo* mbPtr) {
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
            fmtWrite("Available: help, echo, cls, ping, uptime, sysinfo, mem, reboot, shutdown");
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
            fmtWrite("lanternOS (%s) v%s (built %s)", __BUILD_ARCH__, VER, __BUILD_DATE__);
        }
        cmd("mem") {
            unsigned int total = getTotalMem(mbPtr);
            unsigned int used = getUsedMem();
            if (strcmp(tokens[1], "/m") == 0)      fmtWrite("total = %dm, used = %dm, free = %dm", total/1024, used/1024, (total - used)/1024);
            else if (strcmp(tokens[1], "/g") == 0) fmtWrite("total = %dg, used = %dg, free = %dg", total/1024/1024, used/1024/1024, (total - used)/1024/1024);
            else if (strcmp(tokens[1], "/b") == 0) fmtWrite("total = %db, used = %db, free = %db", total*1024, used*1024, (total - used)*1024);
            else                                   fmtWrite("total = %dk, used = %dk, free = %dk", total, used, total - used);
        }
        cmd("panic") {
            if (!(atoi(tokens[1]) <= 21 || atoi(tokens[1]) == 34)) fmtWrite("Unknown fault interrupt");
            if (tokens[1]) sendInterrupt(atoi(tokens[1]));
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

void kmain(struct multibootInfo* mbPtr) {
    initIdt();
    initGdt();
    clearScreen();
    initTimer(100);
    enableCursor(14, 15);
    // __asm__ volatile("sti");
    fmtWrite("Welcome to lanternOS!\n");
    fmtWrite("Type 'help' for commands.\n\n");
    for (;;) com(mbPtr);
}