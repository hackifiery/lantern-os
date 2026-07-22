#define KERN
#define EXEC_ADDR 0x200000
#include "io.h"
#include <stdint.h>
#include "sys.h"
#include "idt.h"
#include "ustar.h"
#include "version.h"
#include "api.h"
#include "binary.h"
#include "elf.h"
#include <string.h>
#include <stdio.h>

#define MAX_TOKENS 16

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

void sh(struct MemoryInfo* mbPtr, struct KernelAPI *api) {
    char input[256];
    char* tokens[MAX_TOKENS];
    uint16_t dskBuf[256];

    int bits = supports64bit();

    for(;;) {
        /*scanf("%s", NULL);
        continue;*/
        printf("/$ "); // TODO: change after dir support
        scanf("%[^\n]", input);
        //printf("got: %s, len = %d\n", input, (int)strlen(input));
        fflush(stdin); // clear stdin buffer
        int tokenCount = tokenize(input, tokens, 16);
        if (tokenCount == 0) continue;
        #define cmd(s) else if (strcmp(tokens[0], s) == 0)

        if (strcmp(tokens[0], "") == 0) continue;

        cmd("help") {
            printf("Available: help, echo, clear, ping, uptime, uname, free, cat, ls, panic, reboot, shutdown\n");
        }
        cmd("echo") {
            if (strcmp(tokens[1], ">") == 0) {
                tarLoad();
                blocking = 0;
                char buf[256];
                scanf("%s", buf);
                buf[strlen(buf)] = '\n'; // echo adds newline
                buf[strlen(buf)] = '\0';
                tarEdit(tokens[2], buf, strlen(buf));
                tarFlush();
                blocking = 1;
            }
            else {
                for(int i = 1; i < tokenCount; i++) {
                    printf("%s ", tokens[i]);
                }
            }
        }
        cmd("clear") {
            clearScreen();
            continue;
        }
        cmd("ping") printf("Pong!\n");
        cmd("uptime") {
            unsigned int seconds = sysTicks / 100;
            printf("%d s\n", seconds);
        }
        cmd("uname") {
            printf("lanternOS i386 v%s (built %s on %s)\n", VER, __BUILD_DATE__, __BUILD_ARCH__);
        }
        cmd("arch") printf("x86%s\n", bits ? "_64": "");
        cmd("free") {
            unsigned int total = getTotalMem(mbPtr);
            unsigned int used = getUsedMem();
            if (strcmp(tokens[1], "-m") == 0)      printf("total = %dm, used = %dm, free = %dm\n", total/1024, used/1024, (total - used)/1024);
            else if (strcmp(tokens[1], "-g") == 0) printf("total = %dg, used = %dg, free = %dg\n", total/1024/1024, used/1024/1024, (total - used)/1024/1024);
            else if (strcmp(tokens[1], "-b") == 0) printf("total = %db, used = %db, free = %db\n", total*1024, used*1024, (total - used)*1024);
            else                                   printf("total = %dk, used = %dk, free = %dk\n", total, used, total - used);
        }
        
        cmd("panic") {
            if (atoi(tokens[1]) > 21 || atoi(tokens[1]) == 34 || atoi(tokens[1]) == 9 || atoi(tokens[1]) == 15 || atoi(tokens[1]) == 18 || atoi(tokens[1]) == 20) printf("Unknown fault interrupt\n");
            if (tokenCount == 2) sendInterrupt(atoi(tokens[1]));
            else userPanic();
        }
        cmd("ls") {
            //uint16_t buf[256];
            //ataRead(0, buf);
            tarLoad();
            tarList(tokens[1]);
            printf("\n");
        }
        cmd("rm") {
            if (tarRm(tokens[1])) {
                //printf("removed from buffer\n");
                tarFlush();
                //printf("flushed to disk\n");
                //tarLoad();
                //printf("reloaded, verifying...\n");
                //tarList(tokens[1]);
            }
        }
        cmd("cat") {
            tarLoad();
            if (strcmp(tokens[1], ">") == 0) {
                blocking = 0;
                //printf("\n");
                char buf[256];
                scanf("%s", buf);
                tarEdit(tokens[2], buf, strlen(buf));
                tarFlush();
                blocking = 1;
            }
            else {
                /*DEBUG: struct TarHeader *h = (struct TarHeader *)tarBuf;
                printf("first 4 entries after load:\n");
                for (int i = 0; i < 4 && tarValid(h); i++) {
                    printf("  '%s'\n", h->name);
                    h = tarNext(h);
                }*/
                tarPrintFile(tokens[1]);
            }
        }
        cmd("touch") {
            tarTouch(tokens[1]);
            tarFlush();
        }
        cmd("reboot")   reboot();
        cmd("shutdown") shutdown();
        else {
            if (!tarFind(tokens[0])) printf("Not found\n");
            else if (!loadBinary(tokens[0], *api)) {
                printf("Corrupted binary\n");
            }
        }

        //printf("\n");
        #undef cmd
        for (int i = 0; i < MAX_TOKENS; i++) tokens[i] = 0;
    }
}