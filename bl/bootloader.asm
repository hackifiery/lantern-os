[org 0x7c00]

xor ax, ax
mov ds, ax
mov es, ax
mov ss, ax
mov bp, 0x9000
mov sp, bp

mov [BOOT_DRIVE], dl    ; Save boot drive early

jmp PMode

; get mem via E820
getMem:
    xor ebx, ebx            ; ebx must be 0 to start
    mov edx, 0x534D4150     ; magic number 'SMAP'
    mov di, 0x9000          ; destination buffer offset
    mov [memEntries], word 0

.next_entry:
    mov eax, 0xE820         ; BIOS Function
    mov ecx, 24             ; buf size
    int 0x15
    jc .done                ; carry set means end of list or error

    cmp eax, 0x534D4150     ; verify BIOS set eax to 'SMAP'
    jne .error

    add di, 24              ; move buffer pointer
    inc word [memEntries]   ; count entries
    test ebx, ebx           ; If ebx=0 list is finished
    jnz .next_entry

.done:
    ret

.error:
    call badRead

memEntries: dw 0

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

A20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

loadKern:
    pusha
    movzx si, al        ; SI = total sectors
    mov ch, 0           ; cyl
    mov dh, 0           ; head
    mov cl, 2           ; start sector

.loop:
    mov di, 5           ; retry counter
.retry:
    mov ax, 0x0201      ; read 1
    mov dl, [BOOT_DRIVE]
    int 0x13
    jnc .next
    
    xor ax, ax          ; reset
    int 0x13
    dec di
    jnz .retry
    jmp badRead

.next:
    mov ax, es
    add ax, 0x20        ; advance segment by 512 bytes
    mov es, ax
    
    dec si
    jz .done
    
    inc cl
    cmp cl, 19
    jne .loop
    
    mov cl, 1
    inc dh
    cmp dh, 2
    jne .loop
    
    mov dh, 0
    inc ch
    jmp .loop

.done:
    popa
    ret

badRead:
    mov ah, 0x0e
    mov al, '?'
    int 0x10
    jmp $

PMode:
    ; Load Kernel to 0x10000 (ES:BX = 0x1000:0000)
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    mov al, 100          ; sectors (50 kb)
    call loadKern

    ; Prepare Protected Mode
    call A20
    cli
    lgdt [gdtDescriptor]
    
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Flush pipeline with far jump
    jmp 08h:main

[bits 32]
main:
    mov ax, 10h
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    push 0x9000             ; ptr to entries
    push dword [memEntries]; # of entries
    
    jmp 0x10000

; pad to 512 b
times 510-($-$$) db 0
dw 0xAA55