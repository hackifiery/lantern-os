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
    extern initIdt     ; Declare IDT init
    extern initSerial  ; Declare Serial init

    call initSerial    ; Initialize Serial port
    call initIdt       ; Initialize IDT and enable interrupts (sti is inside loadIdt)
    call kmain         ; NOW jump to your C code
    
    hlt                   ; Halt if C code ever returns
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
resb 16384                     ; 16 KB stack
stack_top: