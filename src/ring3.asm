global enterUsermode
; void enterUsermode(uint32_t entry, uint32_t userStackTop, void *arg);
enterUsermode:
    mov eax, [esp + 4]   ; entry
    mov ecx, [esp + 8]   ; userStackTop
    mov edx, [esp + 12]  ; arg

    mov bx, (4 * 8) | 3
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    sub ecx, 4
    mov [ecx], edx        ; push arg for cdecl main(api)
    sub ecx, 4
    mov dword [ecx], 0    ; dummy return addr — never used, _start calls _exit

    push (4 * 8) | 3       ; SS
    push ecx                ; ESP
    pushf
    push (3 * 8) | 3        ; CS
    push eax                 ; EIP
    iret