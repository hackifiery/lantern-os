#include "idt.h"
#include "io.h"
struct idtEntry idt[256];
struct idt_ptr idtp;

volatile unsigned int sysTicks = 0;

void remapPic() {
    outb(0x20, 0x11); // Initialize Master
    outb(0xA0, 0x11); // Initialize Slave
    outb(0x21, 0x20); // Master offset (32)
    outb(0xA1, 0x28); // Slave offset (40)
    outb(0x21, 0x04); // Tell Master there is a slave
    outb(0xA1, 0x02); // Tell Slave its cascade identity
    outb(0x21, 0x01); // 8086 mode
    outb(0xA1, 0x01);
    outb(0x21, 0x0);  // Unmask all interrupts
    outb(0xA1, 0x0);
}

void setIdtGate(unsigned char num, unsigned int base) {
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel     = 0x08;
    idt[num].always0 = 0;
    idt[num].flags   = 0x8E; // Present, Ring 0, Interrupt Gate
}

void initIdt() {
    idtp.limit = (sizeof(struct idtEntry) * 256) - 1;
    idtp.base  = (unsigned int)&idt;

    for(int i = 0; i < 256; i++) setIdtGate(i, 0);

    // register gates
    extern void irq0();
    extern void irq1();
    setIdtGate(32, (unsigned int)irq0); 
    setIdtGate(33, (unsigned int)irq1);

    remapPic();
    loadIdt((unsigned int)&idtp);
}

// This is called by the Assembly stub
void irqHandler() {
    sysTicks++;
    // Send End of Interrupt (EOI) to the PIC chip
    outb(0x20, 0x20); 
}