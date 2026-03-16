[org 0x7c00]

; setup stack
xor ax, ax
mov ds, ax
mov es, ax
mov ss, ax
mov bp, 0x9000
mov sp, bp

mov [BOOT_DRIVE], dl    ; Save boot drive (bios puts it in dl)
KERNEL equ 0x10000

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

A20:                    ; fast method
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

loadKern:                ; needs: al (# of sectors to read), es:bx (buffer)
    pusha
    movzx si, al         ; get sectors to read
    mov ch, 0            ; cyl 0
    mov dh, 0            ; head 0
    mov cl, 2            ; sec 2 (sec 1 is bootloader)
    mov dl, [BOOT_DRIVE] ; drive #
    mov ah, 2            ; disk read
    int 0x13             ; disk read interrupt
    jc readFail
    popa
    ret

readFail:                ; prints a '?'
    mov ah, 0x0e
    mov al, '?'
    int 0x10
    jmp $                ; fah

PMode:
    mov ax, 0x1000
    mov es, ax
    mov bx, 0
    mov al, 100          ; 100 sectors, 50kb
    call loadKern

    call A20
    cli
    lgdt [gdtDescriptor] ; get gdt

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

    jmp KERNEL           ; i hope...

; padding + boot sig
times 510-($-$$) db 0
dw 0xAA55