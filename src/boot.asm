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
extern initIdt
extern initSerial

_start:
    ; DEBUG: mov dword [0xb8004], 0x2f4c2f4c
    cli                ; Disable interrupts during setup
    ; save the arguments passed by the bootloader
    pop eax
    pop ebx            ; entryCount
    pop ecx            ; entries

    mov esp, stack_top ; Reset stack for the kernel

    push ecx
    push ebx

    call initSerial
    call initIdt
    call kmain
    
    hlt
.hang:
    hlt
    jmp .hang


halt:
    hlt

section .bss
align 16
stack_bottom:
resb 16384              ; 16 KB stack
stack_top: