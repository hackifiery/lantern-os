[bits 32]
global loadIdt
global irq0
global irq1
global panic
global sendInterrupt
extern irqHandler
extern keyboardHandler
extern fault

; exceptions w/o error code
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push 0      ; Dummy error code
    push %1     ; Interrupt number
    jmp isr_common_stub
%endmacro

; exceptions w/error codes
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    push %1     ; Interrupt number
    jmp isr_common_stub
%endmacro

ISR_NOERRCODE 0  ; div error
ISR_NOERRCODE 1  ; debug
; 2 is a normal interrupt
ISR_NOERRCODE 3  ; breakpoint (INT3)
ISR_NOERRCODE 4  ; overflow (INTO)
ISR_NOERRCODE 5  ; BOUND range exceeded
ISR_NOERRCODE 6  ; invalid opcode
ISR_NOERRCODE 7  ; device not available
ISR_ERRCODE   8  ; double fault
ISR_NOERRCODE 9  ; coprocessor segment overrun
ISR_ERRCODE   10 ; invalid tss
ISR_ERRCODE   11 ; segment not present
ISR_ERRCODE   12 ; stack-segment fault
ISR_ERRCODE   13 ; general protection
ISR_ERRCODE   14 ; page fault
; 15 is reserved
ISR_NOERRCODE 16 ; fpu error
ISR_ERRCODE   17 ; alignment check
; 18 is machine-dependent
ISR_NOERRCODE 19 ; simd fp exceptions
; virtualization not used
ISR_ERRCODE   21 ; control protection exceptions
ISR_NOERRCODE 34 ; user-thrown exception

panic:
    int 34

sendInterrupt:
    push ebp
    mov ebp, esp
    mov al, [ebp + 8]    ; Get the interrupt number (first parameter)
    mov byte [intInstr + 1], al ; Patch the 'int' instruction below

intInstr:
    int 0x00             ; The 0x00 will be replaced by AL at runtime
    
    pop ebp
    ret

isr_common_stub:
    pushad
    push esp        ; PUSH THE POINTER to the start of the registers on the stack
    
    call fault

    add esp, 4      ; Clean up the 'push esp'
    popad           ; Restore registers
    add esp, 8      ; Clean up error code and ISR number
    iretd

loadIdt:
    mov edx, [esp + 4]
    lidt [edx]
    sti               ; interrupts
    ret

irq0:
    pushad            ; Push EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    call irqHandler
    popad             ; Restore them all
    iretd


irq1:
    pushad
    call keyboardHandler
    popad
    iretd