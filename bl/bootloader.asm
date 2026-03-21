[org 0x7c00]

jmp stuff              ; simulate section .text (this is a flat binary)

msgStart   db "Lightbulb bootloader v0.0.1. Copyright (c) 2026 hackifiery, all rights reserved.", 0
;msgStart   db 0
msgLoad    db "Kernel...", 0
msgMem     db "Memory...", 0
msgA20     db "A20...", 0
msgCli     db "Interrupts...", 0
msgLgdt    db "GDT...", 0
msgPmode   db "Protected mode and transfer to kernel...", 0

msgOk      db "ok", 13, 10, 0

stuff:

; setup stack
xor ax, ax
mov ds, ax
mov es, ax
mov ss, ax
mov bp, 0x9000
mov sp, bp

mov [BOOT_DRIVE], dl    ; Save boot drive (bios puts it in dl)
KERNEL equ 0x10000
MMAP   equ 0x7000

mov si, msgStart
call log

jmp PMode

BOOT_DRIVE db 0

gdtStart: 
    dq 0x0
gdtCode:  
    dw 0xFFFF, 0x0, 0x9A00, 0x00CF
gdtData:  
    dw 0xFFFF, 0x0, 0x9200, 0x00CF
gdtEnd:

gdtDescriptor:
    dw gdtEnd - gdtStart - 1
    dd gdtStart         ; NASM handles the 0x7c00 offset because of [org]

log:                    ; logs a msg to the screen. needs: si (string)
    pusha
    mov ah, 0x0e
.loop:
    lodsb               ; load byte at ds:si into al & increment si
    test al, al         ; test for null char
    jz .done
    int 0x10
    jmp .loop
.done:
    popa
    ret

dap:
    db 0x10             ; size (16b)
    db 0                ; always 0
    dw 100              ; sec count
    dw 0                ; dest offset
    dw 0x1000           ; dest seg
    dd 1                ; sec 1 (0 is bootloader)
    dd 0                ; high 32 bits of lba

getMem:
    mov si, msgMem
    call log
    push es
    mov ax, 0
    mov es, ax
    mov di, MMAP+4      ; now writes to physical 0x7004
    mov ebx, 0
    mov bp, 0
    mov edx, 0x0534D4150; magic word
.loop:
    mov eax, 0xe820
    mov [es:di + 20], dword 1 ; use es override to be safe
    mov ecx, 24
    int 0x15
    jc .done
    cmp eax, edx
    jne .done
    
    add di, 24
    inc bp
    test ebx, ebx
    jnz .loop
.done:
    mov [MMAP], bp          ; Save count at physical 0x7000
    pop es                  ; Restore ES to 0x1000

    mov si, msgOk
    call log

    ret
    

A20:                     ; fast method
    mov si, msgA20
    call log

    in al, 0x92
    or al, 2
    out 0x92, al

    mov si, msgOk
    call log

    ret

loadKern:                ; needs: al (# of sectors to read), es:bx (buffer)
    mov si, msgLoad
    call log
    
    mov dl, [BOOT_DRIVE]
    mov si, dap
    mov ah, 0x42         ; lba read
    int 0x13             ; disk read interrupt
    jc readFail

    mov si, msgOk
    call log

    ret

readFail:                ; prints a '?'
    mov ah, 0x0e
    mov al, '?'
    int 0x10
    jmp $                ; fah

PMode:
    call loadKern

    call getMem
    call A20

    mov si, msgCli
    call log

    cli

    mov si, msgOk
    call log

    mov si, msgLgdt
    call log

    lgdt [gdtDescriptor] ; get gdt

    mov si, msgOk
    call log

    mov si, msgPmode
    call log
    mov si, msgOk
    call log
    ; enter protected mode
    mov eax, cr0 
    or al, 1
    mov cr0, eax

    jmp 08h:main

main:
    [bits 32]
    mov ax, 10h          ; 0x10 is the data segment in the new gdt
    ; update everything
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000     ; better 32-bit stack addr

    push MMAP + 4      ; arg 2 (e820 entries)
    movzx eax, word [MMAP] ; get the 16-bit count and zero-extend it to 32-bit
    push eax           ;a arg 1 (entryCount)
    ; DEBUG: hlt
    call KERNEL           ; i hope...

; padding + boot sig
times 510-($-$$) db 0
dw 0xAA55