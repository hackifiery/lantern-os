bits 32
section .multiboot
align 4
    dd 0x1BADB002              ; Magic
    dd 0x03                    ; Flags
    dd -(0x1BADB002 + 0x03)    ; Checksum

section .text
extern kmain
global _start
global halt

halt:
    hlt

_start:
    cli                ; Disable interrupts during setup
    mov esp, stack_top ; Set up stack pointer
    extern initIdt
    extern initSerial

    call initSerial
    call initIdt
    push ebx            ; multiboot
    call kmain
    
    hlt
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
resb 16384              ; 16 KB stack
stack_top: