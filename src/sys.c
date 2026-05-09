#include "sys.h"
#include "io.h"
#include <stdint.h>
//#include <stdint.h>

extern unsigned int _kernelStart;
extern unsigned int _kernelEnd;

unsigned int getUsedMem(void) {
    unsigned int start = (unsigned int)&_kernelStart;
    unsigned int end = (unsigned int)&_kernelEnd;
    unsigned int used = (end - start) / 1024; // in KB

    return used;
}

int supports64bit(void) {
    uint32_t eax, edx;

    __asm__ __volatile__ ("cpuid" : "=a"(eax) : "a"(0x80000000) : "ebx", "ecx", "edx");
    if (eax < 0x80000001) {
        return 0; // ext functions not supported
    }

    __asm__ __volatile__ ("cpuid" : "=a"(eax), "=d"(edx) : "a"(0x80000001) : "ebx", "ecx");
    return (edx & (1 << 29)) != 0;
}

unsigned int getTotalMem(struct MemoryInfo* mem) {
    unsigned long long total = 0;
    // fmtWrite("\nmem entry_count is %d\n", mem->entry_count);
    for (unsigned int i = 0; i < mem->entry_count; i++) {
        // fmtWrite("checking entry %d, type is %d, with size %d bytes\n", i, mem->entries[i].type, sizeof(mem->entries[i]));
        // Type 1 is = good ram
        if (mem->entries[i].type == 1) {
            // fmtWrite("found ram\n");
            total += mem->entries[i].length;
        }
    }
    // fmtWrite("\n");
    /*unsigned char* raw = (uint8_t*)0x7004;
    for(int i = 0; i < 24; i++) {
        fmtWrite("%d ", raw[i]);
    }*/
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
