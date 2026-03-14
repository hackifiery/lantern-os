[bits 32]
extern irqHandler
global loadIdt
global irq0

loadIdt:
    mov edx, [esp + 4]
    lidt [edx]
    sti               ; interrupts
    ret

irq0:
    pushad            ; Push EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    call irqHandler
    popad             ; Restore them all
    iretd

extern keyboardHandler
global irq1

irq1:
    pushad
    call keyboardHandler
    popad
    iretd