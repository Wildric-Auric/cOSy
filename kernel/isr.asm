[extern isr_hdl]
[extern irq_hdl]
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

irq_end:
    pusha 
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    call irq_hdl
    mov eax, [esp + 40]       ;offset to fist pushed value by isr 
    add dword [esp + 44], eax ;44 is offset till EIP
    pop ebx
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

%macro IRQ 3
global irq%1
irq%1:
    cli
    push byte %2   ; first byte
    push byte %3   ; second byte
    jmp irq_end
%endmacro


ISR 0, 0, 0
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
ISR 11, 0, 11
ISR 12, 0, 12
ISR 13, 0, 13
ISR 14, 0, 14
ISR 15, 0, 15
ISR 16, 0, 16
ISR 17, 0, 17
ISR 18, 0, 18
ISR 19, 0, 19
ISR 20, 0, 20
ISR 21, 0, 21
ISR 22, 0, 22
ISR 23, 0, 23
ISR 24, 0, 24
ISR 25, 0, 25
ISR 26, 0, 26
ISR 27, 0, 27
ISR 28, 0, 28
ISR 29, 0, 29
ISR 30, 0, 30
ISR 31, 0, 31
    
IRQ 0, 0,  32
IRQ 1, 0,  33
IRQ 2, 0,  34
IRQ 3, 0,  35
IRQ 4, 0,  36
IRQ 5, 0,  37
IRQ 6, 0,  38
IRQ 7, 0,  39
IRQ 8, 0,  40
IRQ 9, 0,  41
IRQ 10,0,  42
IRQ 11,0,  43
IRQ 12,0,  44
IRQ 13,0,  45
IRQ 14,0,  46
IRQ 15,0,  48

