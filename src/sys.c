#include "sys.h"
#include "io.h"

extern void halt(void);

void reboot(void) {
    short good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    halt();
}

void shutdown(void) {
    // TODO: implement better shutdown
    outw(0x604, 0x2000);
}