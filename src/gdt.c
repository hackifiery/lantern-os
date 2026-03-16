#include "gdt.h"

// 3 entries: Null, Code, and Data
struct GdtEntry gdt[3];
struct GdtPtr gdtp;

extern void gdtFlush(unsigned int a);

void setGdtGate(int num, unsigned int base, unsigned int limit, unsigned char access, unsigned char gran) {
    gdt[num].baseLow    = (base & 0xFFFF);
    gdt[num].baseMiddle = (base >> 16) & 0xFF;
    gdt[num].baseHigh   = (base >> 24) & 0xFF;

    gdt[num].limitLow   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;

    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access      = access;
}

void initGdt(void) {
    gdtp.limit = (sizeof(struct GdtEntry) * 3) - 1;
    gdtp.base  = (unsigned int)&gdt;

    // 0x00: Null segment
    setGdtGate(0, 0, 0, 0, 0);

    // 0x08: Code segment (Base: 0, Limit: 4GB, 32-bit, Ring 0)
    // Granularity 0xCF = 4KB blocks, 32-bit mode
    setGdtGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // 0x10: Data segment (Base: 0, Limit: 4GB, 32-bit, Ring 0)
    setGdtGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // Call the assembly function to load the GDT
    gdtFlush((unsigned int)&gdtp);
}