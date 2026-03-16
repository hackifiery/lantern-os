#include "sys.h"
#include "io.h"
//#include <stdint.h>

extern unsigned int _kernelStart;
extern unsigned int _kernelEnd;

unsigned int getUsedMem(void) {
    unsigned int start = (unsigned int)&_kernelStart;
    unsigned int end = (unsigned int)&_kernelEnd;
    unsigned int used = (end - start) / 1024; // in KB

    return used;
}

unsigned int getTotalMem(struct MemoryInfo* mem) {
    unsigned long long total = 0;
    for (unsigned int i = 0; i < mem->entry_count; i++) {
        // Type 1 is Usable RAM
        if (mem->entries[i].type == 1) {
            total += mem->entries[i].length;
        }
    }
    return (unsigned int)(total / 1024);
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
