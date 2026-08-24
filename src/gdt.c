#include "gdt.h"
#include <stdint.h>
#include <string.h>

// null, kernel code, kernel data, user code, user data, tss
struct GdtEntry gdt[6];
struct GdtPtr gdtp;

struct Tss tss;
extern void gdtFlush(unsigned int a);
extern void tssFlush(void);
extern uint32_t tss_stack_top;

void setTssGate(int num, uint32_t esp0, uint32_t ss0) {
    uint32_t base = (uint32_t) &tss;
	uint32_t limit = sizeof(struct Tss);
    setGdtGate(num, base, limit, 0xE9, 0x00); // 0xE9 = present, ring 3, type 9
    memset(&tss, 0, sizeof(struct Tss));
    tss.ss0 = ss0;
    tss.esp0 = esp0;
}

void setGdtGate(int num, unsigned int base, unsigned int limit, uint8_t access, uint8_t gran) {
    gdt[num].baseLow    = (base & 0xFFFF);
    gdt[num].baseMiddle = (base >> 16) & 0xFF;
    gdt[num].baseHigh   = (base >> 24) & 0xFF;

    gdt[num].limitLow   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;

    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access      = access;
}

void initGdt(void) {
    gdtp.limit = (sizeof(struct GdtEntry) * 6) - 1;
    gdtp.base  = (unsigned int)&gdt;

    // 0x00: Null segment
    setGdtGate(0, 0, 0, 0, 0);

    // 0x08: Kernel Code segment (Base: 0, Limit: 4GB, 32-bit, Ring 0)
    // Granularity 0xCF = 4KB blocks, 32-bit mode
    setGdtGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // 0x10: Kernel Data segment (Base: 0, Limit: 4GB, 32-bit, Ring 0)
    setGdtGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // 0x1B: User code segment (Base: 0, Limit: 4GB, 32-bit, Ring 3)
    setGdtGate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    // 0x23: User data segment (Base: 0, Limit: 4GB, 32-bit, Ring 3)
    setGdtGate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    // 0x28: TSS segment (Base: &tss, Limit: sizeof(struct Tss), Ring 3)
    setTssGate(5, (uint32_t)&tss_stack_top, 0x10);

    // Call the assembly function to load the GDT
    gdtFlush((unsigned int)&gdtp);
    tssFlush();
}