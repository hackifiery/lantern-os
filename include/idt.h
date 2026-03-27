#ifndef LANTERN_IDT_H
#define LANTERN_IDT_H

#include "kstdint.h"
#include "sys.h"

struct idtEntry {
    uint16_t base_lo;    // Lower 16 bits of handler address
    uint16_t sel;        // Kernel segment selector (usually 0x08)
    uint8_t  always0;    // This must always be zero
    uint8_t  flags;      // Flags (0x8E for interrupt gate)
    uint16_t base_hi;    // Upper 16 bits of handler address
} __attribute__((packed));

struct idtPtr {
    uint16_t limit;      // Size of IDT - 1
    uint32_t base;       // Address of the IDT array
} __attribute__((packed));

extern void loadIdt(unsigned int idtPtr_address);
extern void userPanic(void);
extern void sendInterrupt(uint8_t n);
void interruptDispatcher(struct Registers *r);
extern volatile unsigned int sysTicks;

void initTimer(unsigned int frequency);
void initIdt(void);

#endif // LANTERN_IDT_H