[bits 32]
global int80Handler
extern syscallDispatcher

int80Handler:
    ; CPU automatically pushed: SS, ESP, EFLAGS, CS, EIP
    push eax        ; orig_eax (preserve syscall number)
    push gs
    push fs
    push es
    push ds
    push eax        ; eax (general-register slot)
    push ebp
    push edi
    push esi
    push edx
    push ecx
    push ebx

    ; pass pointer to the top of the stack as 1st argument to C
    mov eax, esp
    push eax
    call syscallDispatcher
    add esp, 4      ; clean up parameter

    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    pop ebp
    pop eax
    pop ds
    pop es
    pop fs
    pop gs
    add esp, 4      ; discard orig_eax — do NOT pop into eax again, it holds the return value

    iret            ; back to ring 3