#ifndef LANTERN_IDT_H
#define LANTERN_IDT_H

struct idtEntry {
    unsigned short base_lo;    // Lower 16 bits of handler address
    unsigned short sel;        // Kernel segment selector (usually 0x08)
    unsigned char  always0;    // This must always be zero
    unsigned char  flags;      // Flags (0x8E for interrupt gate)
    unsigned short base_hi;    // Upper 16 bits of handler address
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;      // Size of IDT - 1
    unsigned int   base;       // Address of the IDT array
} __attribute__((packed));

extern void loadIdt(unsigned int idt_ptr_address);
extern volatile unsigned int sysTicks;
void initTimer(unsigned int frequency);
void initIdt(void);

#endif // LANTERN_IDT_H