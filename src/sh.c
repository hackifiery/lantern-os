#define KERN
#include "io.h"
#include "kstdint.h"
#include "sys.h"
#include "idt.h"
#include "ustar.h"
#include "version.h"
#include "api.h"
#include "string_utils.h"

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
    char* tokens[16];
    uint16_t dskBuf[256];

    for(;;) {
        fmtWrite("sh > ");
        fmtGet("%s", input);

        int tokenCount = tokenize(input, tokens, 16);
        if (tokenCount == 0) continue;
        #define cmd(s) else if (strcmp(tokens[0], s) == 0)

        if (strcmp(tokens[0], "") == 0) continue;

        cmd("help") {
            fmtWrite("Available: help, echo, clear, ping, uptime, uname, free, cat, ls, panic, reboot, shutdown\n");
        }
        cmd("echo") {
            if (strcmp(tokens[1], ">") == 0) {
                tarLoad();
                blocking = 0;
                char buf[256];
                fmtGet("%s", buf);
                buf[strlen(buf)] = '\n'; // echo adds newline
                buf[strlen(buf)] = '\0';
                tarEdit(tokens[2], buf, strlen(buf));
                tarFlush();
                blocking = 1;
            }
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
            //uint16_t buf[256];
            //ataRead(0, buf);
            tarLoad();
            tarList(tokens[1]);
            fmtWrite("\n");
        }
        cmd("rm") {
            if (tarRm(tokens[1])) {
                //fmtWrite("removed from buffer\n");
                tarFlush();
                //fmtWrite("flushed to disk\n");
                //tarLoad();
                //fmtWrite("reloaded, verifying...\n");
                //tarList(tokens[1]);
            }
        }
        cmd("cat") {
            tarLoad();
            if (strcmp(tokens[1], ">") == 0) {
                blocking = 0;
                //fmtWrite("\n");
                char buf[256];
                fmtGet("%s", buf);
                tarEdit(tokens[2], buf, strlen(buf));
                tarFlush();
                blocking = 1;
            }
            else {
                /*DEBUG: struct TarHeader *h = (struct TarHeader *)tarBuf;
                fmtWrite("first 4 entries after load:\n");
                for (int i = 0; i < 4 && tarValid(h); i++) {
                    fmtWrite("  '%s'\n", h->name);
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
            char *data = 0;
            int size = tarReadFile(tokens[0], &data);
            if (size == 0) { fmtWrite("not found: %s\n", tokens[0]); continue; }
            // copy to 0x200000
            uint8_t *dest = (uint8_t *)0x200000;
            for (int i = 0; i < size; i++) dest[i] = ((uint8_t *)data)[i];
            // jump to it
            /*DEBUG: fmtWrite("api addr: %x\n", (unsigned int)&api);
            fmtWrite("api.fmtWrite: %x\n", (unsigned int)api.fmtWrite);
            fmtWrite("jumping to: %x\n", (unsigned int)0x200000);
            fmtWrite("calc bytes: %02x %02x %02x %02x\n",
                    dest[0], dest[1], dest[2], dest[3]);*/
            typedef void (*Program)(struct KernelAPI *);
            Program prog = (Program)0x200000;
            prog(api);
            //fmtWrite("Unknown command: %s", tokens[0]);
        }

        //fmtWrite("\n");
        #undef cmd
    }
}