global gdtFlush
gdtFlush:
    mov eax, [esp + 4]  ; Get the pointer to gdtp from the stack
    lgdt [eax]          ; Load the GDT

    mov ax, 0x10        ; 0x10 is our Data Segment (index 2 * 8)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush     ; Far jump to our Code Segment (index 1 * 8)
.flush:
    ret