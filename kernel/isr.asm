[extern isr_hdl]

; puhsha stacks: edi,esi,ebp,esp,ebx,edx,ecx,eax
; we stack 9 32 reg bfore call to handler, 36 bytes
isr_end:
    pusha 
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    call isr_hdl
    mov eax, [esp + 40]       ;offset to fist pushed value by isr 
    add dword [esp + 44], eax ;44 is offset till EIP
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    
    add esp, 8 

    sti 
    iret

; Before isr is called the stack is as follows
; | EIP    |
; | CS     |
; | EFLAGS |
; | ESP    | 
; | SS     | 
; 20 bytes
; buttom is first


%macro ISR 3
global isr%1
isr%1:
    cli
    push byte %2   ; first byte
    push byte %3   ; second byte
    jmp isr_end
%endmacro


ISR 0, 2, 0
ISR 1, 0, 1
ISR 2, 0, 2
ISR 3, 0, 3
ISR 4, 0, 4
ISR 5, 0, 5
ISR 6, 0, 6
ISR 7, 0, 7
ISR 8, 0, 8
ISR 9, 0, 9
ISR 10, 0, 10

    
    

