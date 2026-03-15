#include "sys.h"
#include "io.h"
//#include <stdint.h>

extern void halt(void);

extern unsigned int _kernelStart;
extern unsigned int _kernelEnd;

unsigned int getUsedMem(void) {
    unsigned int start = (unsigned int)&_kernelStart;
    unsigned int end = (unsigned int)&_kernelEnd;
    unsigned int used = (end - start) / 1024; // in KB

    return used;
}

unsigned int getTotalMem(struct multibootInfo* mbPtr) {
    unsigned int kb = mbPtr->mem_lower + mbPtr->mem_upper;
    return kb;
}

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