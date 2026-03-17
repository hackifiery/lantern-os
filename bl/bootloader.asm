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
MMAP   equ 0x7000

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

getMem:
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
    ret
    

A20:                     ; fast method
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

    call getMem
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

    push MMAP + 4      ; arg 2 (e820 entries)
    movzx eax, word [MMAP] ; get the 16-bit count and zero-extend it to 32-bit
    push eax           ;a arg 1 (entryCount)

    call KERNEL           ; i hope...

; padding + boot sig
times 510-($-$$) db 0
dw 0xAA55