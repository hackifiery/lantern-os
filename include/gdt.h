#ifndef LANTERN_GDT_H
#define LANTERN_GDT_H
#include <stdint.h>

struct GdtEntry {
    uint16_t limitLow;
    uint16_t baseLow;
    uint8_t  baseMiddle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  baseHigh;
} __attribute__((packed));

struct GdtPtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct Tss {
    uint32_t prevTss;
    uint32_t esp0, ss0;   // only these matter for ring0->ring3 returns
    uint32_t esp1, ss1;
    uint32_t esp2, ss2;
    uint32_t cr3, eip, eflags;
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap, iomapBase;
} __attribute__((packed));

extern struct Tss tss;

void initGdt(void);
void setGdtGate(int num, unsigned int base, unsigned int limit, uint8_t access, uint8_t gran);

#endif // LANTERN_GDT_H