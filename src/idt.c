#include "idt.h"
#include "io.h"
#include "sys.h"
struct idtEntry idt[256];
struct idtPtr idtp;
volatile unsigned int sysTicks = 0;

void remapPic(void) {
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
    idt[num].flags   = 0x8E; 
}

void initIdt(void) {
    idtp.limit = (sizeof(struct idtEntry) * 256) - 1;
    idtp.base  = (unsigned int)&idt;

    for(int i = 0; i < 256; i++) setIdtGate(i, 0);
    // exceptions/faults
    extern void isr0();
    extern void isr1();
    // extern void isr2();
    // extern void isr3();
    extern void isr4();
    extern void isr5();
    extern void isr6();
    extern void isr7();
    extern void isr8();
    // extern void isr9();
    extern void isr10();
    extern void isr11();
    extern void isr12();
    extern void isr13();
    extern void isr14();
    // extern void isr15();
    extern void isr16();
    extern void isr17();
    // extern void isr18();
    extern void isr19();
    // extern void isr20();
    extern void isr21();
    extern void isr34();
    
    setIdtGate(0, (unsigned int)isr0);
    setIdtGate(1, (unsigned int)isr1);
    // setIdtGate(1, (unsigned int)isr2);
    // setIdtGate(3, (unsigned int)isr3);
    setIdtGate(4, (unsigned int)isr4);
    setIdtGate(5, (unsigned int)isr5);
    setIdtGate(6, (unsigned int)isr6);
    setIdtGate(7, (unsigned int)isr7);
    setIdtGate(8, (unsigned int)isr8);
    // setIdtGate(9, (unsigned int)isr9);
    setIdtGate(10, (unsigned int)isr10);
    setIdtGate(11, (unsigned int)isr11);
    setIdtGate(12, (unsigned int)isr12);
    setIdtGate(13, (unsigned int)isr13);
    setIdtGate(14, (unsigned int)isr14);
    // setIdtGate(15, (unsigned int)isr15);
    setIdtGate(16, (unsigned int)isr16);
    setIdtGate(17, (unsigned int)isr17);
    // setIdtGate(18, (unsigned int)isr18);
    setIdtGate(19, (unsigned int)isr19);
    // setIdtGate(20, (unsigned int)isr20);
    setIdtGate(21, (unsigned int)isr21);

    // register gates
    extern void irq0(void);
    extern void irq1(void);
    setIdtGate(32, (unsigned int)irq0); 
    setIdtGate(33, (unsigned int)irq1);

    setIdtGate(34, (unsigned int)isr34); // user-thrown exception

    remapPic();
    loadIdt((unsigned int)&idtp);
}

void irqHandler(struct Registers *r) {
    // we must send eoi to the slave first
    if (r->int_no >= 40) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);
    if (r->int_no == 32) {
        sysTicks++;
    }
}

void initTimer(unsigned int frequency) {
    // PIT has an internal clock of 1193180 Hz
    unsigned int divisor = 1193180 / frequency;

    // Send the command byte (0x36 sets square wave mode)
    outb(0x43, 0x36);

    // Split the divisor into two bytes and send them to the PIT
    outb(0x40, (unsigned char)(divisor & 0xFF));
    outb(0x40, (unsigned char)((divisor >> 8) & 0xFF));
}

void fault(struct Registers *r) {
    COLOR = vgaColor(WHITE, BLUE);
    clearScreen();
    fmtWrite("--- KERNEL PANIC!!! ---\n\n");
    

    switch(r->int_no) {
        case 0:  fmtWrite("Division Error (#DE)"); break;
        case 1:  fmtWrite("Debug Exception (#DB)"); break;
        // case 3:  fmtWrite("Breakpoint (#BP)"); break;
        case 4:  fmtWrite("Overflow (#OF)"); break;
        case 5:  fmtWrite("BOUND Range Exceeded (#BR)"); break;
        case 6:  fmtWrite("Invalid Opcode (#UD)"); break;
        case 7:  fmtWrite("Device Not Available (#NM)"); break;
        case 8:  fmtWrite("Double Fault (#DF)"); break;
        // case 9:  fmtWrite("Coprocessor Segment Overrun"); break;
        case 10: fmtWrite("Invalid TSS (#TS)"); break;
        case 11: fmtWrite("Segment Not Present (#NP)"); break;
        case 12: fmtWrite("Stack-Segment Fault (#SS)"); break;
        case 13: fmtWrite("General Protection Fault (#GP)"); break;
        case 14: 
            fmtWrite("Page Fault (#PF)");
            //  the address is in the CR2 register
            unsigned int faulting_addr;
            __asm__ volatile("mov %%cr2, %0" : "=r" (faulting_addr));
            fmtWrite("\nAddress: 0x%08x", faulting_addr);
            break;
        case 16: fmtWrite("x87 Floating-Point Exception (#MF)"); break;
        case 17: fmtWrite("Alignment Check (#AC)"); break;
        case 19: fmtWrite("SIMD Floating-Point Exception (#XM)"); break;
        case 21: fmtWrite("Control Protection Exception (#CP)"); break;
        case 34: fmtWrite("User-thrown Exception (#UT)"); break;
        
        default:
            fmtWrite("Unknown Exception #%d", r->int_no);
            break;
    }

    fmtWrite("\n\nINT: 0x%02x (%d)  ERR: 0x%x\n", r->int_no, r->int_no, r->err_code);
    // edi, esi, ebp, esp, ebx, edx, ecx, eax
    fmtWrite("\nEIP: 0x%08x  CS: 0x%08x\n", r->eip, r->cs);
    fmtWrite("EAX: 0x%08x EBX: 0x%08x\nECX: 0x%08x EDX: 0x%08x\n", r->eax, r->ebx, r->ecx, r->edx);
    fmtWrite("EDI: 0x%08x ESI: 0x%08x\nEBP: 0x%08x ESP: 0x%08x\n", r->edi, r->esi, r->ebp, r->esp);

    fmtWrite("\n\nSystem Halted.");
    disableCursor();
    halt();
}

void interruptDispatcher(struct Registers *r) {
    if (r->int_no >= 32 && r->int_no < 48 && r->int_no != 34) {
        // irq
        if (r->int_no == 33) {
            keyboardHandler();
        }
        else {
            irqHandler(r);
        }
    }
    else {
        fault(r); 
    }
}